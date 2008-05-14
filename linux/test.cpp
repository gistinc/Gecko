#include "moz-web-view.h"
#include <gtk/gtk.h>

void
title_cb(MozWebView *view, const char *title, gpointer user_data)
{
  gtk_label_set_text (GTK_LABEL(user_data), title);
}

void
activate_cb(GtkWidget *widget, gpointer user_data)
{
  const char *uri = gtk_entry_get_text (GTK_ENTRY (widget));

  int id = gtk_notebook_get_current_page (GTK_NOTEBOOK (user_data));
  GtkWidget *view = gtk_notebook_get_nth_page (GTK_NOTEBOOK (user_data), id);
  moz_web_view_load_uri(MOZ_WEB_VIEW(view), uri);
}

void
add_page (GtkNotebook *notebook)
{
  GtkWidget *view = moz_web_view_new();
  GtkWidget *tabLabel = gtk_label_new("");

  g_signal_connect(view, "title", G_CALLBACK(title_cb), tabLabel);
  moz_web_view_load_uri(MOZ_WEB_VIEW(view), "http://www.google.com");

  gtk_notebook_append_page(notebook, view, tabLabel);
}

int
main(int argc, char **argv)
{
  gtk_init(&argc, &argv);
  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(window, "delete_event", gtk_main_quit, NULL);

  GtkWidget *box = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(window), box);

  GtkWidget *notebook = gtk_notebook_new();
  add_page(GTK_NOTEBOOK(notebook));
  add_page(GTK_NOTEBOOK(notebook));

  GtkWidget *entry = gtk_entry_new();
  g_signal_connect(entry, "activate", G_CALLBACK(activate_cb), notebook);

  gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 5);
  gtk_box_pack_start(GTK_BOX(box), notebook, TRUE, TRUE, 0);

  GtkWidget *statusbar = gtk_statusbar_new();
  gtk_box_pack_start(GTK_BOX(box), statusbar, FALSE, FALSE, 0);

  gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);

  gtk_widget_show_all (window);

  gtk_main();
}
