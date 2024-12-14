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

UserArray *remove_user_array(UserArray *self, gsize index) {

  g_free(self->arr[index].login);

  if (index != self->len - 1)
    memmove(self->arr + index, self->arr + index + 1,
            (self->len - index - 1) * sizeof(User));

  self->len--;
  self->arr = realloc(self->arr, self->len * sizeof(User));
  return self;
}
