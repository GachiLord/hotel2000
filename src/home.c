#include "checkIn.h"
#include "login.h"
#include <gtk/gtk.h>

GtkWidget *initHomePage() {
  GtkWidget *stack = gtk_stack_new();
  gtk_stack_set_transition_type(GTK_STACK(stack),
                                GTK_STACK_TRANSITION_TYPE_CROSSFADE);
  // create login page
  GtkWidget *login = initLoginPage();
  gtk_stack_add_named(GTK_STACK(stack), login, "login");

  // create app pages and put them in a stack
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  GtkWidget *switcher = gtk_stack_switcher_new();
  GtkWidget *main_stack = gtk_stack_new();
  // pages
  GtkWidget *checkIn = checkInPage();
  GtkWidget *checkOut = gtk_label_new("check out");
  gtk_stack_add_titled(GTK_STACK(main_stack), checkIn, "checkIn", "checkIn");
  gtk_stack_add_titled(GTK_STACK(main_stack), checkOut, "checkOut", "checkOut");
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
