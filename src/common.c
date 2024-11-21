#include "database.h"
#include "glib.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

extern GtkWidget *APP_OVERLAY;
extern DbState *DB_STATE;
static GtkWidget *TOAST;

static gboolean remove_overlay() {
  gtk_overlay_remove_overlay(GTK_OVERLAY(APP_OVERLAY), TOAST);
  TOAST = NULL;
  return false;
}

void show_toast(const char *const text) {
  if (TOAST != NULL)
    return;

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
  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    g_printerr("Query failed: %s", PQerrorMessage(DB_STATE->conn));
    show_toast(err_msg);
    PQclear(res);
    return 1;
  }
  return 0;
}
