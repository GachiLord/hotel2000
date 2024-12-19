#include "common.h"
#include "goods.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>
#include <stdio.h>

// logic

static void handle_update(const char *title, double price, gpointer data) {
  GtkListBoxRow *row = GTK_LIST_BOX_ROW(data);
  GtkCenterBox *box = GTK_CENTER_BOX(gtk_list_box_row_get_child(row));

  GtkLabel *title_l = GTK_LABEL(gtk_center_box_get_start_widget(box));
  GtkLabel *price_l = GTK_LABEL(gtk_center_box_get_end_widget(box));

  gtk_label_set_text(title_l, title);

  char *price_s;
  asprintf(&price_s, "%lf", price);
  gtk_label_set_text(price_l, price_s);
  g_free(price_s);
}

static void handle_item_click(GtkListBoxRow *row, const Item item, gpointer _) {
  add_widget_to_main_stack(
      goods_update_component(handle_update, item.item_id, HOME_WIDGET, row));
}

// UI

GtkWidget *search_goods_page() {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_margin_top(box, DEFAULT_MARGIN_TOP);
  gtk_box_append(GTK_BOX(box), search_goods_component(handle_item_click, NULL,
                                                      false, HOME_WIDGET));

  return box;
}
