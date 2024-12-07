#include <gtk/gtk.h>

#pragma once

// logic

typedef struct {
  char *item_id;
  char *title;
  double price;
} Item;

typedef struct {
  Item *arr;
  gsize len;
} ItemArray;

void free_item_fields(Item *i);
void free_item(Item *i);
void free_item_array(ItemArray *i);

// UI

typedef void (*ItemClickHandler)(Item item, gpointer data);

GtkWidget *search_goods_component(ItemClickHandler on_item_click, gpointer data,
                                  bool handle_cancel, GtkWidget *parent);
GtkWidget *create_goods_page();
GtkWidget *search_goods_page();
