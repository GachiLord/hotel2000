#include <libpq-fe.h>

#pragma once

typedef struct {
  PGconn *conn;
  char *user;
  char *password;
  char *port;
  char *host;
  char *database;
} DbState;

int db_connect();

void init_db_state();
void free_db_state();
