/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Pioneer Research Center USA, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *    Tristan Van Berkom <tristan.van.berkom@gmail.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

// Glib/System includes
#include <string.h>
#include <gdk/gdkwin32.h>

// Local includes
#include "moz-web-view-common.h"

#define GET_PRIV(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MOZ_TYPE_WEB_VIEW, MozWebViewPriv))

//#define TRACING

#ifdef TRACING
#  define DBG(x) x
#else
#  define DBG(x)
#endif


/* GObjectClass */
static void moz_web_view_finalize     (GObject      *object);
static void moz_web_view_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec);
static void moz_web_view_get_property (GObject      *object,
                                       guint         prop_id,
                                       GValue       *value,
                                       GParamSpec   *pspec);

/* GtkWidgetClass */
static void moz_web_view_realize           (GtkWidget         *widget);
static void moz_web_view_unrealize         (GtkWidget         *widget);
static void moz_web_view_hierarchy_changed (GtkWidget         *widget,
                                            GtkWidget         *previous_toplevel);
static void moz_web_view_size_allocate     (GtkWidget         *widget,
                                            GtkAllocation     *allocation);
static void moz_web_view_map               (GtkWidget         *widget);
static void moz_web_view_unmap             (GtkWidget         *widget);

/* various local stubs */
static gboolean handle_toplevel_configure (GtkWidget         *toplevel,
                                           GdkEventConfigure *event,
                                           GtkWidget         *widget);
static HWND     create_native_window      (GtkWidget         *widget);
static LRESULT  window_procedure          (HWND               hwnd,
                                           UINT               message,
                                           WPARAM             wparam,
                                           LPARAM             lparam);

class ViewListener;
struct _MozWebViewPriv {
    /* Essential api to the gecko */
    MozApp       *app;
    MozView      *view;
    ViewListener *listener;

    /* Toplevel native window hack */
    HWND         native_window;

    /* Record some properties here */
    gchar       *requested_uri;
    gchar       *title;
    gchar       *status;
    gchar       *location;
};

static guint signals[LAST_SIGNAL] = { 0 };

static void
moz_web_view_viewable_init (MozViewableIface *iface) {}

G_DEFINE_TYPE_WITH_CODE (MozWebView, moz_web_view, GTK_TYPE_BIN,
                         G_IMPLEMENT_INTERFACE (MOZ_TYPE_VIEWABLE,
                                                moz_web_view_viewable_init))

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

/*******************************************************************
 *                      Class Initialization                       *
 *******************************************************************/
static void
moz_web_view_class_init(MozWebViewClass *klass)
{
    GObjectClass     *object_klass = G_OBJECT_CLASS(klass);
    GtkWidgetClass   *widget_klass = GTK_WIDGET_CLASS(klass);

    object_klass->finalize      = moz_web_view_finalize;
    object_klass->get_property  = moz_web_view_get_property;

    widget_klass->realize           = moz_web_view_realize;
    widget_klass->unrealize         = moz_web_view_unrealize;
    widget_klass->hierarchy_changed = moz_web_view_hierarchy_changed;
    widget_klass->size_allocate     = moz_web_view_size_allocate;
    widget_klass->map               = moz_web_view_map;
    widget_klass->unmap             = moz_web_view_unmap;

    signals[TITLE_CHANGED] = g_signal_lookup ("title-changed", MOZ_TYPE_WEB_VIEW);
    signals[STATUS_CHANGED] = g_signal_lookup ("status-changed", MOZ_TYPE_WEB_VIEW);
    signals[LOCATION_CHANGED] = g_signal_lookup ("location-changed", MOZ_TYPE_WEB_VIEW);
    signals[URI_REQUESTED] = g_signal_lookup ("uri-requested", MOZ_TYPE_WEB_VIEW);
    signals[DOCUMENT_LOADED] = g_signal_lookup ("document-loaded", MOZ_TYPE_WEB_VIEW);

    /* Implement MozViewable properties */
    g_object_class_override_property(object_klass, PROP_REQUESTED_URI, "requested-uri");
    g_object_class_override_property(object_klass, PROP_TITLE, "title");
    g_object_class_override_property(object_klass, PROP_STATUS, "status");
    g_object_class_override_property(object_klass, PROP_LOCATION, "location");

    g_type_class_add_private(object_klass, sizeof(MozWebViewPriv));
}

static void
moz_web_view_init(MozWebView *view)
{
    MozWebViewPriv *priv;

    view->priv = priv = GET_PRIV(view);

    GTK_WIDGET_SET_FLAGS(view, GTK_NO_WINDOW);
    GTK_WIDGET_SET_FLAGS(view, GTK_CAN_FOCUS);

    priv->app = new MozApp();
    priv->view = new MozView();
    priv->listener = new ViewListener(view);
    priv->view->SetListener(priv->listener);
}

