#include "user.h"

void free_user_array(UserArray *users) {
  if (users == NULL)
    return;

  for (gsize i = 0; i < users->len; i++) {
    g_free(users->arr[i].login);
  }

  g_free(users->arr);
  g_free(users);
}
