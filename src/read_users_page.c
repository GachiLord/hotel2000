#include "common.h"
#include "user.h"
#include <glib.h>
#include <gtk/gtk.h>

// state

typedef struct {
  GtkFrame *frame;
  GtkListBox *list;
  UserArray *users;
} WidgetState;

static WidgetState state;

// logic

static UserArray *read_users() {
  PGresult *res = PQexec(DB_STATE->conn, "SELECT * FROM read_users()");

  if (handle_db_error(res, "Не удалось выполнить запрос") == false) {
    return NULL;
  }

  int len = PQntuples(res);

  if (len == 0) {
    PQclear(res);
    return NULL;
  }

  UserArray *arr = g_malloc(sizeof(UserArray));
  User *guests = g_malloc(len * sizeof(User));

  for (int i = 0; i < len; i++) {
    guests[i] = (User){g_strdup(PQgetvalue(res, i, 0))};
  }

  PQclear(res);
  *arr = (UserArray){guests, len};

  return arr;
}

static bool delete_user(const char *login) {
  char *query;
  asprintf(&query, "call delete_user('%s')", login);

  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);

  if (handle_db_error(res, "Недостаточно прав для удаления") == false) {
    return false;
  }
  PQclear(res);
  show_toast("Пользователь удален");
  return true;
}

static void handle_delete(GtkWidget *_, gpointer data) {
  int index = GPOINTER_TO_INT(data);
  if (delete_user(state.users->arr[index].login)) {
    GtkListBoxRow *row = gtk_list_box_get_row_at_index(state.list, index);
    GtkWidget *label = gtk_label_new("Удалено");
    gtk_widget_set_size_request(label, 500, 40);
    gtk_list_box_row_set_child(row, label);
  }
}

static void render_items(WidgetState *s) {
  if (s->users == NULL) {
    gtk_widget_set_visible(GTK_WIDGET(s->frame), false);
    return;
  }

  gtk_list_box_remove_all(s->list);
  gtk_widget_set_visible(GTK_WIDGET(s->frame), true);

  for (gsize i = 0; i < s->users->len; i++) {
    GtkWidget *item = gtk_center_box_new();
    gtk_widget_set_size_request(item, 500, 40);

    GtkWidget *title = gtk_label_new(s->users->arr[i].login);
    gtk_center_box_set_start_widget(GTK_CENTER_BOX(item), title);
    gtk_widget_set_margin_start(title, 10);

    GtkWidget *delete_button = gtk_button_new_with_label("Удалить");
    gtk_widget_set_margin_end(title, 10);
    if (DB_STATE->permission_level >= MANAGER)
      gtk_center_box_set_end_widget(GTK_CENTER_BOX(item), delete_button);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(handle_delete),
                     GINT_TO_POINTER(i));

    gtk_list_box_append(GTK_LIST_BOX(s->list), item);
  }
}

static void handle_mapped(GtkWidget *_, gpointer __) {
  if (state.users != NULL) {
    free_user_array(state.users);
  }

  state.users = read_users();
  render_items(&state);
}

static void handle_destroy(GtkWidget *_, gpointer __) {
  free_user_array(state.users);
}

// UI

GtkWidget *read_users_page() {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  GtkWidget *list = gtk_list_box_new();
  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_widget_set_margin_top(box, DEFAULT_MARGIN_TOP);
  gtk_widget_set_visible(frame, false);
  gtk_frame_set_child(GTK_FRAME(frame), list);
  gtk_widget_set_halign(frame, GTK_ALIGN_CENTER);
  gtk_list_box_set_selection_mode(GTK_LIST_BOX(list), GTK_SELECTION_NONE);
  gtk_box_append(GTK_BOX(box), frame);

  state = (WidgetState){GTK_FRAME(frame), GTK_LIST_BOX(list), NULL};

  g_signal_connect(box, "map", G_CALLBACK(handle_mapped), NULL);
  g_signal_connect(box, "destroy", G_CALLBACK(handle_destroy), NULL);

  return box;
}
