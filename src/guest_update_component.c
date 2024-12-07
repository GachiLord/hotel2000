#include "common.h"
#include "glib.h"
#include "goods.h"
#include "guests.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

// state

typedef struct {
  GtkWidget *component;
  GtkLabel *name;
  GtkLabel *passport;
  GtkLabel *phone;
  GtkWidget *parent;

  const char *guest_id;
} WidgetState;

static WidgetState state;

// logic

// manage guest

static Person *get_guest(const char *guest_id) {
  char *query;
  asprintf(&query, "SELECT * FROM read_guest(%s)", guest_id);

  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);

  if (handle_db_error(res, "Не удалось выполнить запрос") != 0) {
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

static int update_guest(const char *guest_id, const char *name,
                        const char *passport, const char *phone) {
  char *query;
  asprintf(&query, "call update_guest(%s, '%s', '%s', '%s')", guest_id, name,
           passport, phone);

  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);

  if (handle_db_error(res, "Не удалось выполнить запрос") != 0) {
    return -1;
  }
  PQclear(res);
  show_toast("Данные обновлены");
  return 0;
}

static void handle_save(GtkWidget *_, gpointer __) {
  const char *name = gtk_editable_get_text(GTK_EDITABLE(state.name));
  const char *passport = gtk_editable_get_text(GTK_EDITABLE(state.passport));
  const char *phone = gtk_editable_get_text(GTK_EDITABLE(state.phone));

  update_guest(state.guest_id, name, passport, phone);
}

// manage orders

static void handle_order_choose(GtkListBox *widget, GtkListBoxRow *row,
                                ItemArray **goods) {}

static void handle_add_order(GtkWidget *_, gpointer __) {
  // GtkWidget *search = search_goods_component(handle_order_choose, false,
  // state.component); gtk_stack_add_child(GTK_STACK search);
}

// handle destroy

static void handle_close(GtkWidget *_, gpointer __) {
  remove_widget_from_main_stack(state.component, state.parent);
}

// UI

GtkWidget *guest_update_component(const char *guest_id, GtkWidget *parent) {
  // fetch guest's data
  Person *guest = get_guest(guest_id);
  g_assert(guest != NULL);
  // main containers
  GtkWidget *main_box_wrapper = gtk_box_new(GTK_ORIENTATION_VERTICAL, 100);
  GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 300);
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

  gtk_widget_set_size_request(name, -1, 40);
  gtk_box_append(GTK_BOX(user_box), name);

  GtkWidget *phone = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(phone), "Телефон");
  gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(phone)),
                            guest->phone, -1);
  gtk_widget_set_size_request(phone, -1, 40);
  gtk_box_append(GTK_BOX(user_box), phone);

  GtkWidget *passport = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(passport), "Паспорт");
  gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(passport)),
                            guest->passport, -1);
  gtk_widget_set_size_request(passport, -1, 40);
  gtk_box_append(GTK_BOX(user_box), passport);

  // free guest cause no longer needed
  free_person(guest);

  GtkWidget *save_button = gtk_button_new_with_label("Сохранить");
  gtk_widget_set_size_request(save_button, -1, 40);
  gtk_widget_set_halign(save_button, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(save_button, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(user_box), save_button);

  // orders editor
  GtkWidget *order_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_size_request(order_box, 300, -1);
  gtk_box_append(GTK_BOX(main_box), order_box);

  GtkWidget *order_label = gtk_label_new("Заказы");
  gtk_widget_add_css_class(order_label, "header");
  gtk_widget_set_halign(order_label, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(order_box), order_label);

  GtkWidget *add_order_button = gtk_button_new_with_label("Добавить");
  gtk_widget_set_size_request(add_order_button, -1, 40);
  gtk_widget_set_halign(add_order_button, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(add_order_button, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(order_box), add_order_button);

  // close button

  GtkWidget *close_button = gtk_button_new_with_label("Назад");
  gtk_widget_set_halign(close_button, GTK_ALIGN_START);
  gtk_widget_set_margin_start(close_button, 10);
  gtk_box_prepend(GTK_BOX(main_box_wrapper), close_button);

  // init state
  state = (WidgetState){main_box_wrapper, GTK_LABEL(name), GTK_LABEL(passport),
                        GTK_LABEL(phone), parent,          guest_id};

  // handle signals

  g_signal_connect(save_button, "clicked", G_CALLBACK(handle_save), NULL);

  g_signal_connect(close_button, "clicked", G_CALLBACK(handle_close), NULL);

  g_signal_connect(add_order_button, "clicked", G_CALLBACK(handle_add_order),
                   NULL);

  return main_box_wrapper;
}
