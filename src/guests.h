#include "glib.h"
#include <gtk/gtk.h>

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

GtkWidget *create_guests_page();
GtkWidget *search_guests_page();
GtkWidget *search_guests_component(GCallback on_item_click, gpointer data);
void free_person_array(PersonArray *arr);
