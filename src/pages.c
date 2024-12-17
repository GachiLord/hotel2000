#include "common.h"
#include "goods.h"
#include "guests.h"
#include "login.h"
#include "report.h"
#include "rooms.h"
#include "user.h"
#include <gtk/gtk.h>

// define app pages
GtkWidget *HOME_WIDGET;
GtkStack *MAIN_STACK;
// init

/// should only be called after init_pages()
void set_pages_according_to_permission_level() {

  gtk_stack_add_titled(GTK_STACK(MAIN_STACK), free_rooms_page(), "free_rooms",
                       "Поиск номеров");

  gtk_stack_add_titled(GTK_STACK(MAIN_STACK), read_rooms_page(), "read_rooms",
                       "Просмотр номеров");

  gtk_stack_add_titled(GTK_STACK(MAIN_STACK), search_guests_page(),
                       "search_guests", "Поиск гостей");

  if (DB_STATE->permission_level >= HOSTESS) {
    gtk_stack_add_titled(GTK_STACK(MAIN_STACK), create_guests_page(),
                         "create_guests", "Регистрация гостей");
  }

  gtk_stack_add_titled(GTK_STACK(MAIN_STACK), search_goods_page(),
                       "search_goods", "Поиск товаров");

  if (DB_STATE->permission_level >= MANAGER) {
    gtk_stack_add_titled(GTK_STACK(MAIN_STACK), create_goods_page(),
                         "create_goods", "Создание товаров");

    gtk_stack_add_titled(GTK_STACK(MAIN_STACK), create_user_page(),
                         "create_users", "Создание пользователей");
  }

  gtk_stack_add_titled(GTK_STACK(MAIN_STACK), read_users_page(), "read_users",
                       "Просмотр пользователей");

  gtk_stack_add_titled(GTK_STACK(MAIN_STACK), report_page(), "report",
                       "Создать отчет");
}

/// this function should be called on the app startup
GtkWidget *init_pages() {
  GtkWidget *stack = gtk_stack_new();
  gtk_stack_set_transition_type(GTK_STACK(stack),
                                GTK_STACK_TRANSITION_TYPE_CROSSFADE);
  // create login page
  GtkWidget *login = init_login_page();
  gtk_stack_add_named(GTK_STACK(stack), login, "login");

  // create app pages and put them in a stack
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  // set global home var before creating pages
  HOME_WIDGET = box;

  GtkWidget *switcher = gtk_stack_switcher_new();
  GtkWidget *main_stack = gtk_stack_new();

  // set global main_stack var to be able to manupulate pages
  MAIN_STACK = GTK_STACK(main_stack);

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
