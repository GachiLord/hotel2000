#include "glib.h"
#include <gtk/gtk.h>

extern GtkWidget *APP_OVERLAY;
static GtkWidget *TOAST;

static gboolean remove_overlay() {
  GtkWidget *revealer = gtk_revealer_new();
  gtk_overlay_remove_overlay(GTK_OVERLAY(APP_OVERLAY), TOAST);
  TOAST = NULL;
  return false;
}

void showToast(const char *text) {
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
