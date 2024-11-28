#include "rooms.h"
#include "common.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>
#include <stdio.h>

void free_room_array(RoomArray *r) {
  for (gsize i = 0; i < r->len; i++) {
    g_free(r->rooms[i].room_id);
    g_free(r->rooms[i].occupancy);
  }
  g_free(r->rooms);
  g_free(r);
}

void render_rooms_to_list(GtkListBox *list, GtkFrame *frame, RoomArray *rooms) {
  gtk_list_box_remove_all(GTK_LIST_BOX(list));

  if (rooms == NULL || rooms->len == 0) {
    gtk_widget_set_visible(GTK_WIDGET(frame), false);
    return;
  } else {
    gtk_widget_set_visible(GTK_WIDGET(frame), true);
  }

  for (gsize i = 0; i < rooms->len; i++) {
    // format strings for labels
    char *room;
    char *occupancy;
    asprintf(&room, "Номер: %s", rooms->rooms[i].room_id);
    asprintf(&occupancy, "Вместимость: %s", rooms->rooms[i].occupancy);

    GtkWidget *room_l = gtk_label_new(room);
    g_free(room);
    gtk_widget_set_margin_start(room_l, 20);

    GtkWidget *occupancy_l = gtk_label_new(occupancy);
    g_free(occupancy);
    gtk_widget_set_margin_end(occupancy_l, 20);

    // add item to the list
    GtkWidget *item = gtk_center_box_new();

    gtk_center_box_set_start_widget(GTK_CENTER_BOX(item), room_l);
    gtk_center_box_set_end_widget(GTK_CENTER_BOX(item), occupancy_l);

    gtk_widget_set_size_request(item, 500, 50);
    gtk_widget_set_halign(item, GTK_ALIGN_CENTER);
    gtk_list_box_append(list, item);
  }
  free_room_array(rooms);
}
