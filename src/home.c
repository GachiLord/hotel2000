#include "login.h"
#include <gtk/gtk.h>

GtkWidget *initHomePage() {
  GtkWidget *stack = gtk_stack_new();
  gtk_stack_set_transition_type(GTK_STACK(stack),
                                GTK_STACK_TRANSITION_TYPE_CROSSFADE);

  GtkWidget *login = initLoginPage();
  gtk_stack_add_named(GTK_STACK(stack), login, "login");

  GtkWidget *hello = gtk_label_new("Love 2000");
  gtk_stack_add_named(GTK_STACK(stack), hello, "home");

  GtkWidget *hello1 = gtk_label_new("Love 3000");
  gtk_stack_add_child(GTK_STACK(stack), hello1);

  return stack;
}
