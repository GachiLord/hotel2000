#include "common.h"
#include "guests.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

// state

typedef struct {
  GtkWidget *component;
  PersonArray *arr;
  char *room_id;
} WidgetState;

static void free_widget_state(WidgetState *s) {
  free_person_array(s->arr);
  g_free(s->room_id);
  g_free(s);
}

// logic

static PersonArray *list_room_guests(const char *room_id) {
  char *query;
  asprintf(&query, "SELECT * FROM read_room_guests('%s')", room_id);

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

  PersonArray *arr = g_malloc(sizeof(PersonArray));
  Person *guests = g_malloc(len * sizeof(Person));

  for (int i = 0; i < len; i++) {
    guests[i] = (Person){
        g_strdup(PQgetvalue(res, i, 0)), g_strdup(PQgetvalue(res, i, 1)),
        g_strdup(PQgetvalue(res, i, 2)), g_strdup(PQgetvalue(res, i, 3))};
  }
  PQclear(res);
  *arr = (PersonArray){guests, len};

  return arr;
}

static void handle_item_click(GtkListBox *_, GtkListBoxRow *row,
                              gpointer state) {
  WidgetState *s = (WidgetState *)state;
  const char *guest_id = s->arr->guests[gtk_list_box_row_get_index(row)].id;
  g_print("%s", guest_id);
}

static void handle_close(GtkWidget *_, gpointer state) {
  WidgetState *s = (WidgetState *)state;
  gtk_stack_set_visible_child_name(APP_STACK, "home");
  gtk_stack_remove(APP_STACK, s->component);
}

static void handle_destroy(GtkWidget *_, gpointer data) {
  free_widget_state(data);
}

// UI

GtkWidget *room_update_component(const char *room_id, const char *occupancy) {
  // main container
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);

  // state
  PersonArray *current_guests = list_room_guests(room_id);

  WidgetState *state = g_malloc(sizeof(WidgetState));
  *state = (WidgetState){box, current_guests, g_strdup(room_id)};

  // close button

  GtkWidget *close_button = gtk_button_new_with_label("Отмена");
  gtk_widget_set_halign(close_button, GTK_ALIGN_START);
  gtk_widget_set_margin_start(close_button, 10);
  gtk_box_append(GTK_BOX(box), close_button);
  g_signal_connect(close_button, "clicked", G_CALLBACK(handle_close), state);

  // room name
  char *text;
  asprintf(&text, "Номер: %s\tВместимость: %s", room_id, occupancy);
  GtkWidget *room_name = gtk_label_new(NULL);
  gtk_widget_add_css_class(room_name, "header");
  gtk_label_set_markup(GTK_LABEL(room_name), text);
  g_free(text);
  gtk_box_append(GTK_BOX(box), room_name);

  // guests

  GtkWidget *list = gtk_list_box_new();
  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_widget_set_margin_top(box, 10);
  gtk_widget_set_visible(frame, false);
  gtk_frame_set_child(GTK_FRAME(frame), list);
  gtk_widget_set_halign(frame, GTK_ALIGN_CENTER);
  gtk_list_box_set_selection_mode(GTK_LIST_BOX(list), GTK_SELECTION_NONE);
  gtk_box_append(GTK_BOX(box), frame);

  // add button

  GtkWidget *button = gtk_button_new_with_label("Добавить гостя");
  gtk_widget_set_halign(button, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(box), button);

  // handle signals

  g_signal_connect(list, "row-activated", G_CALLBACK(handle_item_click), state);

  g_signal_connect(box, "destroy", G_CALLBACK(handle_destroy), state);

  return box;
}
