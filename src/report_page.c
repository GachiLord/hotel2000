#include "common.h"
#include "report.h"
#include <gtk/gtk.h>
#include <stdio.h>

// state

typedef struct {
  GtkCalendar *start_calendar;
  GtkCalendar *end_calendar;
  GtkFileDialog *dialog;
} WidgetState;

static WidgetState state;

// logic

static char *get_report(const char *start_d, const char *end_d) {
  // query example: SELECT * FROM create_report(TIMESTAMP '2024-12-01 00:00:00',
  // TIMESTAMP '2024-12-15 00:00:00');
  char *query;
  asprintf(&query,
           "SELECT * FROM create_report(TIMESTAMP '%s', TIMESTAMP '%s')",
           start_d, end_d);

  PGresult *res = PQexec(DB_STATE->conn, query);
  g_free(query);

  if (handle_db_error(res, "Не удалось создать отчет") != 0) {
    return NULL;
  }
  char *output = g_strdup(PQgetvalue(res, 0, 0));

  PQclear(res);
  return output;
}

static void handle_create(GtkWidget *_, gpointer __) {
  GDateTime *start_d = gtk_calendar_get_date(state.start_calendar);
  GDateTime *end_d = gtk_calendar_get_date(state.end_calendar);

  gchar *start_s = g_date_time_format_iso8601(start_d);
  gchar *end_s = g_date_time_format_iso8601(end_d);

  g_date_time_unref(start_d);
  g_date_time_unref(end_d);

  char *report_json = get_report(start_s, end_s);

  g_free(start_s);
  g_free(end_s);

  if (report_json != NULL) {
    gtk_file_dialog_save(state.dialog, APP_WINDOW, NULL, handle_save_report,
                         report_json);
  }
}

// UI

GtkWidget *report_page() {
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_margin_top(box, 50);
  gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

  GtkWidget *calendar_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
  gtk_widget_set_halign(calendar_box, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(box), calendar_box);

  GtkWidget *start_l = gtk_label_new("Начало отчета:");
  gtk_widget_add_css_class(start_l, "header");
  gtk_box_append(GTK_BOX(calendar_box), start_l);
  GtkWidget *start_calendar = gtk_calendar_new();
  gtk_box_append(GTK_BOX(calendar_box), start_calendar);

  GtkWidget *end_l = gtk_label_new("Конец отчета:");
  gtk_widget_add_css_class(end_l, "header");
  gtk_box_append(GTK_BOX(calendar_box), end_l);
  GtkWidget *end_calendar = gtk_calendar_new();
  gtk_box_append(GTK_BOX(calendar_box), end_calendar);

  GtkWidget *generate_button = gtk_button_new_with_label("Создать отчет");
  gtk_box_append(GTK_BOX(box), generate_button);
  gtk_widget_set_halign(generate_button, GTK_ALIGN_CENTER);
  g_signal_connect(generate_button, "clicked", G_CALLBACK(handle_create), NULL);

  GtkFileDialog *dialog = gtk_file_dialog_new();
  gtk_file_dialog_set_initial_name(dialog, "Отчет.html");

  state = (WidgetState){GTK_CALENDAR(start_calendar),
                        GTK_CALENDAR(end_calendar), GTK_FILE_DIALOG(dialog)};

  return box;
}
