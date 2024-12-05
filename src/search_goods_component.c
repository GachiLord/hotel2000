#include "common.h"
#include "glib.h"
#include "goods.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

// state

typedef struct {
  GtkListBox *list;
  GtkFrame *frame;
  GtkWidget *component;

  const char *parent_name;
  ItemArray *items;
} WidgetState;

static WidgetState state;

// logic

static ItemArray *find_items_by_name(const char *title) {
  char *query;
  asprintf(&query, "SELECT * FROM find_goods('%s')", title);

  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);

  if (handle_db_error(res, "Не удалось выполнить запрос") != 0) {
    return NULL;
  }

  int len = PQntuples(res);

  if (len == 0) {
    PQclear(res);
    return NULL;
  }

  ItemArray *arr = g_malloc(sizeof(ItemArray));
  Item *guests = g_malloc(len * sizeof(Item));

  for (int i = 0; i < len; i++) {
    const char *id = PQgetvalue(res, i, 0);
    const char *title = PQgetvalue(res, i, 1);
    const char *price = PQgetvalue(res, i, 2);
    double price_numeric;

    sscanf(price, "%lf", &price_numeric);

    guests[i] = (Item){g_strdup(id), g_strdup(title), price_numeric};
  }

  PQclear(res);
  *arr = (ItemArray){guests, len};

  return arr;
}

static void render_items() {
  if (state.items == NULL) {
    gtk_widget_set_visible(GTK_WIDGET(state.frame), false);
    return;
  }

  gtk_list_box_remove_all(state.list);
  gtk_widget_set_visible(GTK_WIDGET(state.frame), true);

  for (gsize i = 0; i < state.items->len; i++) {
    GtkWidget *item = gtk_center_box_new();
    gtk_widget_set_size_request(item, 400, 40);

    GtkWidget *title = gtk_label_new(state.items->arr[i].title);
    gtk_center_box_set_start_widget(GTK_CENTER_BOX(item), title);
    gtk_widget_set_margin_start(title, 10);

    char *price_text;
    asprintf(&price_text, "%f", state.items->arr[i].price);
    GtkWidget *price = gtk_label_new(price_text);
    gtk_widget_set_margin_end(price, 10);
    g_free(price_text);

    gtk_center_box_set_end_widget(GTK_CENTER_BOX(item), price);

    gtk_list_box_append(GTK_LIST_BOX(state.list), item);
  }
}

static void handle_search(GtkWidget *widget, gpointer **__) {
  const char *query = gtk_editable_get_text(GTK_EDITABLE(widget));
  free_item_array(state.items);
  state.items = find_items_by_name(query);

  render_items();
}

static void handle_cancel(GtkWidget *_, gpointer **__) {
  gtk_stack_set_visible_child_name(APP_STACK, state.parent_name);
  gtk_stack_remove(APP_STACK, state.component);
}

static void handle_destroy(GtkWidget *_, gpointer __) {
  free_item_array(state.items);
}

// UI

GtkWidget *search_goods_component(ItemClickHandler on_item_click,
                                  bool handle_cancel, const char *parent_name) {
  // main containers
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  GtkWidget *list = gtk_list_box_new();
  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_widget_set_margin_top(box, 10);
  gtk_widget_set_visible(frame, false);
  gtk_frame_set_child(GTK_FRAME(frame), list);
  gtk_widget_set_halign(frame, GTK_ALIGN_CENTER);
  gtk_list_box_set_selection_mode(GTK_LIST_BOX(list), GTK_SELECTION_NONE);
  gtk_box_append(GTK_BOX(box), frame);

  // state
  state = (WidgetState){GTK_LIST_BOX(list), GTK_FRAME(frame), box, parent_name,
                        NULL};

  // controls

  GtkWidget *search = gtk_search_entry_new();
  gtk_box_prepend(GTK_BOX(box), search);
  gtk_widget_set_halign(search, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(search, 300, 40);

  // handle search
  g_signal_connect(search, "activate", G_CALLBACK(handle_search), NULL);

  // handle click

  if (on_item_click != NULL) {
    g_signal_connect(list, "row-activated", G_CALLBACK(on_item_click),
                     state.items);
  }

  if (handle_cancel) {
    GtkWidget *cancel_button = gtk_button_new_with_label("Отмена");
    gtk_widget_set_halign(cancel_button, GTK_ALIGN_START);
    gtk_widget_set_margin_start(cancel_button, 10);
    g_signal_connect(cancel_button, "clicked", G_CALLBACK(handle_cancel), NULL);
    gtk_box_prepend(GTK_BOX(box), cancel_button);
  }

  // handle widget destroy
  g_signal_connect(box, "destroy", G_CALLBACK(handle_destroy), NULL);

  return box;
}
