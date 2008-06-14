#ifndef __MOZ_AREA_H__
#define __MOZ_AREA_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MOZ_TYPE_AREA            (moz_area_get_type ())
#define MOZ_AREA(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOZ_TYPE_AREA, MozArea))
#define MOZ_AREA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOZ_TYPE_AREA, MozAreaClass))
#define MOZ_IS_AREA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOZ_TYPE_AREA))
#define MOZ_IS_AREA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOZ_TYPE_AREA))
#define MOZ_AREA_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MOZ_TYPE_AREA, MozAreaClass))

typedef struct MozArea      MozArea;
typedef struct MozAreaClass MozAreaClass;

struct MozArea {
    GtkWidget parent;
};

struct MozAreaClass {
    GtkWidgetClass parent_class;

    gboolean (* url_requested)   (MozArea *, const gchar *);
    void     (* document_loaded) (MozArea *);
};

GType       moz_area_get_type  (void) G_GNUC_CONST;
GtkWidget  *moz_area_new       (void);

void        moz_area_display_url           (MozArea     *area,
					    const gchar *url);
void        moz_area_display_data          (MozArea     *area,
					    const gchar *base_url,
					    const gchar *content_type,
					    const gchar *data,
					    gsize        len);
gchar *     moz_area_get_title             (MozArea     *area);
gboolean    moz_area_set_user_agent        (MozArea     *area,
					    const gchar *user_agent);

G_END_DECLS

#endif /* __MOZ_AREA_H__ */

