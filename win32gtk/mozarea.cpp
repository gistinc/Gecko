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
#include "mozarea.h"
#include "mozarea-marshal.h"

#define GET_PRIV(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MOZ_TYPE_AREA, MozAreaPriv))


#ifdef TRACING
#  define DBG(x) x
#else
#  define DBG(x)
#endif


/* GObjectClass */
static void     moz_area_finalize            (GObject           *object);
static void     moz_area_set_property        (GObject           *object,
					   guint              prop_id,
					   const GValue      *value,
					   GParamSpec        *pspec);
static void     moz_area_get_property        (GObject           *object,
					   guint              prop_id,
					   GValue            *value,
					   GParamSpec        *pspec);

/* GtkWidgetClass */
static void     moz_area_realize             (GtkWidget         *widget);
static void     moz_area_unrealize           (GtkWidget         *widget);
static void     moz_area_hierarchy_changed   (GtkWidget         *widget,
					      GtkWidget         *previous_toplevel);
static void     moz_area_size_allocate       (GtkWidget         *widget,
					      GtkAllocation     *allocation);
static void     moz_area_map                 (GtkWidget         *widget);
static void     moz_area_unmap               (GtkWidget         *widget);

/* various local stubs */
static gboolean handle_toplevel_configure    (GtkWidget         *toplevel,
					      GdkEventConfigure *event,
					      GtkWidget         *widget);
static HWND     create_native_window         (GtkWidget         *widget);
static LRESULT  moz_area_window_procedure    (HWND               hwnd,
					      UINT               message,
					      WPARAM             wparam,
					      LPARAM             lparam);

G_DEFINE_TYPE (MozArea, moz_area, GTK_TYPE_WIDGET)

class MOZListener;
typedef struct MozAreaPriv MozAreaPriv;
struct MozAreaPriv {
    /* Essential api to the gecko */
    MozApp      *app;
    MozView     *view;
    MOZListener *listener;

    /* Toplevel native window hack */
    HWND         native_window;

    /* Record some properties here */
    gchar       *requested_url;
    gchar       *title;

};


enum {
    PROP_0,
    PROP_REQUESTED_URL,
    PROP_TITLE
};

enum {
    URL_REQUESTED,
    DOCUMENT_LOADED,
    LAST_SIGNAL
};

static GHashTable *hwnd_hash = NULL;
static guint       moz_area_signals[LAST_SIGNAL] = { 0 };

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

void MOZListener::SetTitle(const char *newTitle)
{
    GtkWidget *widget = this->GetWidget();
    
    g_object_set (G_OBJECT (widget), 
		  "page-title", newTitle,
		  NULL);
}

void MOZListener::StatusChanged(const char *newStatus, PRUint32 statusType)
{
}

void MOZListener::LocationChanged(const char *newLocation)
{
}

PRBool MOZListener::OpenURI(const char* newLocation)
{
    GtkWidget *widget = this->GetWidget();
    gboolean   abort_load = FALSE;
    
    g_signal_emit (widget, moz_area_signals[URL_REQUESTED], 0, 
		   newLocation, &abort_load);
    
    return abort_load;
}

