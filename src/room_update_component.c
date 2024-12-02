#include "common.h"
#include "glib.h"
#include "guests.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

// state

typedef struct {
  GtkWidget *component;
  GtkWidget *list;
  GtkWidget *frame;
  PersonArray *arr;
  char *room_id;
  bool is_delete_mode;
} WidgetState;

static void free_widget_state(WidgetState *s) {
  free_person_array(s->arr);
  g_free(s->room_id);
  g_free(s);
}

static WidgetState *state;

// logic

// fetch current guests

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

// manipulate current users

static int add_guest_to_room(const char *room_id, const char *guest_id) {
  char *query;
  asprintf(&query, "call check_in_guest(%s, %s)", guest_id, room_id);

  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);

  if (handle_db_error(res, "Не удалось добавить гостя") != 0) {
    return -1;
  }

  PQclear(res);
  return 0;
}

static int remove_guest_from_room(const char *room_id, const char *guest_id) {
  char *query;
  asprintf(&query, "call check_out_guest(%s, %s)", guest_id, room_id);

  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);

  if (handle_db_error(res, "Не удалось удалить гостя") != 0) {
    return -1;
  }

  PQclear(res);
  return 0;
}

static void handle_guest_click(GtkListBox *_, GtkListBoxRow *row,
                               gpointer state) {
  WidgetState *s = (WidgetState *)state;
  int index = gtk_list_box_row_get_index(row);
  const char *guest_id = s->arr->guests[index].id;
  if (s->is_delete_mode) {
    if (remove_guest_from_room(s->room_id, guest_id) == 0) {
      remove_person_array(s->arr, index);
      render_guests_to_list(GTK_LIST_BOX(s->list), GTK_FRAME(s->frame), s->arr,
                            true);
      show_toast("Гость удален");
    }
  } else {
    // TODO: implement
  }
}

// adding new users
static void handle_new_guest_choose(GtkListBox *_, GtkListBoxRow *row,
                                    PersonArray **arr) {
  const Person *p = &(*arr)->guests[gtk_list_box_row_get_index(row)];
  // check if guest is present in the array
  for (gsize i = 0; state->arr != NULL && i < state->arr->len; i++) {
    if (g_strcmp0(state->arr->guests[i].id, p->id) == 0) {
      show_toast("Гость уже добавлен в номер");
      return;
    }
  }
  // make deep copy of the person and save it on the heap
  Person *pc = g_malloc(sizeof(Person));
  *pc = (Person){g_strdup(p->id), g_strdup(p->name), g_strdup(p->phone),
                 g_strdup(p->passport)};
  if (add_guest_to_room(state->room_id, pc->id) != 0) {
    return;
  }
  // add to array
  if (state->arr == NULL) {
    // init array if necessary
    state->arr = g_malloc(sizeof(PersonArray));
    *state->arr = (PersonArray){pc, 1};
  } else {
    // or push to existing one
    push_person_array(state->arr, pc);
    g_free(pc);
  }
  // render
  render_guests_to_list(GTK_LIST_BOX(state->list), GTK_FRAME(state->frame),
                        state->arr, true);
  show_toast("Гость добавлен в номер");
}

static void handle_add_user(GtkWidget *_, gpointer data) {
  GtkWidget *search_guests =
      search_guests_component(handle_new_guest_choose, true);
  add_widget_to_main_stack(search_guests, NULL);
}

static void handle_remove_user_mode(GtkWidget *widget, gpointer state) {
  WidgetState *s = (WidgetState *)state;
  s->is_delete_mode = !s->is_delete_mode;
  if (s->is_delete_mode) {
    gtk_button_set_label(GTK_BUTTON(widget), "Отмена");
    show_toast("Переход в режим удаления");
  } else {
    gtk_button_set_label(GTK_BUTTON(widget), "Убрать гостя");
    show_toast("Переход в обычный режим");
  }
}

// handle widget's disposal
static void handle_close(GtkWidget *_, gpointer state) {
  WidgetState *s = (WidgetState *)state;
  remove_widget_from_main_stack(s->component);
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

  state = g_malloc(sizeof(WidgetState));

  // close button

  GtkWidget *close_button = gtk_button_new_with_label("Назад");
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

  // init state

  *state =
      (WidgetState){box, list, frame, current_guests, g_strdup(room_id), false};

  // render
  render_guests_to_list(GTK_LIST_BOX(list), GTK_FRAME(frame), current_guests,
                        true);

  // add buttons

  GtkWidget *buttons_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  GtkWidget *button_add = gtk_button_new_with_label("Добавить гостя");
  gtk_widget_set_halign(button_add, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(buttons_box), button_add);

  GtkWidget *button_remove = gtk_button_new_with_label("Убрать гостя");
  gtk_widget_set_halign(button_remove, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(buttons_box), button_remove);
  gtk_widget_set_halign(buttons_box, GTK_ALIGN_CENTER);

  gtk_box_append(GTK_BOX(box), buttons_box);

  // handle signals

  g_signal_connect(button_add, "clicked", G_CALLBACK(handle_add_user), state);

  g_signal_connect(button_remove, "clicked",
                   G_CALLBACK(handle_remove_user_mode), state);

  g_signal_connect(list, "row-activated", G_CALLBACK(handle_guest_click),
                   state);

  g_signal_connect(box, "destroy", G_CALLBACK(handle_destroy), state);

  return box;
}
