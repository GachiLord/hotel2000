#include "common.h"
#include "database.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

extern DbState *DB_STATE;

// logic

static int create_guest(const char *name, const char *phone,
                        const char *passport) {
  char *query;
  asprintf(&query, "call create_guest ('%s', '%s', '%s')", name, phone,
           passport);

  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);

  int res_code = handle_db_error(res, "Не удалось создать гостя");

  if (res_code == 0) {
    PQclear(res);
    show_toast("Гость создан");
  }

  return res_code;
}

// state

struct {
  GtkWidget *name;
  GtkWidget *phone;
  GtkWidget *passport;
} typedef WidgetState;

// UI

static void handle_destroy(GtkWidget *_, gpointer data) { g_free(data); }

static void create_handler(GtkWidget *_, gpointer data) {
  WidgetState *s = (WidgetState *)data;

  const char *name = gtk_editable_get_text(GTK_EDITABLE(s->name));
  const char *phone = gtk_editable_get_text(GTK_EDITABLE(s->phone));
  const char *passport = gtk_editable_get_text(GTK_EDITABLE(s->passport));

  if (create_guest(name, phone, passport) == 0) {
    gtk_editable_delete_text(GTK_EDITABLE(s->name), 0, -1);
    gtk_editable_delete_text(GTK_EDITABLE(s->phone), 0, -1);
    gtk_editable_delete_text(GTK_EDITABLE(s->passport), 0, -1);
  }
}

GtkWidget *create_guests_page() {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
  gtk_widget_set_margin_top(box, 50);
  gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(box, 300, -1);

  GtkWidget *name = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(name), "Имя");
  gtk_widget_set_size_request(name, -1, 40);
  gtk_box_append(GTK_BOX(box), name);

  GtkWidget *phone = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(phone), "Телефон");
  gtk_widget_set_size_request(phone, -1, 40);
  gtk_box_append(GTK_BOX(box), phone);

  GtkWidget *passport = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(passport), "Паспорт");
  gtk_widget_set_size_request(passport, -1, 40);
  gtk_box_append(GTK_BOX(box), passport);

  GtkWidget *button = gtk_button_new_with_label("Создать");
  gtk_widget_set_size_request(button, -1, 40);
  gtk_widget_set_halign(button, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(button, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(box), button);

  // define state
  WidgetState *state = g_malloc(sizeof(WidgetState));
  *state = (WidgetState){name, phone, passport};
  // handle create
  g_signal_connect(button, "clicked", G_CALLBACK(create_handler), state);
  // handle destroy
  g_signal_connect(box, "destroy", G_CALLBACK(handle_destroy), state);

  return box;
}
