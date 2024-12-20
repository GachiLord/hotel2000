#include <gtk/gtk.h>

#pragma once

// logic

// items

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

// orders

typedef struct {
  char *order_id;
  bool has_paid;
  double sold_for;
  int amount;
  char *title;
} Order;

typedef struct {
  Order *arr;
  gsize len;
} OrderArray;

void free_order_fields(Order *o);
void free_order(Order *o);
void free_order_array(OrderArray *o);
OrderArray *new_order_array(gsize len);
void push_order_array(OrderArray *self, Order order);
OrderArray *remove_order_array(OrderArray *self, gsize index);

// UI

typedef void (*ItemClickHandler)(GtkListBoxRow *row, Item item, gpointer data);
typedef void (*ItemUpdateHandler)(const char *title, double price,
                                  gpointer data);

GtkWidget *search_goods_component(ItemClickHandler on_item_click, gpointer data,
                                  bool handle_cancel, GtkWidget *parent);
GtkWidget *goods_update_component(ItemUpdateHandler, const char *item_id,
                                  GtkWidget *parent, gpointer data);

GtkWidget *create_goods_page();
GtkWidget *search_goods_page();
