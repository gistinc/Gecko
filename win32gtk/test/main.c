#include <gtk/gtk.h>


// Win32 header files
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>

#include "mozarea.h"

static GtkWidget *loading;
static GtkWidget *title;


static void
on_entry_activate (GtkEntry *entry, MozArea *area)
{
  moz_area_display_url (area, gtk_entry_get_text (entry));
}

static void
on_refresh_clicked (GtkButton *refresh, MozArea *area)
{
  GtkWidget *entry = (GtkWidget *)g_object_get_data (G_OBJECT (area), "url-entry");

  if (entry)
    {
      moz_area_display_url (area, gtk_entry_get_text (GTK_ENTRY (entry)));
    }
}

static void
on_html_entry_activate (GtkEntry *entry, MozArea *area)
{
  const gchar *filename = gtk_entry_get_text (entry);
  gchar *contents = NULL;
  GError *error = NULL;
  gsize   len;
  
  // Display data of the html file denoted by the entry
  if (g_file_get_contents (filename, &contents, &len, &error))
    {
      moz_area_display_data (area, "file://", "text/html", contents, len);
    }
  else
    {
      g_warning ("Couldnt get file contents of %s (%s)", filename, error->message);
      g_error_free (error);
    }
}


static void
reparent_mozilla_window (GtkWidget *button, GtkWidget *mozilla)
{
  GtkWidget *parent = gtk_widget_get_parent (mozilla);

  g_object_ref (G_OBJECT (mozilla));

  gtk_container_remove (GTK_CONTAINER(parent), mozilla);

  g_print ("Unparented widget realized ? %s\n", GTK_WIDGET_REALIZED (mozilla) ? "yes" : "no");

  gtk_container_add (GTK_CONTAINER(parent), mozilla);

  g_object_ref (G_OBJECT (mozilla));

  g_print ("Reparented widget realized ? %s\n", GTK_WIDGET_REALIZED (mozilla) ? "yes" : "no");

}


static void 
mozilla_title_notify (GObject *mozilla,
		      GParamSpec *pspec,
		      gpointer    user_data)
{
  gchar *title_text = moz_area_get_title (MOZ_AREA (mozilla));
  gtk_label_set_text (GTK_LABEL (title), title_text);
  g_free (title_text);
}

static void
mozilla_load_done (MozArea *mozilla,
		   gpointer unused)
{
  gtk_label_set_text (GTK_LABEL (loading), "loaded");
}

static gboolean
mozilla_url_requested (MozArea *mozilla,
		       const gchar *url,
		       gpointer unused)
{
  gchar *text = g_strdup_printf ("loading %s", url);
  gtk_label_set_text (GTK_LABEL (loading), text);
  g_free (text);

  return FALSE;
}

static GtkWidget *
build_window (char *first_url)
{
    GtkWidget *window;
    GtkWidget *mozilla;
    GtkWidget *vbox, *hbox, *notebook;
    GtkWidget *entry, *button, *button2;

    GtkWidget *placeholder_vbox;
    GtkWidget *placeholder_label;
    GtkWidget *placeholder_entry;
    GtkWidget *placeholder_label2;

    window   = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    mozilla  = moz_area_new ();
    vbox     = gtk_vbox_new (FALSE, 0);
    hbox     = gtk_hbox_new (FALSE, 0);
    notebook = gtk_notebook_new ();
    loading  = gtk_label_new ("loading...");
    title    = gtk_label_new ("<page title>");

    placeholder_label  = gtk_label_new ("Enter a local html file to render as data:");
    placeholder_label2 = gtk_label_new ("");
    placeholder_vbox   = gtk_vbox_new (FALSE, 0);
    placeholder_entry  = gtk_entry_new ();
    button2            = gtk_button_new_with_label ("Unparent/Reparent mozilla");

    gtk_container_add (GTK_CONTAINER (window), vbox);
    
    gtk_box_pack_start (GTK_BOX (vbox), loading,
			FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), title,
			FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox,
			FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), notebook,
			TRUE, TRUE, 0);

    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), mozilla, NULL);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), placeholder_vbox, NULL);
    gtk_box_pack_start (GTK_BOX (placeholder_vbox), placeholder_label,
			TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (placeholder_vbox), placeholder_entry,
			FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (placeholder_vbox), placeholder_label2,
			TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (placeholder_vbox), button2,
			FALSE, TRUE, 0);
    
    g_signal_connect (G_OBJECT (button2), "clicked",
		      G_CALLBACK (reparent_mozilla_window), mozilla);
    g_signal_connect (G_OBJECT (placeholder_entry), "activate",
		      G_CALLBACK (on_html_entry_activate), mozilla);



    entry    = gtk_entry_new ();
    button   = gtk_button_new_from_stock ("gtk-refresh");

    gtk_box_pack_start (GTK_BOX (hbox), entry,
			TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), button,
			FALSE, TRUE, 0);

    g_signal_connect (G_OBJECT (button), "clicked",
		      G_CALLBACK (on_refresh_clicked), mozilla);
    g_signal_connect (G_OBJECT (entry), "activate",
		      G_CALLBACK (on_entry_activate), mozilla);


    g_object_set_data (G_OBJECT (mozilla), "url-entry", entry);

    if (first_url) {
      moz_area_display_url (MOZ_AREA (mozilla), first_url);
    } else {
      moz_area_display_url (MOZ_AREA (mozilla), "google.com");
    }

    g_signal_connect (G_OBJECT (mozilla), "notify::page-title",
		      G_CALLBACK (mozilla_title_notify), NULL);

    g_signal_connect (G_OBJECT (mozilla), "document-loaded",
		      G_CALLBACK (mozilla_load_done), NULL);

    g_signal_connect (G_OBJECT (mozilla), "url-requested",
		      G_CALLBACK (mozilla_url_requested), NULL);


    gtk_window_set_default_size (GTK_WINDOW (window), 600, 400);

    return window;
}


int 
main (int argc, char *argv[])
{
    GtkWidget *window;
    char *first_url = NULL;

    gtk_init (&argc, &argv);

    if (argc > 1)
      first_url = argv[1];

    window = build_window (first_url);

    gtk_widget_show_all (window);

    gtk_main ();


    return 0;
}


