#include "goods.h"

// items

void free_item_fields(Item *i) {
  g_free(i->item_id);
  g_free(i->title);
}
void free_item(Item *i) {
  free_item_fields(i);
  g_free(i);
}
void free_item_array(ItemArray *items) {
  if (items == NULL)
    return;
  for (gsize i = 0; i < items->len; i++) {
    free_item_fields(&items->arr[i]);
  }
  g_free(items->arr);
  g_free(items);
}

// orders

void free_order_fields(Order *o) {
  g_free(o->title);
  g_free(o->order_id);
}

void free_order(Order *o) {
  free_order_fields(o);
  g_free(o);
}

void free_order_array(OrderArray *o) {
  if (o == NULL)
    return;

  for (gsize i = 0; i < o->len; i++) {
    free_order_fields(o->arr + i);
  }
  g_free(o->arr);
  g_free(o);
}

void push_order_array(OrderArray *self, Order order) {
  self->arr = g_realloc(self->arr, (self->len + 1) * sizeof(Order));
  self->arr[self->len] = order;
  self->len++;
}

OrderArray *remove_order_array(OrderArray *self, gsize index) {
  free_order_fields(self->arr + index);

  if (index != self->len - 1)
    memmove(self->arr + index, self->arr + index + 1,
            (self->len - index - 1) * sizeof(Order));

  self->len--;
  self->arr = realloc(self->arr, self->len * sizeof(Order));
  return self;
}
