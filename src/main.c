#include "database.h"
#include "home.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

GtkWidget *APP_WINDOW;
GtkWidget *APP_STACK;
GtkWidget *APP_OVERLAY;

static void activate(GtkApplication *app, gpointer user_data) {
  // init app's window
  APP_WINDOW = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(APP_WINDOW), "Hotel2000");
  gtk_window_set_default_size(GTK_WINDOW(APP_WINDOW), 1000, 700);
  // add css
  GtkCssProvider *css_provider = gtk_css_provider_new();
  gtk_css_provider_load_from_string(
      css_provider, ".toast { background-color: #4B4B4B; color: "
                    "white; opacity: 0.9; padding: 10px;"
                    "border-radius: 10px; margin-bottom: 10px;"
                    "font-weight: bold; }");
  gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                                             GTK_STYLE_PROVIDER(css_provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);
  // init pages
  APP_OVERLAY = gtk_overlay_new();
  APP_STACK = initHomePage();
  gtk_overlay_set_child(GTK_OVERLAY(APP_OVERLAY), APP_STACK);
  gtk_window_set_child(GTK_WINDOW(APP_WINDOW), APP_OVERLAY);

  gtk_window_present(GTK_WINDOW(APP_WINDOW));
}

int main(int argc, char **argv) {
  // init db state
  initDbState();
  // gtk
  GtkApplication *app;
  int status;

  app = gtk_application_new("com.github.GachiLord.hotel2000",
                            G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