/*******************************************************************
 *                           GObjectClass                          *
 *******************************************************************/
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

    delete view->priv->app;
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

/*******************************************************************
 *                            GtkWidgetClass                       *
 *******************************************************************/
static void
moz_web_view_realize(GtkWidget *widget)
{
    MozWebViewPriv *priv = GET_PRIV(widget);
    GdkWindowAttr attributes;
    gint attributes_mask;

    GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual(widget);
    attributes.colormap = gtk_widget_get_colormap(widget);
    attributes.event_mask = gtk_widget_get_events(widget) |  //| GDK_ALL_EVENTS_MASK;
	GDK_EXPOSURE_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK |
	GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
	GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_FOCUS_CHANGE_MASK;

    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

    widget->window = gtk_widget_get_parent_window(widget);
    g_object_ref(widget->window);

    priv->native_window = create_native_window(widget);

    priv->view->CreateBrowser(priv->native_window, 0, 0,
			      widget->allocation.width, widget->allocation.height);

    widget->style = gtk_style_attach(widget->style, widget->window);
    gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);

    if (priv->requested_uri)
        moz_web_view_load_uri(MOZ_WEB_VIEW(widget), priv->requested_uri);
}

static void
moz_web_view_unrealize(GtkWidget *widget)
{
    MozWebViewPriv *priv = GET_PRIV(widget);

    RemoveProp(priv->native_window, "moz-view-widget");

    if (!DestroyWindow(priv->native_window))
	g_warning("Problems destoying native view window");

    priv->native_window = NULL;

    GTK_WIDGET_CLASS(moz_web_view_parent_class)->unrealize(widget);
}

static void
moz_web_view_hierarchy_changed(GtkWidget         *widget,
                               GtkWidget         *previous_toplevel)
{
    GtkWidget *toplevel = gtk_widget_get_toplevel(widget);

    /* At least toplevel or previous_toplevel should exist */
    g_assert(toplevel || previous_toplevel);

    DBG(g_print("Hierarchy changed ! toplevel %p previous_toplevel %p\n", toplevel, previous_toplevel));

    if (GTK_IS_WINDOW(toplevel)) {
        g_signal_connect(toplevel, "configure-event",
                         G_CALLBACK(handle_toplevel_configure), widget);
    }

    if (GTK_IS_WINDOW(previous_toplevel)) {
        g_signal_handlers_disconnect_by_func(previous_toplevel,
                                             (gpointer)handle_toplevel_configure,
                                             widget);
    }
}

static void
moz_web_view_size_allocate(GtkWidget     *widget,
                           GtkAllocation *allocation)
{
    MozWebViewPriv *priv = GET_PRIV(widget);

    g_return_if_fail(allocation != NULL);

    widget->allocation = *allocation;

    if (GTK_WIDGET_REALIZED(widget)) {
        gint x, y;

        gdk_window_get_position(widget->window, &x, &y);
        x += allocation->x;
        y += allocation->y;

        SetWindowPos(priv->native_window, NULL, x, y,
                     allocation->width, allocation->height,
                     SWP_NOACTIVATE | SWP_NOZORDER);

        priv->view->SetPositionAndSize(0, 0, allocation->width, allocation->height);
    }
}

static void
moz_web_view_map(GtkWidget *widget)
{
    MozWebViewPriv *priv = GET_PRIV(widget);

    /* This will ensure a realized widget */
    GTK_WIDGET_CLASS(moz_web_view_parent_class)->map(widget);

    ShowWindow(priv->native_window, SW_SHOW);
}

static void
moz_web_view_unmap(GtkWidget *widget)
{
    MozWebViewPriv *priv = GET_PRIV(widget);

    GTK_WIDGET_CLASS(moz_web_view_parent_class)->unmap(widget);

    ShowWindow(priv->native_window, SW_HIDE);
}

/* We handle configure events on the toplevel in order to
 * reposition our window when the toplevel moves.
 */
static gboolean
handle_toplevel_configure(GtkWidget         *toplevel,
                          GdkEventConfigure *event,
                          GtkWidget         *widget)
{
    MozWebViewPriv *priv;
    gint x, y;

    priv = GET_PRIV(widget);

    DBG(g_print("Configure event !\n"));

    gdk_window_get_position(widget->window, &x, &y);
    x += widget->allocation.x;
    y += widget->allocation.y;

    SetWindowPos(priv->native_window, NULL, x, y,
                 widget->allocation.width, widget->allocation.height,
                 SWP_NOACTIVATE | SWP_NOZORDER);

    return FALSE;
}

/* Handle the window procedure by the window class directly
 * in order to call SetFocus() at the appropriate times.
 */
