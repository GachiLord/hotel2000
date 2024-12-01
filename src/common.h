#include "database.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

// vars

extern DbState *DB_STATE;
extern GtkWindow *APP_WINDOW;
extern GtkStack *APP_STACK;
extern GtkOverlay *APP_OVERLAY;

// ui

void show_toast(const char *text);

int handle_db_error(PGresult *res, const char *const err_msg);
