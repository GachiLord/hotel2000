#include "common.h"
#include "goods.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

// state

// logic

static void handle_item_click(const Item item, gpointer _) {
  g_print("%s\n", item.title);
  // int index = gtk_list_box_row_get_index(row);
  // const char *guest_id = (*arr)->guests[index].id;
  // add_widget_to_main_stack(guest_update_component(guest_id, "home"), NULL);
}

// UI

GtkWidget *search_goods_page() {
  return search_goods_component(handle_item_click, NULL, false, HOME_WIDGET);
}
