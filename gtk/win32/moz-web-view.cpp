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
#include <gtk/gtk.h>

// Mozilla header files
#include "embed.h"

// Local includes
#include "moz-web-view.h"
#include "moz-web-view-marshal.h"

#define GET_PRIV(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MOZ_TYPE_WEB_VIEW, MozWebViewPriv))

#ifdef TRACING
#  define DBG(x) x
#else
#  define DBG(x)
#endif

/* GObjectClass */
static void     moz_web_view_finalize            (GObject           *object);
static void     moz_web_view_set_property        (GObject           *object,
						  guint              prop_id,
						  const GValue      *value,
						  GParamSpec        *pspec);
static void     moz_web_view_get_property        (GObject           *object,
						  guint              prop_id,
						  GValue            *value,
						  GParamSpec        *pspec);

/* GtkWidgetClass */
static void     moz_web_view_realize             (GtkWidget         *widget);
static void     moz_web_view_unrealize           (GtkWidget         *widget);
static void     moz_web_view_hierarchy_changed   (GtkWidget         *widget,
						  GtkWidget         *previous_toplevel);
static void     moz_web_view_size_allocate       (GtkWidget         *widget,
						  GtkAllocation     *allocation);
static void     moz_web_view_map                 (GtkWidget         *widget);
static void     moz_web_view_unmap               (GtkWidget         *widget);

/* various local stubs */
static gboolean handle_toplevel_configure    (GtkWidget         *toplevel,
					      GdkEventConfigure *event,
					      GtkWidget         *widget);
static HWND     create_native_window         (GtkWidget         *widget);
static LRESULT  window_procedure             (HWND               hwnd,
					      UINT               message,
					      WPARAM             wparam,
					      LPARAM             lparam);

class MOZListener;
struct _MozWebViewPriv {
    /* Essential api to the gecko */
    MozApp      *app;
    MozView     *view;
    MOZListener *listener;

    /* Toplevel native window hack */
    HWND         native_window;

    /* Record some properties here */
    gchar       *requested_uri;
    gchar       *title;
};


enum {
    PROP_0,
    PROP_REQUESTED_URI,
    PROP_TITLE
};

enum {
    TITLE_CHANGED,
    STATUS_CHANGED,
    LOCATION_CHANGED,
    URI_REQUESTED,
    DOCUMENT_LOADED,
    LAST_SIGNAL
};

static GHashTable *hwnd_hash = NULL;
static guint       moz_web_view_signals[LAST_SIGNAL] = { 0 };

static guint
moz_handle_hash (HANDLE handle)
{
    return (guint) handle;
}

static gint
moz_handle_equal (HANDLE a,
		  HANDLE b)
{
    return (a == b);
}

G_DEFINE_TYPE (MozWebView, moz_web_view, GTK_TYPE_BIN)

/*******************************************************************
 *                      MOZListener here                           *
 *******************************************************************/
class MOZListener : public MozViewListener
{
    void SetTitle(const char* newTitle);
    void StatusChanged(const char* newStatus, PRUint32 statusType);
    void LocationChanged(const char* newLocation);
    PRBool OpenURI(const char* newLocation);
    void DocumentLoaded();
    
    GtkWidget *GetWidget();
};

void MOZListener::SetTitle(const char *new_title)
{
    GtkWidget *widget = this->GetWidget();
    
    g_object_set (G_OBJECT (widget), 
		  "page-title", new_title,
		  NULL);

    g_signal_emit(widget, moz_web_view_signals[TITLE_CHANGED], 0, new_title);
}

void MOZListener::StatusChanged(const char *new_status, PRUint32 flags)
{
    GtkWidget *widget = this->GetWidget();
    g_signal_emit(widget, moz_web_view_signals[STATUS_CHANGED], 0, new_status);
}

void MOZListener::LocationChanged(const char *new_uri)
{
    GtkWidget *widget = this->GetWidget();
    g_signal_emit(widget, moz_web_view_signals[LOCATION_CHANGED], 0, new_uri);
}

