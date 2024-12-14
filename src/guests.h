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

void free_person(Person *p);
void free_person_fields(Person *p);
void free_person_array(PersonArray *arr);
Person person_copy(Person p);
PersonArray *new_person_array(gsize len);
PersonArray *extend_person_array(PersonArray *self, PersonArray *child);
PersonArray *push_person_array(PersonArray *self, Person item);
PersonArray *remove_person_array(PersonArray *self, gsize index);

// UI

typedef void (*GuestClickHandler)(GtkListBoxRow *row, const Person guest,
                                  gpointer data);
typedef void (*GuestUpdateHandler)(const char *name, const char *passport,
                                   const char *phone, gpointer data);

GtkWidget *create_guests_page();
GtkWidget *search_guests_page();
GtkWidget *search_guests_component(GuestClickHandler, gpointer data,
                                   bool handle_cancel, GtkWidget *parent);
GtkWidget *guest_update_component(const char *guest_id, GtkWidget *parent,
                                  GuestUpdateHandler update_handler,
                                  gpointer data);

void render_guests_to_list(GtkListBox *list, GtkFrame *frame,
                           const PersonArray *guests,
                           bool remove_children_on_update);
