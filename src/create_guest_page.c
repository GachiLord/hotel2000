#include "common.h"
#include "database.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

// logic

static bool create_guest(const char *name, const char *phone,
                         const char *passport) {
  char *query;
  asprintf(&query, "call create_guest ('%s', '%s', '%s')", name, passport,
           phone);

  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);

  bool res_code = handle_db_error(res, "Не удалось создать гостя");

  if (res_code == true) {
    PQclear(res);
    show_toast("Гость создан");
  }

  return res_code;
}

// state

typedef struct {
  GtkWidget *name;
  GtkWidget *phone;
  GtkWidget *passport;
} WidgetState;

static WidgetState state;

// UI

static void create_handler(GtkWidget *_, gpointer __) {

  const char *name = gtk_editable_get_text(GTK_EDITABLE(state.name));
  const char *phone = gtk_editable_get_text(GTK_EDITABLE(state.phone));
  const char *passport = gtk_editable_get_text(GTK_EDITABLE(state.passport));

  if (create_guest(name, phone, passport)) {
    gtk_editable_delete_text(GTK_EDITABLE(state.name), 0, -1);
    gtk_editable_delete_text(GTK_EDITABLE(state.phone), 0, -1);
    gtk_editable_delete_text(GTK_EDITABLE(state.passport), 0, -1);
  }
}

GtkWidget *create_guests_page() {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
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
  state = (WidgetState){name, phone, passport};
  // handle create
  g_signal_connect(button, "clicked", G_CALLBACK(create_handler), NULL);

  return box;
}
