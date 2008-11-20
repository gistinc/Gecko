#include "moz-web-view.h"

#include <gtk/gtkwindow.h>
#include "embed.h"

enum {
    TITLE_CHANGED,
    STATUS_CHANGED,
    LOCATION_CHANGED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

class ViewListener : public MozViewListener {
public:
    ViewListener(MozWebView *view) : mView(view) {}
    virtual ~ViewListener() {}

    virtual void SetTitle(const char *new_title) {
        g_signal_emit(G_OBJECT(mView), signals[TITLE_CHANGED], 0, new_title);
    }

    virtual void StatusChanged(const char *new_status, PRUint32 flags) {
        g_signal_emit(G_OBJECT(mView), signals[STATUS_CHANGED], 0, new_status);
    }

    virtual void LocationChanged(const char *new_uri) {
        g_signal_emit(G_OBJECT(mView), signals[LOCATION_CHANGED], 0, new_uri);
    }

private:
    MozWebView *mView;
};

struct _MozWebViewPriv {
    MozView *view;
    GtkWidget *offscreen;
    GtkWidget *offscreen_window;
    GtkWidget *mozWidget;
    ViewListener *listener;
};

G_DEFINE_TYPE(MozWebView, moz_web_view, GTK_TYPE_BIN)

static void
moz_web_view_map(GtkWidget *widget)
{
    g_return_if_fail(widget != NULL);
    g_return_if_fail(MOZ_IS_WEB_VIEW(widget));

    MozWebView *view = MOZ_WEB_VIEW(widget);

    GTK_WIDGET_SET_FLAGS(widget, GTK_MAPPED);

    view->priv->view->Show();

    if (gtk_bin_get_child(GTK_BIN(widget)) != view->priv->mozWidget) {
        // XXX: gtkmozembed does this reparenting during realization, but
        // for as-yet-undiagnosed reasons it's not working there.
        // *Ideally* we wouldn't need to do this reparenting at all,
        // but the moz backend always assumes that its parent is realized.
        gtk_widget_reparent(view->priv->mozWidget, widget);
    }

    gdk_window_show(widget->window);

}

static void
moz_web_view_unmap(GtkWidget *widget)
{
    g_return_if_fail(widget != NULL);
    g_return_if_fail(MOZ_IS_WEB_VIEW(widget));

    MozWebView *view = MOZ_WEB_VIEW(widget);

    GTK_WIDGET_UNSET_FLAGS(widget, GTK_MAPPED);

    view->priv->view->Hide();
    gdk_window_hide(widget->window);
}

static void
moz_web_view_realize(GtkWidget *widget)
{
    g_return_if_fail(widget != NULL);
    g_return_if_fail(MOZ_IS_WEB_VIEW(widget));

    MozWebView *view = MOZ_WEB_VIEW(widget);

    GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

    GdkWindowAttr attributes;
    gint attributes_mask;

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual(widget);
    attributes.colormap = gtk_widget_get_colormap(widget);
    attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

    widget->window = gdk_window_new(gtk_widget_get_parent_window(widget),
                                    &attributes, attributes_mask);
    gdk_window_set_user_data(widget->window, view);

    widget->style = gtk_style_attach(widget->style, widget->window);
    gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);
}

static void
moz_web_view_unrealize(GtkWidget *widget)
{
    g_return_if_fail(widget != NULL);
    g_return_if_fail(MOZ_IS_WEB_VIEW(widget));

    MozWebView *view = MOZ_WEB_VIEW(widget);

    if (GTK_WIDGET_MAPPED(widget))
        moz_web_view_unmap(widget);

    if (gtk_bin_get_child(GTK_BIN(widget)) == view->priv->mozWidget) {
        gtk_widget_reparent(view->priv->mozWidget,
                            GTK_WIDGET(view->priv->offscreen));
    }

    GTK_WIDGET_CLASS(moz_web_view_parent_class)->unrealize(widget);
}

