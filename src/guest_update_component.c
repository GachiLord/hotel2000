#include "common.h"
#include "glib-object.h"
#include "glib.h"
#include "goods.h"
#include "gtk/gtkshortcut.h"
#include "guests.h"
#include "user.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>
#include <stdio.h>

// state

typedef struct {
  // component
  GtkWidget *component;
  GtkWidget *parent;
  // user
  GtkLabel *name;
  GtkLabel *passport;
  GtkLabel *phone;
  // orders
  GtkListBox *list;
  GtkFrame *frame;
  // data
  const char *guest_id;
  gpointer update_handler_data;
  int update_handler_row_index;
  GuestUpdateHandler update_handler;
  // state
  OrderArray *orders;
} WidgetState;

// logic

// manage guest

static Person *get_guest(const char *guest_id) {
  char *query;
  asprintf(&query, "SELECT * FROM read_guest(%s)", guest_id);

  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);

  if (handle_db_error(res, "Не удалось выполнить запрос") == false) {
    return NULL;
  }

  Person *p = g_malloc(sizeof(Person));

  p->id = g_strdup(PQgetvalue(res, 0, 0));
  p->name = g_strdup(PQgetvalue(res, 0, 1));
  p->passport = g_strdup(PQgetvalue(res, 0, 2));
  p->phone = g_strdup(PQgetvalue(res, 0, 3));

  PQclear(res);

  return p;
}

static bool update_guest(const char *guest_id, const char *name,
                         const char *passport, const char *phone) {
  char *query;
  asprintf(&query, "call update_guest(%s, '%s', '%s', '%s')", guest_id, name,
           passport, phone);

  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);

  if (handle_db_error(res, "Не удалось выполнить запрос") == false) {
    return false;
  }
  PQclear(res);
  show_toast("Данные обновлены");
  return true;
}

static void handle_save(GtkWidget *_, gpointer state) {
  WidgetState *s = (WidgetState *)state;
  const char *name = gtk_editable_get_text(GTK_EDITABLE(s->name));
  const char *passport = gtk_editable_get_text(GTK_EDITABLE(s->passport));
  const char *phone = gtk_editable_get_text(GTK_EDITABLE(s->phone));

  if (update_guest(s->guest_id, name, passport, phone) &&
      s->update_handler != NULL) {
    s->update_handler(name, passport, phone, s->update_handler_data);
  }
}

static bool check_out(const char *guest_id) {
  char *query;
  asprintf(&query, "call check_out_guest(%s)", guest_id);

  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);

  if (handle_db_error(res, "Не удалось выполнить запрос") == false) {
    return false;
  }
  PQclear(res);
  show_toast("Операция выполнена");
  return true;
}

static void handle_checkout(GtkWidget *_, gpointer state) {
  WidgetState *s = (WidgetState *)state;

  if (check_out(s->guest_id) == false)
    return;

  remove_widget_from_main_stack(s->component, s->parent);
}

// manage orders

static OrderArray *get_guest_orders(const char *guest_id) {
  char *query;
  asprintf(&query, "SELECT * FROM read_guest_orders('%s')", guest_id);

  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);

  if (handle_db_error(res, "Не удалось выполнить запрос") == false) {
    return NULL;
  }

  int len = PQntuples(res);

  if (len == 0) {
    PQclear(res);
    return NULL;
  }

  OrderArray *arr = g_malloc(sizeof(OrderArray));
  Order *orders = g_malloc(len * sizeof(Order));

  for (int i = 0; i < len; i++) {
    orders[i].order_id = g_strdup(PQgetvalue(res, i, 0));

    orders[i].has_paid = PQgetvalue(res, i, 1)[0] == 't';

    double sold_for;
    sscanf(PQgetvalue(res, i, 2), "%lf", &sold_for);
    orders[i].sold_for = sold_for;

    int amount;
    sscanf(PQgetvalue(res, i, 3), "%d", &amount);
    orders[i].amount = amount;

    orders[i].title = g_strdup(PQgetvalue(res, i, 4));
  }

  PQclear(res);
  *arr = (OrderArray){orders, len};

  return arr;
}

static int create_order(const char *guest_id, const char *item_id,
                        double price) {
  char *query;
  asprintf(&query,
           "SELECT * FROM create_order(%s, %s, %lf::float8::numeric::money)",
           guest_id, item_id, price);

  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);

  if (handle_db_error(res, "Не удалось выполнить запрос") == false) {
    return -1;
  }

  int id;
  sscanf(PQgetvalue(res, 0, 0), "%d", &id);
  PQclear(res);

  return id;
}

