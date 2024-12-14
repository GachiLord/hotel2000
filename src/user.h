#include <gtk/gtk.h>

#pragma once

// logic

typedef enum { VIEWER, HOSTESS, MANAGER } PersmissionLevel;

typedef struct {
  char *login;
} User;

typedef struct {
  User *arr;
  gsize len;
} UserArray;

void free_user_array(UserArray *users);
UserArray *remove_user_array(UserArray *users, gsize index);

// UI

GtkWidget *create_user_page();
GtkWidget *read_users_page();
