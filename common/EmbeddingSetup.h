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
 * Mozilla Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2007
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Pelle Johnsen <pjohnsen@mozilla.com>
 *   Dave Camp <dcamp@mozilla.com>
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

#ifndef MOZEMBED_EMBEDDINGSETUP_H
#define MOZEMBED_EMBEDDINGSETUP_H

#include "nsIDirectoryService.h"
#include "prtypes.h"

typedef PRUint32 nsresult;

/**
 * Initializes embedding, i.e. loads needed libs do other needed setup.
 *
 * You can call this as many times you like,
 * it will only do the initialization once,
 * but be sure to call TermEmbedding a matching number of times.
 *
 * @param aProfilePath Optional argument to set the path where
 *  profile data is stored. The directory will be created if it
 *  doesn't exist.
 * @param aEmbedPath Optional path to the actual xulrunner code,
 *  to use a specific version rather than any registered version.
 */
nsresult InitEmbedding(const char* aProfilePath = 0,
                       const char* aEmbedPath = 0);

/**
 * Terminates embedding, i.e. does teardown and unloads needed libs.
 *
 * Make sure to match every call of InitEmbedding with a call
 * to TermEmbedding. Only the last call to TermEmbedding will do
 * the actual termination.
 */
nsresult TermEmbedding();

#endif /* Header guard */