static bool update_order(const char *order_id, double sold_for, int amount,
                         bool has_paid) {
  char *query;
  asprintf(&query,
           "call update_order(%s, %lf::float8::numeric::money, %d, "
           "%d::int::boolean)",
           order_id, sold_for, amount, has_paid);

  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);

  if (handle_db_error(res, "Не удалось выполнить запрос") == false) {
    return false;
  }

  show_toast("Изменения сохранены");
  PQclear(res);

  return true;
}

static bool delete_order(const char *order_id) {
  char *query;
  asprintf(&query, "call delete_order(%s)", order_id);

  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);

  if (handle_db_error(res, "Не удалось выполнить запрос") == false) {
    return false;
  }

  show_toast("Заказ удален");
  PQclear(res);

  return true;
}

static void handle_item_save(GtkWidget *widget, gpointer state) {
  WidgetState *s = (WidgetState *)state;

  GtkWidget *grid = gtk_widget_get_parent(widget);
  GtkListBoxRow *row = GTK_LIST_BOX_ROW(gtk_widget_get_parent(grid));

  GtkGrid *g = GTK_GRID(grid);

  const char *order_id =
      s->orders->arr[gtk_list_box_row_get_index(row)].order_id;

  double price = gtk_spin_button_get_value(
      GTK_SPIN_BUTTON(gtk_grid_get_child_at(g, 2, 0)));
  int amount = gtk_spin_button_get_value_as_int(
      GTK_SPIN_BUTTON(gtk_grid_get_child_at(g, 4, 0)));
  bool has_paid = gtk_check_button_get_active(
      GTK_CHECK_BUTTON(gtk_grid_get_child_at(g, 5, 0)));
  update_order(order_id, price, amount, has_paid);
}

static void handle_item_delete(GtkWidget *widget, gpointer state) {
  WidgetState *s = (WidgetState *)state;

  GtkWidget *grid = gtk_widget_get_parent(widget);
  GtkWidget *row = gtk_widget_get_parent(grid);
  GtkWidget *list = gtk_widget_get_parent(GTK_WIDGET(row));

  int row_index = gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(row));
  const char *order_id = s->orders->arr[row_index].order_id;

  // remove from db
  if (delete_order(order_id) == false)
    return;
  // update state
  remove_order_array(s->orders, row_index);
  // render
  gtk_list_box_remove(GTK_LIST_BOX(list), row);
  if (s->orders->len == 0) {
    gtk_widget_set_visible(GTK_WIDGET(s->frame), false);
  }
}

static GtkWidget *new_order_list_item(Order order, WidgetState *s) {

  GtkWidget *item = gtk_grid_new();
  gtk_widget_set_halign(item, GTK_ALIGN_END);
  gtk_widget_set_size_request(item, 500, 50);

  GtkWidget *title = gtk_label_new(order.title);
  gtk_widget_set_margin_start(title, 10);
  gtk_widget_set_margin_top(title, 10);
  gtk_grid_attach(GTK_GRID(item), title, 0, 0, 1, 1);

  GtkWidget *price = gtk_label_new("Цена:");
  gtk_widget_set_margin_start(price, 10);
  gtk_widget_set_margin_top(price, 10);
  gtk_grid_attach_next_to(GTK_GRID(item), price, title, GTK_POS_RIGHT, 1, 1);

  GtkAdjustment *price_adjustment =
      gtk_adjustment_new(order.sold_for, 0.0, 100000.0, 1.0, 5.0, 0.0);
  GtkWidget *price_button = gtk_spin_button_new(price_adjustment, 1.0, 0);
  gtk_widget_set_sensitive(price_button, DB_STATE->permission_level > VIEWER);
  gtk_widget_set_margin_start(price_button, 10);
  gtk_widget_set_margin_top(price_button, 10);
  gtk_grid_attach_next_to(GTK_GRID(item), price_button, price, GTK_POS_RIGHT, 1,
                          1);

  GtkWidget *amount = gtk_label_new("Количество:");
  gtk_widget_set_margin_start(amount, 10);
  gtk_widget_set_margin_top(amount, 10);
  gtk_grid_attach_next_to(GTK_GRID(item), amount, price_button, GTK_POS_RIGHT,
                          1, 1);

  GtkAdjustment *amount_adjustment =
      gtk_adjustment_new(order.amount, 1.0, 100000.0, 1.0, 5.0, 0.0);
  GtkWidget *amount_button = gtk_spin_button_new(amount_adjustment, 1.0, 0);
  gtk_widget_set_sensitive(amount_button, DB_STATE->permission_level > VIEWER);
  gtk_widget_set_margin_start(amount_button, 10);
  gtk_widget_set_margin_top(amount_button, 10);
  gtk_grid_attach_next_to(GTK_GRID(item), amount_button, amount, GTK_POS_RIGHT,
                          1, 1);

  GtkWidget *has_paid_button = gtk_check_button_new_with_label("Оплачено");
  gtk_widget_set_sensitive(has_paid_button,
                           DB_STATE->permission_level > VIEWER);
  gtk_check_button_set_active(GTK_CHECK_BUTTON(has_paid_button),
                              order.has_paid);
  gtk_widget_set_margin_start(has_paid_button, 10);
  gtk_widget_set_margin_top(has_paid_button, 10);
  gtk_grid_attach_next_to(GTK_GRID(item), has_paid_button, amount_button,
                          GTK_POS_RIGHT, 1, 1);

  if (DB_STATE->permission_level > VIEWER) {
    GtkWidget *save_button = gtk_button_new_with_label("Сохранить");
    gtk_widget_set_margin_start(save_button, 10);
    gtk_widget_set_margin_top(save_button, 10);
    gtk_grid_attach_next_to(GTK_GRID(item), save_button, has_paid_button,
                            GTK_POS_RIGHT, 1, 1);
    g_signal_connect(save_button, "clicked", G_CALLBACK(handle_item_save), s);

    GtkWidget *delete_button = gtk_button_new_with_label("Удалить");
    gtk_widget_set_margin_start(delete_button, 10);
    gtk_widget_set_margin_top(delete_button, 10);
    gtk_grid_attach_next_to(GTK_GRID(item), delete_button, save_button,
                            GTK_POS_RIGHT, 1, 1);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(handle_item_delete),
                     s);
  }
  return item;
}

