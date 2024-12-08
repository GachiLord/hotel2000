#include "common.h"
#include "glib.h"
#include "goods.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

// state

typedef struct {
  // component
  GtkListBox *list;
  GtkFrame *frame;
  GtkWidget *component;
  GtkWidget *parent;
  // callback
  ItemClickHandler item_click_handler;
  gpointer component_data;
  // data
  ItemArray *items;
} WidgetState;

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

static void render_items(WidgetState *s) {
  if (s->items == NULL) {
    gtk_widget_set_visible(GTK_WIDGET(s->frame), false);
    return;
  }

  gtk_list_box_remove_all(s->list);
  gtk_widget_set_visible(GTK_WIDGET(s->frame), true);

  for (gsize i = 0; i < s->items->len; i++) {
    GtkWidget *item = gtk_center_box_new();
    gtk_widget_set_size_request(item, 500, 40);

    GtkWidget *title = gtk_label_new(s->items->arr[i].title);
    gtk_center_box_set_start_widget(GTK_CENTER_BOX(item), title);
    gtk_widget_set_margin_start(title, 10);

    char *price_text;
    asprintf(&price_text, "%f", s->items->arr[i].price);
    GtkWidget *price = gtk_label_new(price_text);
    gtk_widget_set_margin_end(price, 10);
    g_free(price_text);

    gtk_center_box_set_end_widget(GTK_CENTER_BOX(item), price);

    gtk_list_box_append(GTK_LIST_BOX(s->list), item);
  }
}

static void handle_search(GtkWidget *widget, gpointer state) {
  WidgetState *s = (WidgetState *)state;
  const char *query = gtk_editable_get_text(GTK_EDITABLE(widget));
  free_item_array(s->items);
  s->items = find_items_by_name(query);

  render_items(s);
}

static void handle_map(GtkWidget *_, gpointer state) {
  WidgetState *s = (WidgetState *)state;
  if (s->items == NULL) {
    s->items = find_items_by_name("");
    render_items(s);
  }
}

static void handle_item_click(GtkListBox *_, GtkListBoxRow *row,
                              gpointer state) {
  WidgetState *s = (WidgetState *)state;
  Item item = s->items->arr[gtk_list_box_row_get_index(row)];

  s->item_click_handler(item, s->component_data);
}

static void handle_cancel(GtkWidget *_, gpointer state) {
  WidgetState *s = (WidgetState *)state;
  gtk_stack_set_visible_child(APP_STACK, s->parent);
  gtk_stack_remove(APP_STACK, s->component);
}

static void handle_destroy(GtkWidget *_, gpointer state) {
  WidgetState *s = (WidgetState *)state;
  free_item_array(s->items);
  g_free(s);
}

// UI

GtkWidget *search_goods_component(ItemClickHandler on_item_click, gpointer data,
                                  bool should_handle_cancel,
                                  GtkWidget *parent) {
  // main containers
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  GtkWidget *list = gtk_list_box_new();
  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_widget_set_margin_top(box, 50);
  gtk_widget_set_visible(frame, false);
  gtk_frame_set_child(GTK_FRAME(frame), list);
  gtk_widget_set_halign(frame, GTK_ALIGN_CENTER);
  gtk_list_box_set_selection_mode(GTK_LIST_BOX(list), GTK_SELECTION_NONE);
  gtk_box_append(GTK_BOX(box), frame);

  // state
  WidgetState *state = g_malloc(sizeof(WidgetState));
  *state = (WidgetState){GTK_LIST_BOX(list),
                         GTK_FRAME(frame),
                         box,
                         parent,
                         on_item_click,
                         data,
                         NULL};

  // controls

  GtkWidget *search = gtk_search_entry_new();
  gtk_box_prepend(GTK_BOX(box), search);
  gtk_widget_set_halign(search, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(search, 500, 40);

  // handle search
  g_signal_connect(search, "activate", G_CALLBACK(handle_search), state);

  // handle click

  if (on_item_click != NULL) {
    g_signal_connect(list, "row-activated", G_CALLBACK(handle_item_click),
                     state);
  }

  if (should_handle_cancel) {
    GtkWidget *cancel_button = gtk_button_new_with_label("Отмена");
    gtk_widget_set_halign(cancel_button, GTK_ALIGN_START);
    gtk_widget_set_margin_start(cancel_button, 10);
    g_signal_connect(cancel_button, "clicked", G_CALLBACK(handle_cancel),
                     state);
    gtk_box_prepend(GTK_BOX(box), cancel_button);
  }

  // handle show

  g_signal_connect(box, "map", G_CALLBACK(handle_map), state);

  // handle widget destroy
  g_signal_connect(box, "destroy", G_CALLBACK(handle_destroy), state);

  return box;
}
