#include "goods.h"

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
