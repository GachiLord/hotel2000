#include "common.h"
#include "database.h"
#include <glib.h>
#include <gtk/gtk.h>
#include <libpq-fe.h>
#include <stdio.h>

// logic

extern DbState *DB_STATE;

struct {
  char *room_id;
  char *occupancy;
} typedef Room;

struct {
  Room *rooms;
  gsize len;
} typedef RoomArray;

static void free_room_array(RoomArray *r) {
  for (int i = 0; i < r->len; i++) {
    g_free(r->rooms[i].room_id);
    g_free(r->rooms[i].occupancy);
  }
  g_free(r->rooms);
  g_free(r);
}

static RoomArray *get_free_rooms(int occupancy) {
  // build query
  char *query;
  asprintf(&query, "SELECT * from find_free_rooms(%d)", occupancy);
  // excute
  PGresult *res = PQexec(DB_STATE->conn, query);
  // check for error
  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    return NULL;
  }
  // create Room structs
  gsize r_len = PQntuples(res);
  Room *rooms = g_malloc(sizeof(Room) * r_len);
  for (int i = 0; i < r_len; i++) {
    rooms[i].room_id = g_strdup(PQgetvalue(res, i, 0));
    rooms[i].occupancy = g_strdup(PQgetvalue(res, i, 1));
  }
  PQclear(res);
  g_free(query);
  // return array of rooms
  RoomArray *arr = g_malloc(sizeof(RoomArray));
  *arr = (RoomArray){rooms, r_len};
  return arr;
}

// state

struct {
  GtkWidget *list;
  GtkWidget *frame;
  int occupancy;
} typedef WidgetState;

static void handle_search(GtkWidget *widget, gpointer state) {
  WidgetState *s = (WidgetState *)state;
  RoomArray *rooms = get_free_rooms(s->occupancy);
  gtk_list_box_remove_all(GTK_LIST_BOX(s->list));

  if (rooms->len == 0) {
    gtk_widget_set_visible(s->frame, false);
  } else {
    gtk_widget_set_visible(s->frame, true);
  }

  for (int i = 0; i < rooms->len; i++) {
    char *text;
    asprintf(&text, "Номер: %s\t\t\tВместимость: %s", rooms->rooms[i].room_id,
             rooms->rooms[i].occupancy);
    GtkWidget *item = gtk_label_new(text);
    g_free(text);
    gtk_widget_set_size_request(item, 300, 50);
    gtk_widget_set_halign(item, GTK_ALIGN_CENTER);
    gtk_list_box_append(GTK_LIST_BOX(s->list), item);
  }
  free_room_array(rooms);
}

static void handle_occpancy(GtkWidget *widget, gpointer state) {
  WidgetState *s = (WidgetState *)state;
  s->occupancy = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
}

// ui

GtkWidget *check_in_page() {
  // main containers
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  GtkWidget *list = gtk_list_box_new();
  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_widget_set_visible(frame, false);
  gtk_frame_set_child(GTK_FRAME(frame), list);
  gtk_widget_set_halign(frame, GTK_ALIGN_CENTER);
  gtk_list_box_set_selection_mode(GTK_LIST_BOX(list), GTK_SELECTION_NONE);
  gtk_widget_set_halign(list, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(box), frame);

  // state
  WidgetState *state = g_malloc(sizeof(WidgetState));
  *state = (WidgetState){list, frame, 2};

  // wrapper
  GtkWidget *wrap = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(wrap, GTK_ALIGN_CENTER);
  gtk_widget_set_margin_top(wrap, 20);
  gtk_box_prepend(GTK_BOX(box), wrap);
  // spin button
  GtkAdjustment *adjustment =
      gtk_adjustment_new(2.0, 1.0, 100.0, 1.0, 5.0, 0.0);
  GtkWidget *spin_button = gtk_spin_button_new(adjustment, 1.0, 0);
  gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spin_button), true);
  g_signal_connect(spin_button, "value-changed", G_CALLBACK(handle_occpancy),
                   state);
  gtk_box_prepend(GTK_BOX(wrap), gtk_label_new("мест(а)"));
  gtk_box_prepend(GTK_BOX(wrap), spin_button);
  gtk_box_prepend(GTK_BOX(wrap), gtk_label_new("Вместимость от"));
  // button
  GtkWidget *button = gtk_button_new_with_label("Поиск свободных номеров");
  gtk_widget_set_size_request(button, -1, 40);
  gtk_widget_set_halign(button, GTK_ALIGN_CENTER);
  g_signal_connect(button, "clicked", G_CALLBACK(handle_search), state);
  gtk_box_prepend(GTK_BOX(wrap), button);

  return box;
}
