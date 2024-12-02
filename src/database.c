#include "database.h"
#include "glib.h"
#include <glib/gprintf.h>
#include <libpq-fe.h>

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

void init_db_state() {
  // pass initial values TODO: use persisted values
  DB_STATE = g_malloc(sizeof(DbState));
  DB_STATE->conn = NULL;
  DB_STATE->user = g_strdup("postgres");
  DB_STATE->password = g_strdup("root");
  DB_STATE->port = g_strdup("5432");
  DB_STATE->host = g_strdup("localhost");
  DB_STATE->database = g_strdup("hotel2000");
}

int db_connect() {
  // create connection string
  char *conn;
  asprintf(&conn, "user=%s password=%s port=%s host=%s dbname=%s",
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
