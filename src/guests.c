#include "guests.h"
#include "glib.h"

// logic

void free_person_array(PersonArray *arr) {
  if (arr == NULL)
    return;

  for (gsize i = 0; i < arr->len; i++) {
    g_free(arr->guests[i].id);
    g_free(arr->guests[i].name);
    g_free(arr->guests[i].passport);
    g_free(arr->guests[i].phone);
  }
  g_free(arr->guests);
  g_free(arr);
}

PersonArray *push_person_array(PersonArray *self, const Person *item) {
  self->guests = g_realloc(self->guests, (self->len + 1) * sizeof(Person));
  self->guests[self->len] = *item;
  self->len++;
  return self;
}

PersonArray *extend_person_array(PersonArray *self, PersonArray *child) {
  // copy child elements to self
  self->guests =
      g_realloc(self->guests, (self->len + child->len) * sizeof(Person));
  memcpy(self->guests + self->len, child->guests, child->len * sizeof(Person));
  self->len += child->len;
  // free child struct but not its elements, because they are going to be used
  // by self struct
  g_free(child->guests);
  g_free(child);
  // return new-ish struct
  return self;
}

// UI

void render_guests_to_list(GtkListBox *list, GtkFrame *frame,
                           const PersonArray *guests,
                           bool remove_children_on_update) {
  if (remove_children_on_update)
    gtk_list_box_remove_all(GTK_LIST_BOX(list));

  if (guests == NULL) {
    gtk_widget_set_visible(GTK_WIDGET(frame), false);
    return;
  } else {
    gtk_widget_set_visible(GTK_WIDGET(frame), true);
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
    gtk_list_box_append(GTK_LIST_BOX(list), item);
  }
}