static void render_orders(WidgetState *s) {
  if (s->orders == NULL) {
    gtk_widget_set_visible(GTK_WIDGET(s->frame), false);
    return;
  }

  gtk_list_box_remove_all(s->list);
  gtk_widget_set_visible(GTK_WIDGET(s->frame), true);

  for (gsize i = 0; i < s->orders->len; i++) {
    Order order = s->orders->arr[i];

    gtk_list_box_append(GTK_LIST_BOX(s->list), new_order_list_item(order, s));
  }
}

static void handle_order_choose(GtkListBoxRow *_, const Item item,
                                gpointer state) {
  WidgetState *s = (WidgetState *)state;
  // create order in db
  int order_id = create_order(s->guest_id, item.item_id, item.price);
  if (order_id < 0)
    return;
  // update state
  Order order;
  asprintf(&order.order_id, "%d", order_id);
  order.title = g_strdup(item.title);
  order.sold_for = item.price;
  order.has_paid = false;
  if (s->orders == NULL) {
    s->orders = new_order_array(1);
    s->orders->arr[0] = order;
  } else {
    push_order_array(s->orders, order);
  }

  // render
  show_toast("Товар добавлен");
  gtk_list_box_append(GTK_LIST_BOX(s->list), new_order_list_item(order, s));
  gtk_widget_set_visible(GTK_WIDGET(s->frame), true);
}

static void handle_add_order(GtkWidget *_, gpointer state) {
  WidgetState *s = (WidgetState *)state;
  GtkWidget *search =
      search_goods_component(handle_order_choose, state, true, s->component);
  add_widget_to_main_stack(search);
}

// handle destroy

static void handle_close(GtkWidget *_, gpointer state) {
  WidgetState *s = (WidgetState *)state;
  remove_widget_from_main_stack(s->component, s->parent);
}

static void handle_destroy(GtkWidget *_, gpointer state) {
  WidgetState *s = (WidgetState *)state;
  free_order_array(s->orders);
  g_free(s);
}

// UI

