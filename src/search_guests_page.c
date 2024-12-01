#include "common.h"
#include "glib.h"
#include "guests.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

// state

typedef struct {
  GtkWidget *list;
  GtkWidget *frame;
} WidgetState;

// logic

static void handle_item_click(GtkWidget *self, GtkListBoxRow *row,
                              gpointer data) {
  // TODO: implement user editor
  g_print("smth\n");
}

// UI

GtkWidget *search_guests_page() {
  return search_guests_component(G_CALLBACK(handle_item_click), NULL);
}
