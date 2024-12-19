#include "database.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

#pragma once

// vars

extern DbState *DB_STATE;
extern GtkWindow *APP_WINDOW;
extern GtkStack *APP_STACK;
extern GtkStack *MAIN_STACK;
extern GtkOverlay *APP_OVERLAY;
extern GtkWidget *HOME_WIDGET;

// ui

void remove_widget_from_main_stack(GtkWidget *widget, GtkWidget *parent);
void add_widget_to_main_stack(GtkWidget *widget);

void show_toast(const char *text);
bool handle_db_error(PGresult *res, const char *const err_msg);