GtkWidget *guest_update_component(const char *guest_id, GtkWidget *parent,
                                  GuestUpdateHandler handle_update,
                                  gpointer data) {
  // fetch guest's data
  Person *guest = get_guest(guest_id);
  g_assert(guest != NULL);
  // TODO: handle none existent user
  OrderArray *current_orders = get_guest_orders(guest_id);
  // main containers
  GtkWidget *main_box_wrapper = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 150);
  gtk_widget_set_halign(main_box, GTK_ALIGN_CENTER);
  gtk_widget_set_margin_top(main_box_wrapper, 10);
  gtk_box_append(GTK_BOX(main_box_wrapper), main_box);

  // user editor
  GtkWidget *user_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_size_request(user_box, 300, -1);
  gtk_box_append(GTK_BOX(main_box), user_box);

  char *id;
  asprintf(&id, "Гость №%s", guest_id);
  GtkWidget *guest_label = gtk_label_new(id);
  g_free(id);
  gtk_widget_add_css_class(guest_label, "header");
  gtk_widget_set_halign(guest_label, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(user_box), guest_label);

  GtkWidget *name = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(name), "Имя");
  gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(name)), guest->name,
                            -1);
  gtk_widget_set_sensitive(name, DB_STATE->permission_level > VIEWER);

  gtk_widget_set_size_request(name, -1, 40);
  gtk_box_append(GTK_BOX(user_box), name);

  GtkWidget *phone = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(phone), "Телефон");
  gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(phone)),
                            guest->phone, -1);
  gtk_widget_set_size_request(phone, -1, 40);
  gtk_box_append(GTK_BOX(user_box), phone);
  gtk_widget_set_sensitive(phone, DB_STATE->permission_level > VIEWER);

  GtkWidget *passport = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(passport), "Паспорт");
  gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(passport)),
                            guest->passport, -1);
  gtk_widget_set_size_request(passport, -1, 40);
  gtk_widget_set_sensitive(passport, DB_STATE->permission_level > VIEWER);
  gtk_box_append(GTK_BOX(user_box), passport);

  // free guest cause no longer needed
  free_person(guest);

  GtkWidget *user_action_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
  gtk_widget_set_halign(user_action_box, GTK_ALIGN_CENTER);

  GtkWidget *save_button = gtk_button_new_with_label("Сохранить");
  gtk_widget_set_size_request(save_button, -1, 40);
  gtk_widget_set_halign(save_button, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(save_button, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(user_action_box), save_button);

  GtkWidget *checkout_button = gtk_button_new_with_label("Выселить");
  gtk_widget_set_size_request(checkout_button, -1, 40);
  gtk_widget_set_halign(checkout_button, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(checkout_button, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(user_action_box), checkout_button);

  if (DB_STATE->permission_level > VIEWER)
    gtk_box_append(GTK_BOX(user_box), user_action_box);

  // orders editor
  GtkWidget *order_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_box_append(GTK_BOX(main_box), order_box);

  GtkWidget *order_label = gtk_label_new("Заказы");
  gtk_widget_add_css_class(order_label, "header");
  gtk_widget_set_halign(order_label, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(order_box), order_label);

  GtkWidget *list = gtk_list_box_new();
  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_widget_set_visible(frame, false);
  gtk_frame_set_child(GTK_FRAME(frame), list);
  gtk_list_box_set_selection_mode(GTK_LIST_BOX(list), GTK_SELECTION_NONE);
  gtk_box_append(GTK_BOX(order_box), frame);

  GtkWidget *add_order_button = gtk_button_new_with_label("Добавить");
  gtk_widget_set_size_request(add_order_button, -1, 40);
  gtk_widget_set_halign(add_order_button, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(add_order_button, GTK_ALIGN_START);

  if (DB_STATE->permission_level > VIEWER)
    gtk_box_append(GTK_BOX(order_box), add_order_button);

  // close button

  GtkWidget *close_button = gtk_button_new_with_label("Назад");
  gtk_widget_set_halign(close_button, GTK_ALIGN_START);
  gtk_widget_set_margin_start(close_button, 10);
  gtk_box_prepend(GTK_BOX(main_box_wrapper), close_button);

  // init state
  WidgetState *state = g_malloc(sizeof(WidgetState));
  state->component = main_box_wrapper;
  state->name = GTK_LABEL(name);
  state->passport = GTK_LABEL(passport);
  state->phone = GTK_LABEL(phone);
  state->list = GTK_LIST_BOX(list);
  state->frame = GTK_FRAME(frame);
  state->parent = parent;
  state->guest_id = guest_id;
  state->orders = current_orders;
  state->update_handler = handle_update;
  state->update_handler_data = data;

  // render orders

  render_orders(state);

  // handle signals

  g_signal_connect(checkout_button, "clicked", G_CALLBACK(handle_checkout),
                   state);

  g_signal_connect(save_button, "clicked", G_CALLBACK(handle_save), state);

  g_signal_connect(close_button, "clicked", G_CALLBACK(handle_close), state);

  g_signal_connect(add_order_button, "clicked", G_CALLBACK(handle_add_order),
                   state);

  g_signal_connect(main_box_wrapper, "destroy", G_CALLBACK(handle_destroy),
                   state);

  return main_box_wrapper;
}
