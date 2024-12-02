#include "database.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

// vars

extern DbState *DB_STATE;
extern GtkWindow *APP_WINDOW;
extern GtkStack *APP_STACK;
extern GtkOverlay *APP_OVERLAY;

// ui

void remove_widget_from_main_stack(GtkWidget *widget);
void add_widget_to_main_stack(GtkWidget *widget, const char *name);

void show_toast(const char *text);

int handle_db_error(PGresult *res, const char *const err_msg);