static void
moz_web_view_size_allocate(GtkWidget *widget, GtkAllocation *alloc)
{
    g_return_if_fail(widget != NULL);
    g_return_if_fail(MOZ_IS_WEB_VIEW(widget));

    MozWebView *view = MOZ_WEB_VIEW(widget);

    widget->allocation = *alloc;

    if (GTK_WIDGET_REALIZED(widget)) {
        gdk_window_move_resize(widget->window,
                               alloc->x, alloc->y, alloc->width, alloc->height);
        view->priv->view->SetPositionAndSize(0, 0, alloc->width, alloc->height);
    }
}

static void
moz_web_view_destroy(GtkObject *object)
{
    g_return_if_fail(object != NULL);
    g_return_if_fail(MOZ_IS_WEB_VIEW(object));

    MozWebView *view = MOZ_WEB_VIEW(object);

    if (view->priv->view) {
        delete view->priv->view;
        view->priv->view = NULL;
    }

    if (view->priv->listener) {
        delete view->priv->listener;
        view->priv->listener = NULL;
    }

    if (view->priv->offscreen) {
        gtk_widget_destroy(view->priv->offscreen);
        view->priv->offscreen = NULL;
    }
}

static void
moz_web_view_finalize(GObject *object)
{
    g_return_if_fail(object != NULL);
    g_return_if_fail(MOZ_IS_WEB_VIEW(object));

    MozWebView *view = MOZ_WEB_VIEW(object);

    delete view->priv;
}

static void
moz_web_view_class_init(MozWebViewClass *klass)
{
    GtkContainerClass *container_class = GTK_CONTAINER_CLASS(klass);
    GtkWidgetClass    *widget_class    = GTK_WIDGET_CLASS(klass);
    GtkObjectClass    *object_class    = GTK_OBJECT_CLASS(klass);
    GObjectClass      *gobject_class   = G_OBJECT_CLASS(klass);

    widget_class->realize = moz_web_view_realize;
    widget_class->unrealize = moz_web_view_unrealize;
    widget_class->map = moz_web_view_map;
    widget_class->unmap = moz_web_view_unmap;
    widget_class->size_allocate = moz_web_view_size_allocate;

    object_class->destroy = moz_web_view_destroy;

    gobject_class->finalize = moz_web_view_finalize;

    signals[TITLE_CHANGED] =
        g_signal_new("title-changed",
                     G_TYPE_FROM_CLASS(klass),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(MozWebViewClass, title_changed),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__STRING,
                     G_TYPE_NONE, 1, G_TYPE_STRING);

    signals[STATUS_CHANGED] =
        g_signal_new("status-changed",
                     G_TYPE_FROM_CLASS(klass),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(MozWebViewClass, status_changed),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__STRING,
                     G_TYPE_NONE, 1, G_TYPE_STRING);

    signals[LOCATION_CHANGED] =
        g_signal_new("location-changed",
                     G_TYPE_FROM_CLASS(klass),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(MozWebViewClass, location_changed),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__STRING,
                     G_TYPE_NONE, 1, G_TYPE_STRING);


static void
moz_web_view_init(MozWebView *view)
{
    view->priv = new MozWebViewPriv();
    view->priv->view = new MozView();
    view->priv->listener = new ViewListener(view);
    view->priv->view->SetListener(view->priv->listener);

    // XXX: should do one offscreen, not one-per-widget.
    view->priv->offscreen = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_realize(view->priv->offscreen);

    view->priv->view->CreateBrowser(view->priv->offscreen, 0, 0, 500, 500);
    view->priv->mozWidget = gtk_bin_get_child(GTK_BIN(view->priv->offscreen));
}

// Public

GtkWidget *
moz_web_view_new()
{
    return GTK_WIDGET(g_object_new(MOZ_TYPE_WEB_VIEW, NULL));
}

void
moz_web_view_load_uri(MozWebView *view, const char *uri)
{
    view->priv->view->LoadURI(uri);
}
