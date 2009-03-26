#ifndef MOZ_WEB_VIEW_GTK_COMMON_H
#define MOZ_WEB_VIEW_GTK_COMMON_H

#include <moz-web-view.h>
#include <embed.h>

G_BEGIN_DECLS

#define MOZ_TYPE_VIEWABLE            (moz_viewable_get_type ())
#define MOZ_VIEWABLE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOZ_TYPE_VIEWABLE, MozViewable))
#define MOZ_VIEWABLE_CLASS(obj)      (G_TYPE_CHECK_CLASS_CAST ((obj), MOZ_TYPE_VIEWABLE, MozViewableIface))
#define MOZ_IS_VIEWABLE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOZ_TYPE_VIEWABLE))
#define MOZ_VIEWABLE_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), MOZ_TYPE_VIEWABLE, MozViewableIface))

typedef struct _MozViewable      MozViewable; /* Dummy typedef */
typedef struct _MozViewableIface MozViewableIface;

/* NOTE:
 *   This interface is used internally to share the job of 
 * creating and defining signals and properties to be used 
 * in the gtk+ mozilla embedding api. In other words this 
 * header is not to be published or included in applications.
 */
struct _MozViewableIface {
    GTypeInterface base_iface;

    /* Signals */
    void     (*title_changed) (MozViewable *view, const char *title);
    void     (*status_changed) (MozViewable *view, const char *status);
    void     (*location_changed) (MozViewable *view, const char *uri);
    gboolean (*uri_requested)   (MozViewable *view, const char *uri);
    void     (*document_loaded) (MozViewable *view);
};

GType     moz_viewable_get_type               (void) G_GNUC_CONST;

enum {
    TITLE_CHANGED,
    STATUS_CHANGED,
    LOCATION_CHANGED,
    URI_REQUESTED,
    DOCUMENT_LOADED,
    LAST_SIGNAL
};

enum {
    PROP_0,
    PROP_REQUESTED_URI,
    PROP_TITLE,
    PROP_STATUS,
    PROP_LOCATION
};

G_END_DECLS

#endif
