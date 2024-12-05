#include "common.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

// state

typedef struct {
  GtkEditable *title;
  GtkSpinButton *spin_button;
} WidgetState;

WidgetState state;

// logic

static int create_item(const char *title, double price) {
  char *query;
  asprintf(&query, "call create_item('%s', %f::float8::numeric::money)", title,
           price);

  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);

  if (handle_db_error(res, "Не удалось выполнить запрос") != 0) {
    return -1;
  }
  PQclear(res);
  show_toast("Товар успешно создан");
  return 0;
}

static void handle_create(GtkWidget *_, gpointer __) {
  int res = create_item(gtk_editable_get_text(state.title),
                        gtk_spin_button_get_value_as_int(state.spin_button));

  if (res == 0) {
    gtk_editable_delete_text(state.title, 0, -1);
    gtk_spin_button_set_value(state.spin_button, 1000.0);
  }
}

// UI

GtkWidget *create_goods_page() {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
  gtk_widget_set_margin_top(box, 50);
  gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(box, 300, -1);

  GtkWidget *title = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(title), "Название товара");
  gtk_widget_set_size_request(title, -1, 40);
  gtk_box_append(GTK_BOX(box), title);

  GtkAdjustment *adjustment =
      gtk_adjustment_new(1000.0, 0.0, 100000.0, 1.0, 5.0, 0.0);
  GtkWidget *price_button = gtk_spin_button_new(adjustment, 1.0, 0);
  gtk_box_append(GTK_BOX(box), price_button);

  GtkWidget *create_button = gtk_button_new_with_label("Создать");
  gtk_widget_set_size_request(create_button, -1, 40);
  gtk_widget_set_halign(create_button, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(create_button, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(box), create_button);

  // init state

  state = (WidgetState){GTK_EDITABLE(title), GTK_SPIN_BUTTON(price_button)};

  // handle signals

  g_signal_connect(create_button, "clicked", G_CALLBACK(handle_create), NULL);

  return box;
}
