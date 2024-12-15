#include <gtk/gtk.h>

#pragma once

void handle_save_report(GObject *source_object, GAsyncResult *res,
                        gpointer report);

GtkWidget *report_page();
