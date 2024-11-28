#include "database.h"
#include <libpq-fe.h>

// vars

extern DbState *DB_STATE;

// ui

void show_toast(const char *text);

int handle_db_error(PGresult *res, const char *const err_msg);
