#ifndef MOZ_WEB_VIEW_GTK_H
#define MOZ_WEB_VIEW_GTK_H

#include <gtk/gtkbin.h>

G_BEGIN_DECLS

#define MOZ_TYPE_WEB_VIEW             (moz_web_view_get_type())
#define MOZ_WEB_VIEW(obj)             G_TYPE_CHECK_INSTANCE_CAST((obj), MOZ_TYPE_WEB_VIEW, MozWebView)
#define MOZ_WEB_VIEW_CLASS(klass)     G_TYPE_CHECK_CLASS_CAST((klass), MOZ_TYPE_WEB_VIEW, MozWebViewClass)
#define MOZ_IS_WEB_VIEW(obj)          G_TYPE_CHECK_INSTANCE_TYPE((obj), MOZ_TYPE_WEB_VIEW)
#define MOZ_IS_WEB_VIEW_CLASS(klass)  G_TYPE_CHECK_CLASS_TYPE((klass), MOZ_TYPE_WEB_VIEW)

typedef struct _MozWebView      MozWebView;
typedef struct _MozWebViewClass MozWebViewClass;
typedef struct _MozWebViewPriv  MozWebViewPriv;

struct _MozWebView {
  GtkBin bin;
  MozWebViewPriv *priv;
};

struct _MozWebViewClass {
  GtkBinClass parent_class;

  /* Signals */
  void     (*title_changed) (MozWebView *view, const char *title);
  void     (*status_changed) (MozWebView *view, const char *status, guint32 flags);
  void     (*location_changed) (const char *uri);
  gboolean (*uri_requested)   (MozWebView *view, const gchar *uri);
  void     (*document_loaded) (MozWebView *view);
};

GType      moz_web_view_get_type  (void);
GtkWidget *moz_web_view_new       (void);

void       moz_web_view_load_uri       (MozWebView  *view, const char *uri);
void       moz_web_view_load_data      (MozWebView  *view,
					const gchar *base_uri,
					const gchar *content_type,
					const gchar *data,
					gsize        len);
gchar *    moz_web_view_get_title      (MozWebView  *view);
gboolean   moz_web_view_set_user_agent (MozWebView  *view,
					const gchar *user_agent);

G_END_DECLS

#endif
