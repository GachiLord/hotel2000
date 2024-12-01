#include "guests.h"
#include "glib.h"

void free_person_array(PersonArray *arr) {
  if (arr == NULL)
    return;

  for (gsize i = 0; i < arr->len; i++) {
    g_free(arr->guests[i].id);
    g_free(arr->guests[i].name);
    g_free(arr->guests[i].passport);
    g_free(arr->guests[i].phone);
  }
  g_free(arr->guests);
  g_free(arr);
}
