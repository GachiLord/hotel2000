#include "user.h"
#include <libpq-fe.h>

#pragma once

typedef struct {
  PermissionLevel permission_level;
  PGconn *conn;
  char *user;
  char *password;
  char *port;
  char *host;
  char *database;
} DbState;

bool db_connect();

void store_conf();
void init_db_state();
void free_db_state();
