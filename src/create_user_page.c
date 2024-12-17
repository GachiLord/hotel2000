#include "common.h"
#include "gtk/gtk.h"
#include "user.h"

// logic

static int create_user(const char *login, PermissionLevel role,
                       const char *password) {
  char *query;
  asprintf(&query, "call create_user ('%s', '%s', %d)", login, password, role);

  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);

  int res_code = handle_db_error(res, "Не удалось создать пользователя");

  if (res_code == 0) {
    PQclear(res);
    show_toast("Пользователь создан");
  }

  return res_code;
}

// state

typedef struct {
  GtkEditable *name;
  GtkCheckButton *viewer;
  GtkCheckButton *hostess;
  GtkCheckButton *manager;
  GtkEditable *passport;
} WidgetState;

static WidgetState state;

// UI

static void create_handler(GtkWidget *_, gpointer __) {

  const char *name = gtk_editable_get_text(GTK_EDITABLE(state.name));
  const char *passport = gtk_editable_get_text(GTK_EDITABLE(state.passport));
  PermissionLevel level = VIEWER;

  if (gtk_check_button_get_active(state.hostess))
    level = HOSTESS;
  else if (gtk_check_button_get_active(state.manager))
    level = MANAGER;

  if (create_user(name, level, passport) == 0) {
    gtk_editable_delete_text(GTK_EDITABLE(state.name), 0, -1);
    gtk_editable_delete_text(GTK_EDITABLE(state.passport), 0, -1);
  }
}

GtkWidget *create_user_page() {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
  gtk_widget_set_margin_top(box, 50);
  gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(box, 300, -1);

  GtkWidget *login = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(login), "Логин");
  gtk_widget_set_size_request(login, -1, 40);
  gtk_box_append(GTK_BOX(box), login);

  GtkWidget *viewer = gtk_check_button_new_with_label("Просмотр");
  gtk_check_button_set_active(GTK_CHECK_BUTTON(viewer), true);
  gtk_box_append(GTK_BOX(box), viewer);

  GtkWidget *hostess = gtk_check_button_new_with_label("Хостесс");
  gtk_check_button_set_group(GTK_CHECK_BUTTON(hostess),
                             GTK_CHECK_BUTTON(viewer));
  gtk_box_append(GTK_BOX(box), hostess);

  GtkWidget *manager = gtk_check_button_new_with_label("Мэнеджер");
  gtk_check_button_set_group(GTK_CHECK_BUTTON(manager),
                             GTK_CHECK_BUTTON(hostess));
  gtk_box_append(GTK_BOX(box), manager);

  GtkWidget *password = gtk_entry_new();
  gtk_entry_set_visibility(GTK_ENTRY(password), false);
  gtk_entry_set_placeholder_text(GTK_ENTRY(password), "Пароль");
  gtk_widget_set_size_request(password, -1, 40);
  gtk_box_append(GTK_BOX(box), password);

  GtkWidget *button = gtk_button_new_with_label("Создать");
  gtk_widget_set_size_request(button, -1, 40);
  gtk_widget_set_halign(button, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(button, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(box), button);

  // define state
  state = (WidgetState){GTK_EDITABLE(login), GTK_CHECK_BUTTON(viewer),
                        GTK_CHECK_BUTTON(hostess), GTK_CHECK_BUTTON(manager),
                        GTK_EDITABLE(password)};
  // handle create
  g_signal_connect(button, "clicked", G_CALLBACK(create_handler), NULL);

  return box;
}