void MOZListener::DocumentLoaded()
{
    GtkWidget *widget = this->GetWidget();
    g_signal_emit (widget, moz_area_signals[DOCUMENT_LOADED], 0);
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
moz_area_class_init (MozAreaClass *klass)
{
    GObjectClass     *object_klass = G_OBJECT_CLASS (klass);
    GtkWidgetClass   *widget_klass = GTK_WIDGET_CLASS (klass);

    object_klass->finalize      = moz_area_finalize;
    object_klass->set_property  = moz_area_set_property;
    object_klass->get_property  = moz_area_get_property;
    
    widget_klass->realize           = moz_area_realize;
    widget_klass->unrealize         = moz_area_unrealize;
    widget_klass->hierarchy_changed = moz_area_hierarchy_changed;
    widget_klass->size_allocate     = moz_area_size_allocate;
    widget_klass->map               = moz_area_map;
    widget_klass->unmap             = moz_area_unmap;
    
    g_object_class_install_property (object_klass,
				     PROP_REQUESTED_URL,
				     g_param_spec_string
				     ("requested-url",
				      "Requested URL",
				      "the requested url",
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
    
    moz_area_signals[URL_REQUESTED] = 
	g_signal_new ("url-requested",
		      G_TYPE_FROM_CLASS (object_klass),
		      G_SIGNAL_RUN_LAST,
		      G_STRUCT_OFFSET (MozAreaClass, url_requested),
		      g_signal_accumulator_true_handled, NULL,
		      g_cclosure_user_marshal_BOOLEAN__STRING,
		      G_TYPE_BOOLEAN, 1, G_TYPE_STRING);
    
    moz_area_signals[DOCUMENT_LOADED] = 
	g_signal_new ("document-loaded",
		      G_TYPE_FROM_CLASS (object_klass),
		      G_SIGNAL_RUN_LAST,
		      G_STRUCT_OFFSET (MozAreaClass, document_loaded),
		      NULL, NULL,
		      g_cclosure_marshal_VOID__VOID,
		      G_TYPE_NONE, 0);
    
    g_type_class_add_private (object_klass, sizeof (MozAreaPriv));
}

static void
moz_area_init (MozArea *area)
{
    MozAreaPriv *priv;
    
    priv = GET_PRIV (area);
    
    GTK_WIDGET_SET_FLAGS (area, GTK_NO_WINDOW);
    GTK_WIDGET_SET_FLAGS (area, GTK_CAN_FOCUS);
    
    priv->app = new MozApp ();
    priv->view = new MozView ();
    priv->listener = new MOZListener ();
    priv->view->SetListener(priv->listener);
}

/*******************************************************************
 *                           GObjectClass                          *
 *******************************************************************/
static void
moz_area_finalize (GObject *object)
{
    MozAreaPriv *priv;
    
    priv = GET_PRIV (object);
    
    /* The WebBrowserChrome object will be freed by its 
     * smart pointer being freed
     */
    if (priv->requested_url)
	g_free (priv->requested_url);
    if (priv->title)
	g_free (priv->title);
    
    (G_OBJECT_CLASS (moz_area_parent_class)->finalize) (object);
}


static void 
moz_area_set_property (GObject      *object,
		       guint         prop_id,
		       const GValue *value,
		       GParamSpec   *pspec)
{
    MozAreaPriv *priv;

    priv = GET_PRIV (object);
	
    switch (prop_id) {
    case PROP_REQUESTED_URL:
	moz_area_display_url (MOZ_AREA (object), g_value_get_string (value));
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
moz_area_get_property (GObject     *object,
		       guint        prop_id,
		       GValue      *value,
		       GParamSpec  *pspec)
{ 
    MozAreaPriv *priv;
    
    priv = GET_PRIV (object);
    
    switch (prop_id) {
    case PROP_REQUESTED_URL:
	g_value_set_string (value, priv->requested_url);
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
moz_area_realize (GtkWidget *widget)
{
    MozAreaPriv *priv;
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
    
    if (priv->requested_url)
	moz_area_display_url (MOZ_AREA (widget), priv->requested_url);
}

static void
moz_area_unrealize (GtkWidget *widget)
{
    MozAreaPriv *priv = GET_PRIV (widget);
    
    if (!DestroyWindow (priv->native_window))
	g_warning ("Problems destoying native area window");
    
    g_hash_table_remove (hwnd_hash, priv->native_window);
    
    priv->native_window = NULL;
    
    GTK_WIDGET_CLASS (moz_area_parent_class)->unrealize (widget);
}

static void
moz_area_hierarchy_changed (GtkWidget         *widget,
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
moz_area_size_allocate (GtkWidget     *widget,
			GtkAllocation *allocation)
{
    MozAreaPriv *priv = GET_PRIV (widget);
    
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
moz_area_map (GtkWidget *widget)
{
    MozAreaPriv *priv = GET_PRIV (widget);
    
    /* This will ensure a realized widget */
    GTK_WIDGET_CLASS (moz_area_parent_class)->map (widget);
    
    ShowWindow (priv->native_window, SW_SHOW);
}

static void
moz_area_unmap (GtkWidget *widget)
{
    MozAreaPriv *priv = GET_PRIV (widget);
    
    GTK_WIDGET_CLASS (moz_area_parent_class)->unmap (widget);
    
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
    MozAreaPriv *priv;
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
moz_area_window_procedure (HWND   hwnd,
			   UINT   message,
			   WPARAM wparam,
			   LPARAM lparam)
{
    GtkWidget *widget;
    MozAreaPriv *priv;

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
	wcl.lpfnWndProc = (WNDPROC)moz_area_window_procedure;
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
moz_area_new (void)
{
    return GTK_WIDGET (g_object_new (MOZ_TYPE_AREA, NULL));
}

void
moz_area_display_url (MozArea *area, const gchar  *url)
{	
    MozAreaPriv *priv; 
    
    g_return_if_fail (MOZ_IS_AREA (area));
    g_return_if_fail (url != NULL);

    priv = GET_PRIV (area);

    if (priv->requested_url && priv->requested_url != url)
	g_free (priv->requested_url);
    
    priv->requested_url = g_strdup (url);
    
    if (GTK_WIDGET_REALIZED (area))
	priv->view->LoadURI (priv->requested_url);
    
    g_object_notify (G_OBJECT (area), "requested-url");
}

void
moz_area_display_data (MozArea     *area,
		       const gchar *base_url,
		       const gchar *content_type,
		       const gchar *data,
		       gsize        len)
{
    MozAreaPriv *priv; 
    
    g_return_if_fail (MOZ_IS_AREA (area));
    g_return_if_fail (base_url != NULL);
    g_return_if_fail (content_type != NULL);
    g_return_if_fail (data != NULL);
    g_return_if_fail (len > 0);

    priv = GET_PRIV (area);

    if (GTK_WIDGET_REALIZED (area))
	priv->view->LoadData(base_url, content_type, (PRUint8 *)data, (PRUint32)len);
}

gchar *
moz_area_get_title (MozArea *area)
{
    MozAreaPriv *priv; 
    
    g_return_val_if_fail (MOZ_IS_AREA (area), NULL);

    priv = GET_PRIV (area);

    return g_strdup (priv->title);
}

gboolean
moz_area_set_user_agent (MozArea     *area,
			 const gchar *user_agent)
{
    nsresult rv;
    MozAreaPriv *priv;

    g_return_val_if_fail (MOZ_IS_AREA (area), FALSE);
    g_return_val_if_fail (user_agent != NULL, FALSE);

    priv = GET_PRIV (area);
    rv = priv->app->SetCharPref("general.useragent.override", user_agent);
    
    return TRUE; //!NS_FAILED (rv);
}