static LRESULT
window_procedure(HWND   hwnd,
		  UINT   message,
		  WPARAM wparam,
		  LPARAM lparam)
{
    GtkWidget *widget;
    MozWebViewPriv *priv;

    widget = (GtkWidget *)GetProp(hwnd, "moz-view-widget");

    DBG(g_print("Procedure called ! widget %p\n", widget));


    /* The first few messages are fired inside CreateWindowEx(), so we
     * havent set the widget data yet.
     */
    if (widget)
        priv = GET_PRIV(widget);
    else
        return DefWindowProcW(hwnd, message, wparam, lparam);

    switch (message) {
        case WM_INITDIALOG:
        case WM_INITMENU:
        case WM_DESTROY:
            return TRUE;
        case WM_SYSCOMMAND:
            if (wparam == SC_CLOSE) {
                DBG(g_print("Close message..\n"));
                return TRUE;
            }
        break;
        case WM_ACTIVATE:
            switch (wparam) {
                case WA_ACTIVE:
                case WA_CLICKACTIVE:
                    priv->view->SetFocus(true);
                break;
                case WA_INACTIVE:
                    priv->view->SetFocus(false);
                break;
                default:
                break;
            }
        break;
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

/*****************************************************
 * Hack ahead !!!
 *
 * Problems here have to do with focus handling, i.e.
 * sharing and passing keyboard focus back and forth from
 * the gecko window to the rest of the app. The gecko
 * wants us to turn focus on and off when we receive the
 * WM_ACTIVATE message for our window; Gtk+ does not give
 * us an opportunity to act on this message (TODO: patch
 * gtk+ to do so and run tests).
 *
 * Also tried to turn on and off focus near when activate
 * messages come (i.e. focus in/out of the toplevel window)
 * with no luck (works to get started, but the gecko
 * will never relinquish focus afterwords).
 *
 * The current hack/workaround:
 *   We are using a native toplevel window, that we reposition
 * to follow the widget hierarchy on toplevel configure events,
 * this way we handle the WM_ACTIVATE messages and focus handleing
 * works, on the other hand accelerators keys tied into the higher
 * level window wont trigger when the gecko has focus (is that
 * already the case ?) and the apps toplevel will be painted as
 * an inactive window.
 */
static HWND
create_native_window(GtkWidget *widget)
{
    static ATOM klass = 0;
    HWND window_handle, parent_handle;
    DWORD dwStyle, dwExStyle;

    if (!klass) {
         static WNDCLASSEXW wcl;

         wcl.cbSize = sizeof(WNDCLASSEX);
         wcl.style = 0;

        /* DON'T set CS_<H,V>REDRAW. It causes total redraw
         * on WM_SIZE and WM_MOVE. Flicker, Performance!
         */
         wcl.lpfnWndProc = (WNDPROC)window_procedure;
         wcl.cbClsExtra = 0;
         wcl.cbWndExtra = 0;
         wcl.hInstance = GetModuleHandle(NULL);
         wcl.hIcon = 0;
         wcl.hIconSm = 0;
         wcl.lpszMenuName = NULL;
         wcl.hIcon = NULL;
         wcl.hIconSm = NULL;
         wcl.hbrBackground = NULL;
         wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
         wcl.lpszClassName = L"MozWindow";

         klass = RegisterClassExW(&wcl);
    }

    dwExStyle = WS_EX_TOOLWINDOW; // for popup windows
    dwStyle = WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS; // popup window

    parent_handle = (HWND)GDK_WINDOW_HWND(gtk_widget_get_parent_window(widget));
    window_handle = CreateWindowExW(dwExStyle,
                                    MAKEINTRESOURCEW(klass),
                                    L"",
                                    dwStyle,
                                    0, 0, 1, 1,
                                    parent_handle,
                                    NULL,
                                    GetModuleHandle(NULL),
                                    NULL);

    SetProp(window_handle, "moz-view-widget", (HANDLE)widget);

    return window_handle;
}

/*******************************************************************
 *                                API                              *
 *******************************************************************/
GtkWidget *
moz_web_view_new(void)
{
    return GTK_WIDGET(g_object_new(MOZ_TYPE_WEB_VIEW, NULL));
}

void
moz_web_view_load_uri(MozWebView *view, const gchar *uri)
{
    g_return_if_fail(MOZ_IS_WEB_VIEW(view));
    g_return_if_fail(uri && uri[0]);

	if (GTK_WIDGET_REALIZED (view))
	{
	    view->priv->view->LoadURI(uri);
	}
	else
	{
		/* widget not realized, the requested uri will be loaded when realized */
		g_free (view->priv->requested_uri);
		view->priv->requested_uri = g_strdup (uri);
	}
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

    if (GTK_WIDGET_REALIZED(view))
        view->priv->view->LoadData(base_uri, content_type, (PRUint8 *)data, (PRUint32)len);
}
