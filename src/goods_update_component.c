#include "common.h"
#include "goods.h"
#include <gtk/gtk.h>

// state

typedef struct {
  // component
  GtkWidget *component;
  GtkWidget *parent;
  GtkEditable *title;
  GtkSpinButton *price;
  // data
  const char *item_id;
  ItemUpdateHandler update_handler;
  gpointer update_handler_data;
} WidgetState;

// logic

static Item read_item(const char *item_id) {
  char *query;
  asprintf(&query, "SELECT * FROM read_item(%s)", item_id);

  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);

  if (handle_db_error(res, "Не удалось выполнить запрос") == false) {
    return (Item){};
  }

  char *id = g_strdup(PQgetvalue(res, 0, 0));
  char *title = g_strdup(PQgetvalue(res, 0, 1));
  char *price = PQgetvalue(res, 0, 2);

  double price_numeric;
  sscanf(price, "%lf", &price_numeric);

  PQclear(res);

  return (Item){id, title, price_numeric};
}

static bool update_item(const char *item_id, const char *title, double price) {
  char *query;
  asprintf(&query, "call update_item(%s, '%s', %lf::float8::numeric::money)",
           item_id, title, price);

  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);

  if (handle_db_error(res, "Не удалось выполнить запрос") == false) {
    return false;
  }
  PQclear(res);
  show_toast("Изменения сохранены");
  return true;
}

static void handle_update(GtkWidget *_, gpointer state) {
  WidgetState *s = (WidgetState *)state;

  const char *title = gtk_editable_get_text(s->title);
  double price = gtk_spin_button_get_value_as_int(s->price);
  bool res = update_item(s->item_id, title, price);

  if (res && s->update_handler != NULL) {
    s->update_handler(title, price, s->update_handler_data);
  }
}

static void handle_close(GtkWidget *_, gpointer state) {
  WidgetState *s = (WidgetState *)state;

  remove_widget_from_main_stack(s->component, s->parent);
}

static void handle_destroy(GtkWidget *_, gpointer state) { g_free(state); }

// UI
GtkWidget *goods_update_component(ItemUpdateHandler update_handler,
                                  const char *item_id, GtkWidget *parent,
                                  gpointer update_handler_data) {
  Item current_item = read_item(item_id);

  GtkWidget *box_wrapper = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_margin_top(box_wrapper, 10);

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
  gtk_widget_set_margin_top(box, 40);
  gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(box, 300, -1);
  gtk_box_append(GTK_BOX(box_wrapper), box);

  GtkWidget *close_button = gtk_button_new_with_label("Закрыть");
  gtk_widget_set_margin_start(close_button, 10);
  gtk_widget_set_halign(close_button, GTK_ALIGN_START);
  gtk_box_prepend(GTK_BOX(box_wrapper), close_button);

  GtkWidget *title = gtk_entry_new();
  gtk_widget_set_sensitive(title, DB_STATE->permission_level >= MANAGER);
  gtk_entry_set_placeholder_text(GTK_ENTRY(title), "Название товара");
  gtk_widget_set_size_request(title, -1, 40);
  gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(title)),
                            current_item.title, -1);
  gtk_box_append(GTK_BOX(box), title);

  GtkAdjustment *adjustment =
      gtk_adjustment_new(current_item.price, 0.0, 100000.0, 1.0, 5.0, 0.0);
  GtkWidget *price_button = gtk_spin_button_new(adjustment, 1.0, 0);
  gtk_widget_set_sensitive(price_button, DB_STATE->permission_level >= MANAGER);
  gtk_box_append(GTK_BOX(box), price_button);

  GtkWidget *save_button = gtk_button_new_with_label("Сохранить");
  gtk_widget_set_size_request(save_button, -1, 40);
  gtk_widget_set_halign(save_button, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(save_button, GTK_ALIGN_START);

  if (DB_STATE->permission_level >= MANAGER)
    gtk_box_append(GTK_BOX(box), save_button);

  // free item cause no longer needed
  free_item_fields(&current_item);
  // init state

  WidgetState *state = g_malloc(sizeof(WidgetState));
  *state = (WidgetState){box_wrapper,
                         parent,
                         GTK_EDITABLE(title),
                         GTK_SPIN_BUTTON(price_button),
                         item_id,
                         update_handler,
                         update_handler_data};

  // handle signals

  g_signal_connect(close_button, "clicked", G_CALLBACK(handle_close), state);

  g_signal_connect(save_button, "clicked", G_CALLBACK(handle_update), state);

  g_signal_connect(box, "destroy", G_CALLBACK(handle_destroy), state);

  return box_wrapper;
}
