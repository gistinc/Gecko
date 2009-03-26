#include "moz-web-view-common.h"

class ViewListener;
struct _MozWebViewPriv {
    MozView *view;
    GtkWidget *offscreen;
    GtkWidget *mozWidget;
    ViewListener *listener;
  
    /* Record some properties here */
    gchar *requested_uri;
    gchar *title;
    gchar *status;
    gchar *location;
};

static guint signals[LAST_SIGNAL] = { 0 };

static void
update_property (MozWebView  *view, gint prop_id, const gchar *new_value)
{
    MozWebViewPriv *priv = view->priv;
    const gchar *name = NULL; 
    gchar **ptr;

    switch (prop_id) {
        case PROP_REQUESTED_URI: ptr = &(priv->requested_uri); name = "requested-uri";  break;
        case PROP_TITLE:         ptr = &(priv->title);         name = "title";  break;
        case PROP_STATUS:        ptr = &(priv->status);        name = "status";  break;
        case PROP_LOCATION:      ptr = &(priv->location);      name = "location";  break;
        default:
            g_assert_not_reached ();
        break;
    }

    g_free (*ptr);
    *ptr = g_strdup (new_value);
    g_object_notify (G_OBJECT (view), name);
}

class ViewListener : public MozViewListener {
public:
    ViewListener(MozWebView *view) : mView(view) {}
    virtual ~ViewListener() {}

    virtual void SetTitle(const char *new_title) {
        update_property (mView, PROP_TITLE, new_title);
        g_signal_emit (mView, signals[TITLE_CHANGED],
                       NULL, new_title);
    }

    virtual void StatusChanged(const char *new_status, PRUint32 flags) {
        update_property (mView, PROP_STATUS, new_status);
        g_signal_emit (mView, signals[STATUS_CHANGED],
                       NULL, new_status);
    }

    virtual void LocationChanged(const char *new_uri) {
        update_property (mView, PROP_LOCATION, new_uri);
        g_signal_emit (mView, signals[LOCATION_CHANGED],
                       NULL, new_uri);
    }

    virtual PRBool OpenURI(const char* new_uri) {
        gboolean   abort_load = FALSE;
        update_property (mView, PROP_REQUESTED_URI, new_uri);
        g_signal_emit (mView, signals[URI_REQUESTED],
                       NULL, new_uri, &abort_load);

        return abort_load;
    }

    virtual void DocumentLoaded() {
        g_signal_emit (mView, signals[DOCUMENT_LOADED], NULL);
    }

    private:
        MozWebView *mView;
};

static void
moz_web_view_viewable_init (MozViewableIface *iface) {}

G_DEFINE_TYPE_WITH_CODE (MozWebView, moz_web_view, GTK_TYPE_BIN,
                         G_IMPLEMENT_INTERFACE (MOZ_TYPE_VIEWABLE,
                                                moz_web_view_viewable_init))

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

    gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);
    widget->style = gtk_style_attach (widget->style, widget->window);
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
        gdk_window_move_resize(widget->window, alloc->x, alloc->y,
                               alloc->width, alloc->height);
        view->priv->view->SetPositionAndSize(0, 0, alloc->width, alloc->height);
    }
}

static void
moz_web_view_destroy(GtkObject *object)
{
    g_return_if_fail(object != NULL);
    g_return_if_fail(MOZ_IS_WEB_VIEW(object));

    MozWebView *view = MOZ_WEB_VIEW(object);

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

    g_free (view->priv->requested_uri);
    g_free (view->priv->title);
    g_free (view->priv->status);
    g_free (view->priv->location);
	
    delete view->priv->view;
    delete view->priv->listener;

    (G_OBJECT_CLASS(moz_web_view_parent_class)->finalize)(object);
}

