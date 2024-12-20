#include "asprintf.h" // this must be included before stdio
#include "database.h"
#include "pages.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

GtkWidget *APP_WINDOW;
GtkWidget *APP_STACK;
GtkWidget *APP_OVERLAY;

static void activate(GtkApplication *app, gpointer user_data) {
  // init app's window
  APP_WINDOW = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(APP_WINDOW), "Hotel2000");
  gtk_window_set_default_size(GTK_WINDOW(APP_WINDOW), 1000, 900);
  gtk_window_set_resizable(GTK_WINDOW(APP_WINDOW), false);
  // add css
  GtkCssProvider *css_provider = gtk_css_provider_new();
  gtk_css_provider_load_from_string(
      css_provider, ".toast { background-color: #4B4B4B; color: "
                    "white; opacity: 0.9; padding: 10px;"
                    "border-radius: 10px; margin-bottom: 10px;"
                    "font-weight: bold; }"
                    ".header { font-size: 22pt; }");
  gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                                             GTK_STYLE_PROVIDER(css_provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);
  // init pages
  APP_OVERLAY = gtk_overlay_new();
  APP_STACK = init_pages();
  gtk_overlay_set_child(GTK_OVERLAY(APP_OVERLAY), APP_STACK);
  gtk_window_set_child(GTK_WINDOW(APP_WINDOW), APP_OVERLAY);

  gtk_window_present(GTK_WINDOW(APP_WINDOW));
}

void window_removed(GtkApplication *self, GtkWindow *window,
                    gpointer user_data) {
  // store app's conf
  store_conf();
  // free app's resources
  free_db_state();
}

int main(int argc, char **argv) {
  // init db state
  init_db_state();
  // gtk
  GtkApplication *app;
  int status;

  app = gtk_application_new("com.github.GachiLord.hotel2000",
                            G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  g_signal_connect(app, "window-removed", G_CALLBACK(window_removed), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
