#include "database.h"
#include <glib.h>
#include <gtk/gtk.h>
#include <libpq-fe.h>

// structs

typedef struct {
  char *room_id;
  char *occupancy;
} Room;

typedef struct {
  Room *rooms;
  gsize len;
} RoomArray;

// logic

void free_room_array(RoomArray *r);
RoomArray *extend_room_array(RoomArray *self, RoomArray *child);

// UI

void render_rooms_to_list(GtkListBox *list, GtkFrame *frame,
                          const RoomArray *rooms,
                          bool remove_children_on_update);
// widgets

GtkWidget *free_rooms_page();
GtkWidget *read_rooms_page();
GtkWidget *room_update_component(const char *room_id, const char *occupancy);