static void 
moz_web_view_get_property (GObject     *object,
                           guint        prop_id,
                           GValue      *value,
                           GParamSpec  *pspec)
{ 
    MozWebViewPriv *priv = MOZ_WEB_VIEW (object)->priv;

    switch (prop_id) {
        case PROP_REQUESTED_URI:
            g_value_set_string (value, priv->requested_uri);
        break;
        case PROP_TITLE:
            g_value_set_string (value, priv->title);
        break;
        case PROP_STATUS:
            g_value_set_string (value, priv->status);
        break;
        case PROP_LOCATION:
            g_value_set_string (value, priv->location);
        break;

       default:
           G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
       break;
	}
}

static void
moz_web_view_class_init(MozWebViewClass *klass)
{
    GtkContainerClass *container_class = GTK_CONTAINER_CLASS(klass);
    GtkWidgetClass    *widget_class    = GTK_WIDGET_CLASS(klass);
    GtkObjectClass    *object_class    = GTK_OBJECT_CLASS(klass);
    GObjectClass      *gobject_class   = G_OBJECT_CLASS(klass);

    gobject_class->get_property  = moz_web_view_get_property;
  
    widget_class->realize = moz_web_view_realize;
    widget_class->unrealize = moz_web_view_unrealize;
    widget_class->map = moz_web_view_map;
    widget_class->unmap = moz_web_view_unmap;
    widget_class->size_allocate = moz_web_view_size_allocate;

    object_class->destroy = moz_web_view_destroy;

    gobject_class->finalize = moz_web_view_finalize;

    signals[TITLE_CHANGED] = g_signal_lookup ("title-changed", MOZ_TYPE_WEB_VIEW);
    signals[STATUS_CHANGED] = g_signal_lookup ("status-changed", MOZ_TYPE_WEB_VIEW);
    signals[LOCATION_CHANGED] = g_signal_lookup ("location-changed", MOZ_TYPE_WEB_VIEW);
    signals[URI_REQUESTED] = g_signal_lookup ("uri-requested", MOZ_TYPE_WEB_VIEW);
    signals[DOCUMENT_LOADED] = g_signal_lookup ("document-loaded", MOZ_TYPE_WEB_VIEW);

    /* Implement MozViewable properties */   
    g_object_class_override_property (gobject_class, PROP_REQUESTED_URI, "requested-uri");
    g_object_class_override_property (gobject_class, PROP_TITLE, "title");
    g_object_class_override_property (gobject_class, PROP_STATUS, "status");
    g_object_class_override_property (gobject_class, PROP_LOCATION, "location");
}

static void
moz_web_view_init(MozWebView *view)
{
    view->priv = g_new0 (MozWebViewPriv, 1);
    view->priv->view = new MozView();
    view->priv->listener = new ViewListener(view);
    view->priv->view->SetListener(view->priv->listener);

    // XXX: should do one offscreen, not one-per-widget.
    view->priv->offscreen = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_realize(view->priv->offscreen);

    view->priv->view->CreateBrowser(view->priv->offscreen, 0, 0, 500, 500);
    view->priv->mozWidget = gtk_bin_get_child(GTK_BIN(view->priv->offscreen));
}

/*******************************************************************
 *                                API                              *
 *******************************************************************/

GtkWidget *
moz_web_view_new()
{
    return GTK_WIDGET(g_object_new(MOZ_TYPE_WEB_VIEW, NULL));
}

void
moz_web_view_load_uri(MozWebView *view, const gchar *uri)
{
    g_return_if_fail(MOZ_IS_WEB_VIEW(view));
    g_return_if_fail(uri && uri[0]);

    view->priv->view->LoadURI(uri);
}

void
moz_web_view_load_data(MozWebView  *view,
                       const gchar *base_uri,
                       const gchar *content_type,
                       const gchar *data,
                       gsize        len)
{
    g_return_if_fail(MOZ_IS_WEB_VIEW(view));
    g_return_if_fail(base_uri != NULL);
    g_return_if_fail(content_type != NULL);
    g_return_if_fail(data != NULL);
    g_return_if_fail(len > 0);

    view->priv->view->LoadData(base_uri, content_type, (PRUint8 *)data, (PRUint32)len);
}
