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
 *        Tristan Van Berkom <tristan.van.berkom@gmail.com>
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

#include "nsError.h"
#include "EmbeddingSetup.h"
#include "moz-web-view-common.h"
#include "moz-web-view-marshal.h"

static MozApp *app = NULL;
static gint init_count = 0;

gboolean
moz_web_view_init_embedding(const gchar *profile_path)
{
    nsresult rv;

    rv = InitEmbedding(profile_path);
    if (NS_FAILED(rv))
        return FALSE;

    if (init_count++ == 0)
    app = new MozApp(profile_path);

    return TRUE;
}

gboolean
moz_web_view_term_embedding(void)
{
    nsresult rv;

    rv = TermEmbedding();
    if (NS_FAILED(rv))
        return FALSE;

    if (--init_count == 0) {
        delete app;
        app = NULL;
    }

    return TRUE;
}

gboolean
moz_web_view_set_char_pref(const char *name, 
                           const char *value)
{
    g_return_val_if_fail(init_count > 0, FALSE);

    return NS_SUCCEEDED(app->SetCharPref(name, value));
}


gboolean
moz_web_view_set_bool_pref(const char *name, 
                           gboolean value)
{
    g_return_val_if_fail(init_count > 0, FALSE);

    return NS_SUCCEEDED(app->SetBoolPref(name, value));
}


gboolean
moz_web_view_set_int_pref(const char *name, 
                          gint value)
{
    g_return_val_if_fail(init_count > 0, FALSE);

    return NS_SUCCEEDED(app->SetIntPref(name, value));
}

gboolean
moz_web_view_get_char_pref(const char *name, 
                           char **value)
{
    g_return_val_if_fail(init_count > 0, FALSE);

    return NS_SUCCEEDED(app->GetCharPref(name, value));
}

gboolean
moz_web_view_get_bool_pref(const char *name, 
                           gboolean *value)
{
    g_return_val_if_fail(init_count > 0, FALSE);

    return NS_SUCCEEDED(app->GetBoolPref(name, value));
}

gboolean
moz_web_view_get_int_pref(const char *name, 
                          gint *value)
{
    g_return_val_if_fail(init_count > 0, FALSE);

    return NS_SUCCEEDED(app->GetIntPref(name, value));
}

/**********************************************************
 *                MozViewableIface                        *
 **********************************************************/
static void
moz_viewable_class_init(gpointer g_iface)
{

    g_signal_new("title-changed",
        MOZ_TYPE_VIEWABLE,
        G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET(MozViewableIface, title_changed),
        NULL, NULL,
        g_cclosure_marshal_VOID__STRING,
        G_TYPE_NONE, 1, G_TYPE_STRING);
    
    g_signal_new("status-changed",
        MOZ_TYPE_VIEWABLE,
        G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET(MozViewableIface, status_changed),
        NULL, NULL,
        g_cclosure_marshal_VOID__STRING,
        G_TYPE_NONE, 1, G_TYPE_STRING);
    
    g_signal_new("location-changed",
        MOZ_TYPE_VIEWABLE,
        G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET(MozViewableIface, location_changed),
        NULL, NULL,
        g_cclosure_marshal_VOID__STRING,
        G_TYPE_NONE, 1, G_TYPE_STRING);
    
    g_signal_new("uri-requested",
        MOZ_TYPE_VIEWABLE,
        G_SIGNAL_RUN_LAST,
        G_STRUCT_OFFSET(MozViewableIface, uri_requested),
        g_signal_accumulator_true_handled, NULL,
        g_cclosure_user_marshal_BOOLEAN__STRING,
        G_TYPE_BOOLEAN, 1, G_TYPE_STRING);
    
    g_signal_new("document-loaded",
        MOZ_TYPE_VIEWABLE,
        G_SIGNAL_RUN_LAST,
        G_STRUCT_OFFSET(MozViewableIface, document_loaded),
        NULL, NULL,
        g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0);
    
    g_object_interface_install_property(g_iface,
        g_param_spec_string("requested-uri",
                            "Requested URI",
                            "The requested uri",
                            NULL,
                            (GParamFlags)G_PARAM_READABLE));

    g_object_interface_install_property(g_iface,
         g_param_spec_string("title",
                             "Page title",
                             "The current webpage title",
                             NULL,
                             (GParamFlags)G_PARAM_READABLE));

    g_object_interface_install_property(g_iface,
         g_param_spec_string("status",
                             "Status text",
                             "User feedback text",
                             NULL,
                             (GParamFlags)G_PARAM_READABLE));

    g_object_interface_install_property(g_iface,
         g_param_spec_string("location",
                             "Location",
                             "Current browser location URI",
                             NULL,
                             (GParamFlags)G_PARAM_READABLE));
}

GType
moz_viewable_get_type(void)
{
    static GType viewable_type = 0;

    if (!viewable_type)
    {
        viewable_type =
          g_type_register_static_simple(G_TYPE_INTERFACE, "MozViewable",
                                        sizeof(MozViewableIface),
                                        (GClassInitFunc) moz_viewable_class_init,
                                        0, NULL, (GTypeFlags)0);

        g_type_interface_add_prerequisite(viewable_type, G_TYPE_OBJECT);
    }

    return viewable_type;
}

static gchar *
moz_web_view_get_prop(MozWebView *view, const gchar *prop)
{
    gchar *ret = NULL;
    g_return_val_if_fail(MOZ_IS_WEB_VIEW(view), NULL);

    g_object_get(G_OBJECT(view), prop, &ret, NULL);

    return ret;
}

/*******************************************************
 *                MozWebView accessors                 *
 *******************************************************/
gchar *
moz_web_view_get_requested_uri(MozWebView *view)
{
    return moz_web_view_get_prop (view, "requested-uri");
}

gchar *
moz_web_view_get_title(MozWebView *view)
{
    return moz_web_view_get_prop (view, "title");
}

gchar *
moz_web_view_get_status(MozWebView *view)
{
    return moz_web_view_get_prop (view, "status");
}

gchar *
moz_web_view_get_location(MozWebView *view)
{
    return moz_web_view_get_prop (view, "location");
}
