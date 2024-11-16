#include <libpq-fe.h>

#pragma once

struct {
  PGconn *conn;
  char *user;
  char *password;
  char *port;
  char *host;
  char *database;
} typedef DbState;

int dbConnect();

void initDbState();
