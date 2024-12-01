#include "common.h"
#include "rooms.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

// logic

static RoomArray *get_rooms_like(int page) {
  char *query;
  asprintf(&query, "SELECT * FROM read_rooms_by_page(%d)", page);
  // excute
  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);
  // check for error
  if (handle_db_error(res, "Не удалось выполнить запрос")) {
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
  int page;
} WidgetState;

static void handle_load(GtkWidget *widget, gpointer state) {

  WidgetState *s = (WidgetState *)state;
  RoomArray *arr = get_rooms_like((s->page));
  render_rooms_to_list(GTK_LIST_BOX(s->list), GTK_FRAME(s->frame), arr, false);
  free_room_array(arr);
  s->page++;
}

static void handle_destroy(GtkWidget *_, gpointer data) { g_free(data); }

// UI

GtkWidget *read_rooms_page() {
  // main containers
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  GtkWidget *scroll = gtk_scrolled_window_new();
  GtkWidget *list = gtk_list_box_new();
  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), list);
  gtk_widget_set_size_request(scroll, 600, 900);
  gtk_widget_set_margin_top(box, 50);
  gtk_widget_set_visible(frame, false);
  gtk_frame_set_child(GTK_FRAME(frame), scroll);
  gtk_widget_set_halign(frame, GTK_ALIGN_CENTER);
  gtk_list_box_set_selection_mode(GTK_LIST_BOX(list), GTK_SELECTION_NONE);
  gtk_box_append(GTK_BOX(box), frame);

  // state
  WidgetState *state = g_malloc(sizeof(WidgetState));
  *state = (WidgetState){list, frame, 1};

  // controls

  GtkWidget *button = gtk_button_new_with_label("Загрузить номера");
  gtk_box_append(GTK_BOX(box), button);
  gtk_widget_set_halign(button, GTK_ALIGN_CENTER);
  // gtk_widget_set_size_request(button, 300, 40);

  // handle search
  g_signal_connect(button, "clicked", G_CALLBACK(handle_load), state);

  // handle widget destroy
  g_signal_connect(box, "destroy", G_CALLBACK(handle_destroy), state);

  return box;
}
