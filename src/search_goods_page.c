#include "common.h"
#include "goods.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

// state

// logic

static void handle_item_click(const Item item, gpointer _) {
  g_print("%s\n", item.title);
  // TODO:
}

// UI

GtkWidget *search_goods_page() {
  return search_goods_component(handle_item_click, NULL, false, HOME_WIDGET);
}
