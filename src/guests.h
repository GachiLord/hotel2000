#include "glib.h"
#include <gtk/gtk.h>

// logic

typedef struct {
  char *id;
  char *name;
  char *passport;
  char *phone;
} Person;

typedef struct {
  Person *guests;
  gsize len;
} PersonArray;

void free_person_array(PersonArray *arr);
PersonArray *extend_person_array(PersonArray *self, PersonArray *child);
PersonArray *push_person_array(PersonArray *self, const Person *item);

// UI

typedef void (*GuestClickHandler)(GtkListBox *_, GtkListBoxRow *row,
                                  PersonArray **arr);

GtkWidget *create_guests_page();
GtkWidget *search_guests_page();
GtkWidget *search_guests_component(GuestClickHandler, bool handle_cancel);

void render_guests_to_list(GtkListBox *list, GtkFrame *frame,
                           const PersonArray *guests,
                           bool remove_children_on_update);