PRBool MOZListener::OpenURI(const char* new_uri)
{
    GtkWidget *widget = this->GetWidget();
    gboolean   abort_load = FALSE;
    
    g_signal_emit (widget, moz_web_view_signals[URI_REQUESTED], 0, 
		   new_uri, &abort_load);
    
    return abort_load;
}

void MOZListener::DocumentLoaded()
{
    GtkWidget *widget = this->GetWidget();
    g_signal_emit (widget, moz_web_view_signals[DOCUMENT_LOADED], 0);
}

GtkWidget *MOZListener::GetWidget()
{
    HWND hwnd = (HWND)pMozView->GetParentWindow();
    GtkWidget *widget = (GtkWidget *)g_hash_table_lookup (hwnd_hash, hwnd);
    return widget;
}

/*******************************************************************
 *                      Class Initialization                       *
 *******************************************************************/
static void
moz_web_view_class_init (MozWebViewClass *klass)
{
    GObjectClass     *object_klass = G_OBJECT_CLASS (klass);
    GtkWidgetClass   *widget_klass = GTK_WIDGET_CLASS (klass);

    object_klass->finalize      = moz_web_view_finalize;
    object_klass->set_property  = moz_web_view_set_property;
    object_klass->get_property  = moz_web_view_get_property;
    
    widget_klass->realize           = moz_web_view_realize;
    widget_klass->unrealize         = moz_web_view_unrealize;
    widget_klass->hierarchy_changed = moz_web_view_hierarchy_changed;
    widget_klass->size_allocate     = moz_web_view_size_allocate;
    widget_klass->map               = moz_web_view_map;
    widget_klass->unmap             = moz_web_view_unmap;
    
    g_object_class_install_property (object_klass,
				     PROP_REQUESTED_URI,
				     g_param_spec_string
				     ("requested-uri",
				      "Requested URI",
				      "the requested uri",
				      NULL,
				      (GParamFlags)G_PARAM_READWRITE));
    
    g_object_class_install_property (object_klass,
				     PROP_TITLE,
				     g_param_spec_string
				     ("page-title",
				      "Page title",
				      "the current webpage title",
				      NULL,
				      (GParamFlags)G_PARAM_READWRITE));
    

    moz_web_view_signals[TITLE_CHANGED] =
	g_signal_new ("title-changed",
		      G_TYPE_FROM_CLASS(klass),
		      G_SIGNAL_RUN_FIRST,
		      G_STRUCT_OFFSET(MozWebViewClass, title_changed),
		      NULL, NULL,
		      g_cclosure_marshal_VOID__STRING,
		      G_TYPE_NONE, 1, G_TYPE_STRING);
    
    moz_web_view_signals[STATUS_CHANGED] =
	g_signal_new ("status-changed",
		      G_TYPE_FROM_CLASS(klass),
		      G_SIGNAL_RUN_FIRST,
		      G_STRUCT_OFFSET(MozWebViewClass, status_changed),
		      NULL, NULL,
		      g_cclosure_marshal_VOID__STRING,
		      G_TYPE_NONE, 1, G_TYPE_STRING);
    
    moz_web_view_signals[LOCATION_CHANGED] =
	g_signal_new ("location-changed",
		      G_TYPE_FROM_CLASS(klass),
		      G_SIGNAL_RUN_FIRST,
		      G_STRUCT_OFFSET(MozWebViewClass, location_changed),
		      NULL, NULL,
		      g_cclosure_marshal_VOID__STRING,
		      G_TYPE_NONE, 1, G_TYPE_STRING);
    
    moz_web_view_signals[URI_REQUESTED] = 
	g_signal_new ("uri-requested",
		      G_TYPE_FROM_CLASS (object_klass),
		      G_SIGNAL_RUN_LAST,
		      G_STRUCT_OFFSET (MozWebViewClass, uri_requested),
		      g_signal_accumulator_true_handled, NULL,
		      g_cclosure_user_marshal_BOOLEAN__STRING,
		      G_TYPE_BOOLEAN, 1, G_TYPE_STRING);
    
    moz_web_view_signals[DOCUMENT_LOADED] = 
	g_signal_new ("document-loaded",
		      G_TYPE_FROM_CLASS (object_klass),
		      G_SIGNAL_RUN_LAST,
		      G_STRUCT_OFFSET (MozWebViewClass, document_loaded),
		      NULL, NULL,
		      g_cclosure_marshal_VOID__VOID,
		      G_TYPE_NONE, 0);
    
    g_type_class_add_private (object_klass, sizeof (MozWebViewPriv));
}

