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

static void handle_item_click(GtkListBox *self, GtkListBoxRow *row,
                              PersonArray **arr) {
  // TODO: implement user editor
  g_print("smth\n");
}

// UI

GtkWidget *search_guests_page() {
  return search_guests_component(handle_item_click, false);
}
