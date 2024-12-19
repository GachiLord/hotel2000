#include "common.h"
#include "database.h"
#include "glib-object.h"
#include "glib.h"
#include "guests.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

// state

typedef struct {
  // component
  GtkWidget *list;
  GtkWidget *frame;
  GtkWidget *parent;
  GtkWidget *component;
  // callback
  GuestClickHandler guest_click_handler;
  gpointer component_data;
  // data
  PersonArray *arr;
} WidgetState;

// logic

static PersonArray *find_guests_by_name(const char *name) {
  char *query;
  asprintf(&query, "SELECT * FROM find_guests('%s')", name);

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

static void handle_search(GtkWidget *widget, gpointer state) {
  WidgetState *s = (WidgetState *)state;
  const char *q = gtk_editable_get_text(GTK_EDITABLE(widget));

  if (g_utf8_strlen(q, -1) == 0) {
    return;
  }

  free_person_array(s->arr);
  s->arr = find_guests_by_name(q);

  render_guests_to_list(GTK_LIST_BOX(s->list), GTK_FRAME(s->frame), s->arr,
                        true);
}

static void handle_item_click(GtkListBox *self, GtkListBoxRow *row,
                              gpointer state) {
  WidgetState *s = (WidgetState *)state;
  const Person p = s->arr->guests[gtk_list_box_row_get_index(row)];

  s->guest_click_handler(row, p, s->component_data);
}

static void handle_close(GtkWidget *_, gpointer state) {
  WidgetState *s = (WidgetState *)state;
  remove_widget_from_main_stack(s->component, s->parent);
}

static void handle_destroy(GtkWidget *_, gpointer state) {
  WidgetState *s = (WidgetState *)state;
  free_person_array(s->arr);
  g_free(state);
}

// UI

GtkWidget *search_guests_component(GuestClickHandler on_item_click,
                                   const gpointer data, bool handle_cancel,
                                   GtkWidget *parent) {
  // main containers
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  GtkWidget *list = gtk_list_box_new();
  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_widget_set_margin_top(box, 10);
  gtk_widget_set_visible(frame, false);
  gtk_frame_set_child(GTK_FRAME(frame), list);
  gtk_widget_set_halign(frame, GTK_ALIGN_CENTER);
  gtk_list_box_set_selection_mode(GTK_LIST_BOX(list), GTK_SELECTION_NONE);
  gtk_box_append(GTK_BOX(box), frame);

  // state
  WidgetState *state = g_malloc(sizeof(WidgetState));
  *state = (WidgetState){
      list, frame, parent, box, on_item_click, data, NULL,
  };

  // controls

  GtkWidget *search = gtk_search_entry_new();
  gtk_box_prepend(GTK_BOX(box), search);
  gtk_widget_set_halign(search, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(search, 300, 40);

  // handle search
  g_signal_connect(search, "activate", G_CALLBACK(handle_search), state);

  // handle click

  if (on_item_click != NULL) {
    g_signal_connect(list, "row-activated", G_CALLBACK(handle_item_click),
                     state);
  }

  if (handle_cancel) {
    GtkWidget *cancel_button = gtk_button_new_with_label("Отмена");
    gtk_widget_set_halign(cancel_button, GTK_ALIGN_START);
    gtk_widget_set_margin_start(cancel_button, 10);
    g_signal_connect(cancel_button, "clicked", G_CALLBACK(handle_close), state);
    gtk_box_prepend(GTK_BOX(box), cancel_button);
  }

  // handle widget destroy
  g_signal_connect(box, "destroy", G_CALLBACK(handle_destroy), state);

  return box;
}
