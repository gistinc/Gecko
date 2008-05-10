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
 * Portions created by the Initial Developer are Copyright (C) 20007
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

#include "xpcom-config.h"
#include "mozilla-config.h"

#include "embed.h"
#include "EmbeddingSetup.h"

// CRT headers
#include <iostream>
#include <string>
using namespace std;

// Mozilla Frozen APIs
#include "nsXULAppAPI.h"
#include "nsXPCOMGlue.h"

#include "nsCOMPtr.h"
#include "nsStringAPI.h"
#include "nsComponentManagerUtils.h"
#include "nsIWeakReference.h"
#include "nsIWeakReferenceUtils.h"

#include "nsIWebBrowser.h"
#include "nsIWebNavigation.h"
#include "nsEmbedCID.h"

#include "nsIWebBrowserFocus.h"

// Non-Frozen
#include "nsIBaseWindow.h"

// our stuff
#include "WebBrowserChrome.h"

class MozView::Private{
public:
  Private() : nativeWindow(NULL), pListener(NULL) {}

  MozViewListener* pListener;
  void* nativeWindow;
  nsCOMPtr<nsIWebBrowser> webBrowser;
  nsCOMPtr<nsIWebNavigation> webNavigation;
  nsCOMPtr<nsIWebBrowserChrome> chrome;
};


nsresult MozView::CreateBrowser(void* aNativeWindow, PRInt32 x, PRInt32 y, PRInt32 width, PRInt32 height)
{
  mPrivate->nativeWindow = aNativeWindow;

  nsresult rv;

  nsCOMPtr<nsIBaseWindow> baseWindow;
  mPrivate->webBrowser = do_CreateInstance(NS_WEBBROWSER_CONTRACTID, &rv);
  if (NS_FAILED(rv)) {
    printf("do_CreateInstance webBrowser\n");
  }
  baseWindow = do_QueryInterface(mPrivate->webBrowser);
  rv = baseWindow->InitWindow(mPrivate->nativeWindow, 0, x, y, width, height);
  if (NS_FAILED(rv)) {
    printf("InitWindow\n");
  }
  
  nsIWebBrowserChrome **aNewWindow = getter_AddRefs(mPrivate->chrome);
  CallQueryInterface(static_cast<nsIWebBrowserChrome*>(new WebBrowserChrome(this)), aNewWindow);
  mPrivate->webBrowser->SetContainerWindow(mPrivate->chrome.get());
  mPrivate->chrome->SetWebBrowser(mPrivate->webBrowser);

  rv = baseWindow->Create();
  if (NS_FAILED(rv)) {
    printf("Create\n");
  }
  rv =baseWindow->SetVisibility(PR_TRUE);
  if (NS_FAILED(rv)) {
    printf("SetVisibility\n");
  }

  // register the progress listener
  nsCOMPtr<nsIWebProgressListener> listener = do_QueryInterface(mPrivate->chrome);
  nsCOMPtr<nsIWeakReference> thisListener(do_GetWeakReference(listener));
  mPrivate->webBrowser->AddWebBrowserListener(thisListener, NS_GET_IID(nsIWebProgressListener));


  mPrivate->webNavigation = do_QueryInterface(mPrivate->webBrowser);
  
  SetFocus(true);

  return 0;
}

MozView::MozView()
{
  mPrivate = new Private();
  InitEmbedding();
}

MozView::~MozView()
{
  // release browser and chrome
  nsCOMPtr<nsIBaseWindow> baseWindow;
  baseWindow = do_QueryInterface(mPrivate->webBrowser);
  baseWindow->Destroy();
  mPrivate->chrome->SetWebBrowser(NULL);

  baseWindow = NULL;

  mPrivate->webBrowser = NULL;
  mPrivate->chrome = NULL;
  delete mPrivate;
}

nsresult MozView::SetPositionAndSize(PRInt32 x, PRInt32 y, PRInt32 width, PRInt32 height)
{
  nsresult rv;
  nsCOMPtr<nsIBaseWindow> baseWindow;
  baseWindow = do_QueryInterface(mPrivate->webBrowser);
  rv = baseWindow->SetPositionAndSize(x, y, width, height, PR_TRUE);
  if (NS_FAILED(rv))
    return 1;
  else
    return 0;
}

nsresult MozView::LoadURI(const char* uri)
{
  nsresult rv;
  rv = mPrivate->webNavigation->LoadURI(NS_ConvertASCIItoUTF16(uri).get(),
    nsIWebNavigation::LOAD_FLAGS_NONE, 0, 0, 0);
  return rv;
}

nsresult MozView::SetFocus(PRBool focus)
{
  nsCOMPtr<nsIWebBrowserFocus> browserFocus;
  browserFocus = do_QueryInterface(mPrivate->webBrowser);
  if(focus)
    browserFocus->Activate();
  else
    browserFocus->Deactivate();
  return NS_OK;
}

void MozView::SetListener(MozViewListener *pNewListener)
{
  mPrivate->pListener = pNewListener;
  mPrivate->pListener->SetMozView(this);
}

MozViewListener* MozView::GetListener()
{
  return mPrivate->pListener;
}

void* MozView::GetNativeWindow()
{
  return mPrivate->nativeWindow;
}

// ---- MozViewListener ---
MozViewListener::MozViewListener()
: pMozView(NULL)
{
}

MozViewListener::~MozViewListener()
{
}

void MozViewListener::SetTitle(const char *newTitle)
{
}

void MozViewListener::StatusChanged(const char* newStatus)
{
}

void MozViewListener::LocationChanged(const char* newLocation)
{
}

void MozViewListener::SetMozView(MozView *pAMozView)
{
  pMozView = pAMozView;
}
