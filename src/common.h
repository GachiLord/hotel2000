#include <libpq-fe.h>

void show_toast(const char *text);

int handle_db_error(PGresult *res, const char *const err_msg);
