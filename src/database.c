#include "database.h"
#include "glib.h"
#include <glib/gprintf.h>
#include <libpq-fe.h>

DbState *DB_STATE;

void free_db_state() {
  g_free(DB_STATE->user);
  g_free(DB_STATE->password);
  g_free(DB_STATE->port);
  g_free(DB_STATE->host);
  g_free(DB_STATE->database);
  PQfinish(DB_STATE->conn);
  g_free(DB_STATE);
}

static void exit_nicely() {
  free_db_state();
  exit(1);
}

void init_db_state() {
  // allocate fields
  DB_STATE = g_malloc(sizeof(DbState));
  DB_STATE->user = g_malloc(25);
  DB_STATE->password = g_malloc(25);
  DB_STATE->port = g_malloc(25);
  DB_STATE->host = g_malloc(25);
  DB_STATE->database = g_malloc(25);
  // pass initial values TODO: use persisted values
  g_stpcpy(DB_STATE->user, "postgres");
  g_stpcpy(DB_STATE->password, "root");
  g_stpcpy(DB_STATE->port, "5432");
  g_stpcpy(DB_STATE->host, "localhost");
  g_stpcpy(DB_STATE->database, "hotel2000");
}

int db_connect() {
  // create connection string
  size_t len =
      g_utf8_strlen("user=", -1) + g_utf8_strlen(DB_STATE->user, -1) +
      g_utf8_strlen(" password=", -1) + g_utf8_strlen(DB_STATE->password, -1) +
      g_utf8_strlen(" port=", -1) + g_utf8_strlen(DB_STATE->port, -1) +
      g_utf8_strlen(" host=", -1) + g_utf8_strlen(DB_STATE->host, -1) +
      g_utf8_strlen(" dbname=", -1) + g_utf8_strlen(DB_STATE->database, -1);
  char *conn = g_malloc(len);
  g_sprintf(conn, "user=%s password=%s port=%s host=%s dbname=%s",
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
    return 1;
  }
  return 0;
}
