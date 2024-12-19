#include "common.h"
#include "database.h"
#include "pages.h"
#include <gtk/gtk.h>

static void login_handler(GtkWidget *widget, gpointer data) {
  if (db_connect()) {
    // set pages on MAIN_STACK
    set_pages_according_to_permission_level();
    // show home
    gtk_stack_set_visible_child_name(APP_STACK, "home");
    g_print("connected to db %s\n", DB_STATE->database);
    show_toast("Вход в систему");
  } else {
    show_toast("Неверный логин или пароль");
  }
}

#define DB_HANDLER(field)                                                      \
  {                                                                            \
    const gchar *text = gtk_editable_get_text(GTK_EDITABLE(widget));           \
    const gsize len = g_utf8_strlen(text, -1);                                 \
                                                                               \
    if (len > 0) {                                                             \
      DB_STATE->field = g_realloc(DB_STATE->field, len + 1);                   \
      g_stpcpy(DB_STATE->field, text);                                         \
    } else {                                                                   \
      DB_STATE->field[0] = '\0';                                               \
    }                                                                          \
  }

static void user_handler(GtkWidget *widget, gpointer data) { DB_HANDLER(user) }

static void password_handler(GtkWidget *widget, gpointer data) {
  DB_HANDLER(password)
}

static void port_handler(GtkWidget *widget, gpointer data) { DB_HANDLER(port) }

static void host_handler(GtkWidget *widget, gpointer data) { DB_HANDLER(host) }

static void database_handler(GtkWidget *widget,
                             gpointer data){DB_HANDLER(database)}

GtkWidget *init_login_page() {
  // init dialog
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
  gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(box, 300, -1);

  // add widgets to the box
  GtkWidget *user = gtk_entry_new();
  gtk_widget_set_size_request(user, -1, 40);
  gtk_entry_set_placeholder_text(GTK_ENTRY(user), "Логин");
  gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(user)),
                            DB_STATE->user, -1);
  g_signal_connect(user, "changed", G_CALLBACK(user_handler), NULL);
  gtk_box_append(GTK_BOX(box), user);

  GtkWidget *password = gtk_entry_new();
  gtk_widget_set_size_request(password, -1, 40);
  gtk_entry_set_visibility(GTK_ENTRY(password), false);
  gtk_entry_set_placeholder_text(GTK_ENTRY(password), "Пароль");
  gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(password)),
                            DB_STATE->password, -1);
  g_signal_connect(password, "changed", G_CALLBACK(password_handler), NULL);
  gtk_box_append(GTK_BOX(box), password);

  GtkWidget *port = gtk_entry_new();
  gtk_widget_set_size_request(port, -1, 40);
  gtk_entry_set_placeholder_text(GTK_ENTRY(port), "Порт");
  gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(port)),
                            DB_STATE->port, -1);
  g_signal_connect(port, "changed", G_CALLBACK(port_handler), NULL);
  gtk_box_append(GTK_BOX(box), port);

  GtkWidget *host = gtk_entry_new();
  gtk_widget_set_size_request(host, -1, 40);
  gtk_entry_set_placeholder_text(GTK_ENTRY(host), "Адрес сервера");
  gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(host)),
                            DB_STATE->host, -1);
  g_signal_connect(host, "changed", G_CALLBACK(host_handler), NULL);
  gtk_box_append(GTK_BOX(box), host);

  GtkWidget *database = gtk_entry_new();
  gtk_widget_set_size_request(database, -1, 40);
  gtk_entry_set_placeholder_text(GTK_ENTRY(database), "База данных");
  gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(database)),
                            DB_STATE->database, -1);
  g_signal_connect(database, "changed", G_CALLBACK(database_handler), NULL);
  gtk_box_append(GTK_BOX(box), database);

  GtkWidget *button = gtk_button_new_with_label("Войти");
  gtk_widget_set_size_request(button, -1, 40);
  g_signal_connect(button, "clicked", G_CALLBACK(login_handler), NULL);
  gtk_widget_set_halign(button, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(button, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(box), button);
  // return created widget
  return box;
}
