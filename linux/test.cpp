#include "embed.h"
#include <gtk/gtk.h>

class GtkEmbedListener : public MozViewListener
{
public:
  GtkEmbedListener(GtkWidget *widget) {
    mWidget = GTK_WIDGET(g_object_ref(widget));
  }
  ~GtkEmbedListener() {
    g_object_ref(mWidget);
  }
  void SetTitle(const char *newTitle) {
    gtk_window_set_title(GTK_WINDOW(mWidget), newTitle);
  }

private:
  GtkWidget *mWidget;
};

int
main(int argc, char **argv)
{
  gtk_init(&argc, &argv);
  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_show (window);
  g_signal_connect(window, "delete_event", gtk_main_quit, NULL);

  MozView view;
  GtkEmbedListener myListener(window);
  view.SetListener(&myListener);

  GtkAllocation alloc = window->allocation;
  view.CreateBrowser(window, 0, 0, alloc.width, alloc.height);

  view.LoadURI("http://www.google.com");

  gtk_main();
}
