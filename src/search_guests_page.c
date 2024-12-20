#include "common.h"
#include "guests.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>

// state

// logic

static void handle_update(const char *name, const char *passport,
                          const char *phone, bool deleted, gpointer data) {
  GtkListBoxRow *row = GTK_LIST_BOX_ROW(data);
  GtkCenterBox *box = GTK_CENTER_BOX(gtk_list_box_row_get_child(row));

  if (deleted) {
    gtk_list_box_row_set_child(row, gtk_label_new("Удалено"));
    gtk_widget_set_size_request(GTK_WIDGET(row), GUESTS_DEFAULT_WIDTH, 50);
    gtk_widget_set_sensitive(GTK_WIDGET(row), false);
    return;
  }

  GtkLabel *name_l = GTK_LABEL(gtk_center_box_get_start_widget(box));
  GtkLabel *passport_l = GTK_LABEL(gtk_center_box_get_center_widget(box));
  GtkLabel *phone_l = GTK_LABEL(gtk_center_box_get_end_widget(box));

  char *name_s;
  asprintf(&name_s, "ФИО: %s", name);
  gtk_label_set_text(name_l, name_s);
  g_free(name_s);

  char *passport_s;
  asprintf(&passport_s, "Паспорт: %s", passport);
  gtk_label_set_text(passport_l, passport_s);
  g_free(passport_s);

  char *phone_s;
  asprintf(&phone_s, "Телефон: %s", phone);
  gtk_label_set_text(phone_l, phone_s);
  g_free(phone_s);
}

static void handle_item_click(GtkListBoxRow *row, Person guest, gpointer __) {

  add_widget_to_main_stack(
      guest_update_component(guest.id, HOME_WIDGET, handle_update, row));
}

// UI

GtkWidget *search_guests_page() {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_margin_top(box, DEFAULT_MARGIN_TOP);
  gtk_box_append(GTK_BOX(box), search_guests_component(handle_item_click, NULL,
                                                       false, HOME_WIDGET));

  return box;
}
