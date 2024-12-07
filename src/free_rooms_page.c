#include "common.h"
#include "rooms.h"
#include <glib.h>
#include <gtk/gtk.h>
#include <libpq-fe.h>
#include <stdio.h>

// logic

static RoomArray *get_free_rooms(int occupancy) {
  // build query
  char *query;
  asprintf(&query, "SELECT * from find_free_rooms(%d)", occupancy);
  // excute
  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);
  // check for error
  if (handle_db_error(res, "Не удалось выполнить запрос") != 0) {
    return NULL;
  }
  // create Room structs
  gsize r_len = PQntuples(res);

  if (r_len == 0) {
    PQclear(res);
    return NULL;
  }

  Room *rooms = g_malloc(sizeof(Room) * r_len);
  for (gsize i = 0; i < r_len; i++) {
    rooms[i].room_id = g_strdup(PQgetvalue(res, i, 0));
    rooms[i].occupancy = g_strdup(PQgetvalue(res, i, 1));
  }
  PQclear(res);
  // return array of rooms
  RoomArray *arr = g_malloc(sizeof(RoomArray));
  *arr = (RoomArray){rooms, r_len};
  return arr;
}

// state

typedef struct {
  GtkWidget *list;
  GtkWidget *frame;

  RoomArray *rooms;
  int occupancy;
} WidgetState;

static WidgetState state;

static void handle_occpancy(GtkWidget *widget, gpointer _) {
  state.occupancy = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
}

static void handle_search(GtkWidget *_, gpointer __) {
  free_room_array(state.rooms);
  state.rooms = get_free_rooms(state.occupancy);
  render_rooms_to_list(GTK_LIST_BOX(state.list), GTK_FRAME(state.frame),
                       state.rooms, true);
}

static void handle_item_click(GtkWidget *_, GtkListBoxRow *row, gpointer __) {
  // get room_id
  const char *room_id =
      state.rooms->rooms[gtk_list_box_row_get_index(row)].room_id;
  const char *occupancy =
      state.rooms->rooms[gtk_list_box_row_get_index(row)].occupancy;
  // invoke room updater component
  GtkWidget *room_update =
      room_update_component(room_id, occupancy, HOME_WIDGET);
  add_widget_to_main_stack(room_update);
}

static void handle_destroy(GtkWidget *_, gpointer data) {
  free_room_array(state.rooms);
}

// ui

GtkWidget *free_rooms_page() {
  // main containers
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  GtkWidget *list = gtk_list_box_new();
  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_widget_set_visible(frame, false);
  gtk_frame_set_child(GTK_FRAME(frame), list);
  gtk_widget_set_halign(frame, GTK_ALIGN_CENTER);
  gtk_list_box_set_selection_mode(GTK_LIST_BOX(list), GTK_SELECTION_NONE);
  gtk_box_append(GTK_BOX(box), frame);

  // state
  state = (WidgetState){list, frame, NULL, 2};

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
                   NULL);
  gtk_box_prepend(GTK_BOX(wrap), gtk_label_new("мест(а)"));
  gtk_box_prepend(GTK_BOX(wrap), spin_button);
  gtk_box_prepend(GTK_BOX(wrap), gtk_label_new("Вместимость от"));
  // button
  GtkWidget *button = gtk_button_new_with_label("Поиск свободных номеров");
  gtk_widget_set_size_request(button, -1, 40);
  gtk_widget_set_halign(button, GTK_ALIGN_CENTER);
  g_signal_connect(button, "clicked", G_CALLBACK(handle_search), NULL);
  gtk_box_prepend(GTK_BOX(wrap), button);

  // handle item click

  g_signal_connect(list, "row-activated", G_CALLBACK(handle_item_click), NULL);

  // handle destroy
  g_signal_connect(box, "destroy", G_CALLBACK(handle_destroy), NULL);

  return box;
}
