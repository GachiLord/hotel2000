#include "database.h"
#include "common.h"
#include "gio/gio.h"
#include "user.h"
#include <glib.h>
#include <glib/gprintf.h>
#include <libpq-fe.h>

// state

DbState *DB_STATE;

void free_db_state() {
  if (DB_STATE->conn != NULL)
    PQfinish(DB_STATE->conn);

  g_free(DB_STATE->user);
  g_free(DB_STATE->password);
  g_free(DB_STATE->port);
  g_free(DB_STATE->host);
  g_free(DB_STATE->database);
  g_free(DB_STATE);
}

static void exit_nicely() {
  free_db_state();
  exit(1);
}

// state persisting

const char *CONF_FILE_NAME = "conf.txt";

#define set_field(field, text)                                                 \
  {                                                                            \
    const gsize len = g_utf8_strlen(text, -1);                                 \
                                                                               \
    if (len > 0) {                                                             \
      DB_STATE->field = g_realloc(DB_STATE->field, len + 1);                   \
      g_stpcpy(DB_STATE->field, text);                                         \
    } else {                                                                   \
      DB_STATE->field[0] = '\0';                                               \
    }                                                                          \
  }

static void load_conf() {
  GFile *file = g_file_new_for_path(CONF_FILE_NAME);

  // load file

  if (file == NULL)
    return;

  gchar *buf;
  gsize len;

  GFileInfo *info = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_SIZE,
                                      G_FILE_QUERY_INFO_NONE, NULL, NULL);

  if (info == NULL || g_file_info_get_size(info) > 2048) {
    if (file != NULL)
      g_object_unref(file);
    if (info != NULL)
      g_object_unref(info);
    return;
  }

  if (g_file_load_contents(file, NULL, &buf, &len, NULL, NULL) == false) {
    if (file != NULL)
      g_object_unref(file);
    if (info != NULL)
      g_object_unref(info);
    g_free(buf);
    return;
  }

  // update state
  char field = 0;
  char *token = strtok(buf, "\n");
  while (token) {
    switch (field) {
    case 0:
      set_field(user, token);
      break;
    case 1:
      set_field(port, token);
      break;
    case 2:
      set_field(host, token);
      break;
    case 3:
      set_field(database, token);
      break;
    }
    token = strtok(NULL, "\n");
    field++;
  }
  
  if (file != NULL)
    g_object_unref(file);
  if (info != NULL)
    g_object_unref(info);
  g_free(buf);

  return;

}

void store_conf() {
  GFile *file = g_file_new_for_path(CONF_FILE_NAME);

  if (file == NULL)
    return;

  char *buf;
  int len = asprintf(&buf, "%s\n%s\n%s\n%s\n", DB_STATE->user, DB_STATE->port,
                     DB_STATE->host, DB_STATE->database);

  g_file_replace_contents(file, buf, len, NULL, false, G_FILE_CREATE_NONE, NULL,
                          NULL, NULL);

  g_object_unref(file);
  g_free(buf);
}

// logic

void init_db_state() {
  DB_STATE = g_malloc(sizeof(DbState));
  DB_STATE->permission_level = VIEWER;
  DB_STATE->conn = NULL;
  DB_STATE->user = g_strdup("postgres");
  DB_STATE->password = g_strdup("");
  DB_STATE->port = g_strdup("5432");
  DB_STATE->host = g_strdup("localhost");
  DB_STATE->database = g_strdup("hotel2000");
  // use persisted values
  load_conf();
}

PermissionLevel get_permission_level() {
  PGresult *res =
      PQexec(DB_STATE->conn, "SELECT * FROM get_permission_level()");

  if (handle_db_error(res, "Не удалось получить роль пользователя") == false) {
    g_printerr(
        "Cannot get permission_level from db, act like user has MANAGER role");
    return 2;
  }

  PermissionLevel level = atoi(PQgetvalue(res, 0, 0));
  PQclear(res);
  return level;
}

bool db_connect() {
  // create connection string
  char *conn;
  asprintf(&conn, "user='%s' password='%s' port='%s' host='%s' dbname='%s'",
           DB_STATE->user, DB_STATE->password, DB_STATE->port, DB_STATE->host,
           DB_STATE->database);
  // start connection and save params
  DB_STATE->conn = PQconnectdb(conn);
  // free conn string
  g_free(conn);
  // check if connection is successful
  if (DB_STATE->conn == NULL) {
    g_printerr("cannot create PGconn");
    exit_nicely();
  }
  if (PQstatus(DB_STATE->conn) != CONNECTION_OK) {
    g_printerr("%s", PQerrorMessage(DB_STATE->conn));
    PQfinish(DB_STATE->conn);
    return false;
  }
  // state's permission_level
  DB_STATE->permission_level = get_permission_level();
  return true;
}
