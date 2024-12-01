#include "common.h"
#include "database.h"
#include "glib-object.h"
#include "guests.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

// state

typedef struct {
  GtkWidget *list;
  GtkWidget *frame;
} WidgetState;

// logic

static PersonArray *find_guests_by_name(const char *name) {
  char *query;
  asprintf(&query, "SELECT * FROM find_guests('%s')", name);

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

static void handle_search(GtkWidget *widget, gpointer data) {
  const char *q = gtk_editable_get_text(GTK_EDITABLE(widget));

  if (g_utf8_strlen(q, -1) == 0) {
    return;
  }

  WidgetState *s = (WidgetState *)data;
  PersonArray *guests = find_guests_by_name(q);

  gtk_list_box_remove_all(GTK_LIST_BOX(s->list));

  if (guests == NULL) {
    gtk_widget_set_visible(s->frame, false);
    return;
  } else {
    gtk_widget_set_visible(s->frame, true);
  }

  for (gsize i = 0; i < guests->len; i++) {
    GtkWidget *item = gtk_center_box_new();

    char *t1;
    asprintf(&t1, "ФИО: %s", guests->guests[i].name);
    GtkWidget *l1 = gtk_label_new(t1);
    g_free(t1);
    gtk_widget_set_margin_start(l1, 20);
    gtk_center_box_set_start_widget(GTK_CENTER_BOX(item), l1);

    char *t2;
    asprintf(&t2, "Паспорт: %s", guests->guests[i].passport);
    GtkWidget *l2 = gtk_label_new(t2);
    g_free(t2);
    gtk_center_box_set_center_widget(GTK_CENTER_BOX(item), l2);

    char *t3;
    asprintf(&t3, "Телефон: %s", guests->guests[i].phone);
    GtkWidget *l3 = gtk_label_new(t3);
    g_free(t3);
    gtk_widget_set_margin_end(l3, 20);
    gtk_center_box_set_end_widget(GTK_CENTER_BOX(item), l3);

    gtk_widget_set_size_request(item, 800, 50);
    gtk_widget_set_halign(item, GTK_ALIGN_CENTER);
    gtk_list_box_append(GTK_LIST_BOX(s->list), item);
  }
  free_person_array(guests);
}

static void handle_destroy(GtkWidget *_, gpointer data) { g_free(data); }

// UI

GtkWidget *search_guests_component(GCallback on_item_click, gpointer data) {
  // main containers
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  GtkWidget *list = gtk_list_box_new();
  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_widget_set_margin_top(box, 50);
  gtk_widget_set_visible(frame, false);
  gtk_frame_set_child(GTK_FRAME(frame), list);
  gtk_widget_set_halign(frame, GTK_ALIGN_CENTER);
  gtk_list_box_set_selection_mode(GTK_LIST_BOX(list), GTK_SELECTION_NONE);
  gtk_box_append(GTK_BOX(box), frame);

  // state
  WidgetState *state = g_malloc(sizeof(WidgetState));
  *state = (WidgetState){list, frame};

  // controls

  GtkWidget *search = gtk_search_entry_new();
  gtk_box_prepend(GTK_BOX(box), search);
  gtk_widget_set_halign(search, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(search, 300, 40);

  // handle search
  g_signal_connect(search, "activate", G_CALLBACK(handle_search), state);

  // handle click

  if (on_item_click != NULL) {
    g_signal_connect(list, "row-activated", on_item_click, data);
  }

  // handle widget destroy
  g_signal_connect(box, "destroy", G_CALLBACK(handle_destroy), state);

  return box;
}
