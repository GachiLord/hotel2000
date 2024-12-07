#include "common.h"
#include "guests.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

// state

// logic

static void handle_item_click(Person guest, gpointer _) {
  add_widget_to_main_stack(guest_update_component(guest.id, HOME_WIDGET));
}

// UI

GtkWidget *search_guests_page() {
  return search_guests_component(handle_item_click, NULL, false, HOME_WIDGET);
}
