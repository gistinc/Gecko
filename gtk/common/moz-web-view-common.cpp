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

#include "nsError.h"

#include "embed.h"
#include "EmbeddingSetup.h"
#include "moz-web-view.h"

static MozApp *app = NULL;
static gint    init_count = 0;

gboolean
moz_web_view_init_embedding (const gchar *profile_path)
{
    nsresult rv;

    rv = InitEmbedding (profile_path);
    if (NS_FAILED (rv))
	return FALSE;

    if (init_count++ == 0)
	app = new MozApp(profile_path);

    return TRUE;
}

gboolean
moz_web_view_term_embedding (void)
{
    nsresult rv;

    rv = TermEmbedding ();
    if (NS_FAILED (rv))
	return FALSE;

    if (--init_count == 0) {
	delete app;
	app = NULL;
    }

    return TRUE;
}

gboolean
moz_web_view_set_char_pref  (const char  *name, 
			     const char  *value)
{
    nsresult rv;

    g_return_val_if_fail (init_count > 0, FALSE);

    rv = app->SetCharPref (name, value);
    
    return !NS_FAILED(rv);
}


gboolean
moz_web_view_set_bool_pref  (const char  *name, 
			     gboolean     value)
{
    nsresult rv;

    g_return_val_if_fail (init_count > 0, FALSE);

    rv = app->SetBoolPref (name, value);
    
    return !NS_FAILED(rv);
}


gboolean
moz_web_view_set_int_pref   (const char  *name, 
			     gint         value)
{
    nsresult rv;

    g_return_val_if_fail (init_count > 0, FALSE);

    rv = app->SetIntPref (name, value);
    
    return !NS_FAILED(rv);
}

gboolean
moz_web_view_get_char_pref  (const char  *name, 
			     char       **value)
{
    nsresult rv;

    g_return_val_if_fail (init_count > 0, FALSE);

    rv = app->GetCharPref (name, value);
    
    return !NS_FAILED(rv);
}

gboolean
moz_web_view_get_bool_pref  (const char  *name, 
			     gboolean    *value)
{
    nsresult rv;

    g_return_val_if_fail (init_count > 0, FALSE);

    rv = app->GetBoolPref (name, value);
    
    return !NS_FAILED(rv);
}

gboolean
moz_web_view_get_int_pref   (const char  *name, 
			     gint        *value)
{
    nsresult rv;

    g_return_val_if_fail (init_count > 0, FALSE);

    rv = app->GetIntPref (name, value);
    
    return !NS_FAILED(rv);
}

