#include "common.h"
#include "guests.h"
#include "login.h"
#include "rooms.h"
#include <gtk/gtk.h>

GtkWidget *init_home_page() {
  GtkWidget *stack = gtk_stack_new();
  gtk_stack_set_transition_type(GTK_STACK(stack),
                                GTK_STACK_TRANSITION_TYPE_CROSSFADE);
  // create login page
  GtkWidget *login = init_login_page();
  gtk_stack_add_named(GTK_STACK(stack), login, "login");

  // create app pages and put them in a stack
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  GtkWidget *switcher = gtk_stack_switcher_new();
  GtkWidget *main_stack = gtk_stack_new();
  // pages
  GtkWidget *free_rooms = free_rooms_page();
  GtkWidget *read_rooms = read_rooms_page();
  GtkWidget *guest_create = create_guests_page();
  GtkWidget *guest_search = search_guests_page();
  gtk_stack_add_titled(GTK_STACK(main_stack), free_rooms, "free_rooms",
                       "Поиск номеров");
  gtk_stack_add_titled(GTK_STACK(main_stack), read_rooms, "read_rooms",
                       "Просмотр номеров");
  gtk_stack_add_titled(GTK_STACK(main_stack), guest_create, "create_guests",
                       "Регистрация гостей");
  gtk_stack_add_titled(GTK_STACK(main_stack), guest_search, "search_guests",
                       "Поиск гостей");
  // bind switcher to main_stack and change the animation
  gtk_stack_switcher_set_stack(GTK_STACK_SWITCHER(switcher),
                               GTK_STACK(main_stack));
  gtk_stack_set_transition_type(GTK_STACK(main_stack),
                                GTK_STACK_TRANSITION_TYPE_CROSSFADE);

  // append menu and stack to the box
  gtk_box_append(GTK_BOX(box), switcher);
  gtk_box_append(GTK_BOX(box), main_stack);

  // add main stack to APP_STACK
  gtk_stack_add_named(GTK_STACK(stack), box, "home");

  return stack;
}
