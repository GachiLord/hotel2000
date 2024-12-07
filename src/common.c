#include "common.h"
#include "database.h"
#include "glib.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

// static

static GtkWidget *TOAST;

// UI

static gboolean remove_overlay() {
  gtk_overlay_remove_overlay(GTK_OVERLAY(APP_OVERLAY), TOAST);
  TOAST = NULL;
  return false;
}

void add_widget_to_main_stack(GtkWidget *widget) {
  gtk_stack_add_child(APP_STACK, widget);
  gtk_stack_set_visible_child(APP_STACK, widget);
}

void remove_widget_from_main_stack(GtkWidget *widget, GtkWidget *parent) {
  gtk_stack_set_visible_child(APP_STACK, parent);
  gtk_stack_remove(APP_STACK, widget);
}

void show_toast(const char *text) {
  if (TOAST != NULL) {
    gtk_label_set_text(GTK_LABEL(TOAST), text);
    return;
  }

  TOAST = gtk_label_new(text);
  // apply css
  gtk_widget_add_css_class(TOAST, "toast");
  gtk_widget_set_halign(TOAST, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(TOAST, GTK_ALIGN_START);
  // add to overlay
  gtk_widget_set_valign(TOAST, GTK_ALIGN_END);
  gtk_overlay_add_overlay(GTK_OVERLAY(APP_OVERLAY), TOAST);
  g_timeout_add(2000, remove_overlay, NULL);
}

int handle_db_error(PGresult *res, const char *const err_msg) {
  int c = PQresultStatus(res);
  if (c != PGRES_TUPLES_OK && c != PGRES_COMMAND_OK) {
    g_printerr("Query failed: %s", PQerrorMessage(DB_STATE->conn));
    show_toast(err_msg);
    PQclear(res);
    return 1;
  }
  return 0;
}