static void
moz_web_view_init (MozWebView *view)
{
    MozWebViewPriv *priv;
    
    view->priv = priv = GET_PRIV (view);
    
    GTK_WIDGET_SET_FLAGS (view, GTK_NO_WINDOW);
    GTK_WIDGET_SET_FLAGS (view, GTK_CAN_FOCUS);
    
    priv->app = new MozApp ();
    priv->view = new MozView ();
    priv->listener = new MOZListener ();
    priv->view->SetListener(priv->listener);
}

/*******************************************************************
 *                           GObjectClass                          *
 *******************************************************************/
static void
moz_web_view_finalize (GObject *object)
{
    MozWebViewPriv *priv;
    
    priv = GET_PRIV (object);
    
    /* The WebBrowserChrome object will be freed by its 
     * smart pointer being freed
     */
    if (priv->requested_uri)
	g_free (priv->requested_uri);
    if (priv->title)
	g_free (priv->title);
    
    (G_OBJECT_CLASS (moz_web_view_parent_class)->finalize) (object);
}


static void 
moz_web_view_set_property (GObject      *object,
			   guint         prop_id,
			   const GValue *value,
			   GParamSpec   *pspec)
{
    MozWebViewPriv *priv;

    priv = GET_PRIV (object);
	
    switch (prop_id) {
    case PROP_REQUESTED_URI:
	moz_web_view_load_uri (MOZ_WEB_VIEW (object), g_value_get_string (value));
	break;
    case PROP_TITLE:
	if (priv->title)
	    g_free (priv->title);
	priv->title = g_value_dup_string (value);
	break;
    default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;
    }
}

