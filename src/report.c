#include "report.h"
#include "common.h"
#include "gio/gio.h"
#include <gtk/gtk.h>

typedef struct {
  char *html;
  gsize len;
} Html;

extern const char report_html[];
extern unsigned int report_html_len;
extern const char chart_js[];
extern unsigned int chart_js_len;

static Html get_report_html(char *report) {
  char *html;

  int bytes_written = asprintf(&html, report_html, chart_js, report);

  return (Html){html, bytes_written};
}

void handle_save_report(GObject *source_object, GAsyncResult *res,
                        gpointer report) {

  if (source_object == NULL)
    return;

  GFile *file =
      gtk_file_dialog_save_finish(GTK_FILE_DIALOG(source_object), res, NULL);

  Html html = get_report_html(report);
  bool success = g_file_replace_contents(file, html.html, html.len, NULL, false,
                                         G_FILE_CREATE_NONE, NULL, NULL, NULL);

  if (success)
    show_toast("Файл сохранен");
  else
    show_toast("Не удалось сохранить файл");

  g_free(report);
  g_free(html.html);
}