static void 
moz_web_view_get_property (GObject     *object,
		       guint        prop_id,
		       GValue      *value,
		       GParamSpec  *pspec)
{ 
    MozWebViewPriv *priv;
    
    priv = GET_PRIV (object);
    
    switch (prop_id) {
    case PROP_REQUESTED_URI:
	g_value_set_string (value, priv->requested_uri);
	break;
    case PROP_TITLE:
	g_value_set_string (value, priv->title);
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
moz_web_view_realize (GtkWidget *widget)
{
    MozWebViewPriv *priv;
    GdkWindowAttr attributes;
    gint attributes_mask;
    
    priv = GET_PRIV (widget);
    
    GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
    
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual (widget);
    attributes.colormap = gtk_widget_get_colormap (widget);
    attributes.event_mask = gtk_widget_get_events (widget) |  //| GDK_ALL_EVENTS_MASK;
	GDK_EXPOSURE_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK |
	GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
	GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_FOCUS_CHANGE_MASK;
    
    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
    
    widget->window = gtk_widget_get_parent_window (widget);
    g_object_ref (widget->window);
    
    priv->native_window = create_native_window (widget);
    
    priv->view->CreateBrowser(priv->native_window, 0, 0,
			      widget->allocation.width, widget->allocation.height);
    
    widget->style = gtk_style_attach (widget->style, widget->window);
    gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
    
    if (priv->requested_uri)
	moz_web_view_load_uri (MOZ_WEB_VIEW (widget), priv->requested_uri);
}

static void
moz_web_view_unrealize (GtkWidget *widget)
{
    MozWebViewPriv *priv = GET_PRIV (widget);
    
    if (!DestroyWindow (priv->native_window))
	g_warning ("Problems destoying native view window");
    
    g_hash_table_remove (hwnd_hash, priv->native_window);
    
    priv->native_window = NULL;
    
    GTK_WIDGET_CLASS (moz_web_view_parent_class)->unrealize (widget);
}

static void
moz_web_view_hierarchy_changed (GtkWidget         *widget,
			    GtkWidget         *previous_toplevel)
{
    GtkWidget *toplevel = gtk_widget_get_toplevel(widget);

    /* At least toplevel or previous_toplevel should exist */
    g_assert (toplevel || previous_toplevel);

    DBG(g_print("Hierarchy changed ! toplevel %p previous_toplevel %p\n", toplevel, previous_toplevel));
    
    if (GTK_IS_WINDOW (toplevel)) {
	g_signal_connect (G_OBJECT (toplevel), "configure-event",
			  G_CALLBACK(handle_toplevel_configure), widget);
    }	
    
    if (GTK_IS_WINDOW (previous_toplevel)) {
	g_signal_handlers_disconnect_by_func
	    (G_OBJECT(previous_toplevel), 
	     (gpointer)handle_toplevel_configure, widget);
    }
}

static void
moz_web_view_size_allocate (GtkWidget     *widget,
			GtkAllocation *allocation)
{
    MozWebViewPriv *priv = GET_PRIV (widget);
    
    g_return_if_fail (allocation != NULL);
    
    widget->allocation = *allocation;
    
    if (GTK_WIDGET_REALIZED (widget)) {
	gint x, y;
	
	gdk_window_get_position (widget->window, &x, &y);
	x += allocation->x;
	y += allocation->y;
	
	SetWindowPos (priv->native_window, NULL,
		      x, y,
		      allocation->width, allocation->height,
		      SWP_NOACTIVATE | SWP_NOZORDER);
	
	priv->view->SetPositionAndSize(0, 0, allocation->width, allocation->height);
    }
}

static void
moz_web_view_map (GtkWidget *widget)
{
    MozWebViewPriv *priv = GET_PRIV (widget);
    
    /* This will ensure a realized widget */
    GTK_WIDGET_CLASS (moz_web_view_parent_class)->map (widget);
    
    ShowWindow (priv->native_window, SW_SHOW);
}

static void
moz_web_view_unmap (GtkWidget *widget)
{
    MozWebViewPriv *priv = GET_PRIV (widget);
    
    GTK_WIDGET_CLASS (moz_web_view_parent_class)->unmap (widget);
    
    ShowWindow (priv->native_window, SW_HIDE);
}

/*******************************************************************
 *                          Various Stubs                          *
 *******************************************************************/
static gboolean 
handle_toplevel_configure (GtkWidget         *toplevel,
			   GdkEventConfigure *event,
			   GtkWidget         *widget)
{
    MozWebViewPriv *priv;
    gint x, y;

    priv = GET_PRIV (widget);

    DBG(g_print ("Configure event !\n"));
    
    gdk_window_get_position (widget->window, &x, &y);
    x += widget->allocation.x;
    y += widget->allocation.y;
    
    SetWindowPos (priv->native_window, NULL,
		  x, y,
		  widget->allocation.width, widget->allocation.height,
		  SWP_NOACTIVATE | SWP_NOZORDER);
    
    return FALSE;
}

static LRESULT 
window_procedure (HWND   hwnd,
		  UINT   message,
		  WPARAM wparam,
		  LPARAM lparam)
{
    GtkWidget *widget;
    MozWebViewPriv *priv;

    widget = (GtkWidget *)g_hash_table_lookup (hwnd_hash, hwnd);
	
    if (widget)
	priv = GET_PRIV (widget);
    else {
	DBG(g_print ("message %d unhandled (no widget data in hash table)\n", message));
	return DefWindowProcW (hwnd, message, wparam, lparam);
    }
    
    switch (message) {
    case WM_INITDIALOG:
	DBG(g_print ("InitDialog message..\n"));
	return TRUE;
    case WM_INITMENU: 
	DBG(g_print ("InitMenu message..\n"));
	return TRUE;
    case WM_SYSCOMMAND:
	if (wparam == SC_CLOSE) {
	    DBG(g_print ("Close message..\n"));
	    return TRUE;
	}
	break;
    case WM_DESTROY:
	return TRUE;
	
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
    return DefWindowProcW (hwnd, message, wparam, lparam);
}

static HWND
create_native_window (GtkWidget *widget)
{
    static ATOM klass = 0;
    HWND window_handle, parent_handle;
    DWORD dwStyle, dwExStyle;
    
    if (!hwnd_hash)
	hwnd_hash = g_hash_table_new ((GHashFunc) moz_handle_hash,
				      (GEqualFunc) moz_handle_equal);
    
    if (!klass) {
	static WNDCLASSEXW wcl; 
	
	wcl.cbSize = sizeof (WNDCLASSEX);
	wcl.style = 0;

	/* DON'T set CS_<H,V>REDRAW. It causes total redraw
	 * on WM_SIZE and WM_MOVE. Flicker, Performance!
	 */
	wcl.lpfnWndProc = (WNDPROC)window_procedure;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hInstance = GetModuleHandle (NULL);
	wcl.hIcon = 0;
	wcl.hIconSm = 0;
	wcl.lpszMenuName = NULL;
	wcl.hIcon = NULL;
	wcl.hIconSm = NULL;
	wcl.hbrBackground = NULL;
	wcl.hCursor = LoadCursor (NULL, IDC_ARROW); 
	wcl.lpszClassName = L"MozWindow";
	
	/* for child windows only... */
	//wcl.style |= CS_PARENTDC; /* MSDN: ... enhances system performance. */
	klass = RegisterClassExW (&wcl);
	
    }
    
    dwExStyle = WS_EX_TOOLWINDOW; // for popup windows 
    dwStyle = WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS; // popup window
    
    parent_handle = (HWND)GDK_WINDOW_HWND (gtk_widget_get_parent_window (widget));
    window_handle = CreateWindowExW (dwExStyle,
				     MAKEINTRESOURCEW (klass),
				     L"",
				     dwStyle,
				     0, 0, 1, 1,
				     parent_handle,
				     NULL,
				     GetModuleHandle (NULL),
				     NULL);
    
    g_hash_table_insert (hwnd_hash, window_handle, widget);
    
    return window_handle;
}

/*******************************************************************
 *                                API                              *
 *******************************************************************/
GtkWidget *
moz_web_view_new (void)
{
    return GTK_WIDGET (g_object_new (MOZ_TYPE_WEB_VIEW, NULL));
}

void
moz_web_view_load_uri (MozWebView *view, const gchar  *uri)
{	
    MozWebViewPriv *priv; 
    
    g_return_if_fail (MOZ_IS_WEB_VIEW (view));
    g_return_if_fail (uri != NULL);

    priv = GET_PRIV (view);

    if (priv->requested_uri && priv->requested_uri != uri)
	g_free (priv->requested_uri);
    
    priv->requested_uri = g_strdup (uri);
    
    if (GTK_WIDGET_REALIZED (view))
	priv->view->LoadURI (priv->requested_uri);
    
    g_object_notify (G_OBJECT (view), "requested-uri");
}

void
moz_web_view_load_data (MozWebView  *view,
			const gchar *base_uri,
			const gchar *content_type,
			const gchar *data,
			gsize        len)
{
    MozWebViewPriv *priv; 
    
    g_return_if_fail (MOZ_IS_WEB_VIEW (view));
    g_return_if_fail (base_uri != NULL);
    g_return_if_fail (content_type != NULL);
    g_return_if_fail (data != NULL);
    g_return_if_fail (len > 0);

    priv = GET_PRIV (view);

    if (GTK_WIDGET_REALIZED (view))
	priv->view->LoadData(base_uri, content_type, (PRUint8 *)data, (PRUint32)len);
}

gchar *
moz_web_view_get_title (MozWebView *view)
{
    MozWebViewPriv *priv; 
    
    g_return_val_if_fail (MOZ_IS_WEB_VIEW (view), NULL);

    priv = GET_PRIV (view);

    return g_strdup (priv->title);
}

gboolean
moz_web_view_set_user_agent (MozWebView  *view,
			     const gchar *user_agent)
{
    nsresult rv;
    MozWebViewPriv *priv;

    g_return_val_if_fail (MOZ_IS_WEB_VIEW (view), FALSE);
    g_return_val_if_fail (user_agent != NULL, FALSE);

    priv = GET_PRIV (view);
    rv = priv->app->SetCharPref("general.useragent.override", user_agent);
    
    return TRUE; //!NS_FAILED (rv);
}
