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
#include "nsIWebBrowserStream.h"
#include "nsIURI.h"
#include "nsNetUtil.h" // NS_NewURI()
#include "nsIPref.h"

#include "nsIWebBrowser.h"
#include "nsIWebNavigation.h"
#include "nsEmbedCID.h"

#include "nsIWebBrowserFocus.h"
#include "nsIWidget.h"
#include "nsIWindowCreator.h"
#include "nsIWindowWatcher.h"

// Non-Frozen
#include "nsIBaseWindow.h"

// our stuff
#include "WebBrowserChrome.h"
#include "ContentListener.h"

// globals
static nsCOMPtr<WindowCreator> sWindowCreator;


MozApp::MozApp(const char* aProfilePath)
{
  InitEmbedding(aProfilePath);
}

MozApp::~MozApp()
{
  TermEmbedding();
}

nsresult MozApp::SetCharPref(const char *name, const char *value)
{
    nsresult rv;
	
    nsCOMPtr<nsIPref> pref (do_GetService (NS_PREF_CONTRACTID, &rv));
    if (NS_FAILED (rv)) return rv;
    
    rv = pref->SetCharPref (name, value);
    
    return rv;
}

nsresult MozApp::SetBoolPref(const char *name, PRBool value)
{
    nsresult rv;
	
    nsCOMPtr<nsIPref> pref (do_GetService (NS_PREF_CONTRACTID, &rv));
    if (NS_FAILED (rv)) return rv;
    
    rv = pref->SetBoolPref (name, value);
    
    return rv;
}

nsresult MozApp::SetIntPref(const char *name, int value)
{
    nsresult rv;
	
    nsCOMPtr<nsIPref> pref (do_GetService (NS_PREF_CONTRACTID, &rv));
    if (NS_FAILED (rv)) return rv;
    
    rv = pref->SetIntPref (name, value);
    
    return rv;
}

nsresult MozApp::GetCharPref(const char *name, char **value)
{
    nsresult rv;
	
    nsCOMPtr<nsIPref> pref (do_GetService (NS_PREF_CONTRACTID, &rv));
    if (NS_FAILED (rv)) return rv;
    
    rv = pref->GetCharPref(name, value);
    
    return rv;
}

nsresult MozApp::GetBoolPref(const char *name, PRBool *value)
{
    nsresult rv;
	
    nsCOMPtr<nsIPref> pref (do_GetService (NS_PREF_CONTRACTID, &rv));
    if (NS_FAILED (rv)) return rv;
    
    rv = pref->GetBoolPref (name, value);
    
    return rv;
}

nsresult MozApp::GetIntPref(const char *name, int *value)
{
    nsresult rv;
	
    nsCOMPtr<nsIPref> pref (do_GetService (NS_PREF_CONTRACTID, &rv));
    if (NS_FAILED (rv)) return rv;
    
    rv = pref->GetIntPref (name, value);
    
    return rv;
}

class MozView::Private{
public:
  Private() : parentWindow(NULL), pListener(NULL) {}

  MozViewListener* pListener;
  void* parentWindow;

  nsCOMPtr<nsIWebBrowser> webBrowser;
  nsCOMPtr<nsIWebNavigation> webNavigation;
  nsCOMPtr<nsIWebBrowserChrome> chrome;
  nsCOMPtr<nsIURIContentListener> contentListener;
};

class WindowCreator :
  public nsIWindowCreator
{
public:
  WindowCreator() {};
  virtual ~WindowCreator() {};

  NS_DECL_ISUPPORTS
  NS_DECL_NSIWINDOWCREATOR
};

NS_IMPL_ISUPPORTS1(WindowCreator, nsIWindowCreator)

NS_IMETHODIMP
WindowCreator::CreateChromeWindow(nsIWebBrowserChrome *parent,
  PRUint32 chromeFlags,
  nsIWebBrowserChrome **_retval)
{
  NS_ENSURE_ARG_POINTER(_retval);

  // get the listener from parent
  // we assume parent is a WebBrowserChrome
  WebBrowserChrome* chrome = static_cast<WebBrowserChrome*>(parent);

  MozViewListener* pListener = chrome->GetMozView()->GetListener();
  if(!pListener)
    return NS_ERROR_FAILURE;

  MozView* mozView = pListener->OpenWindow(chromeFlags);
  if(!mozView)
    return NS_ERROR_FAILURE;

  *_retval = mozView->mPrivate->chrome;
  NS_IF_ADDREF(*_retval);
  return NS_OK;
}

MozView::MozView()
{
  InitEmbedding();
  mPrivate = new Private();

  // TODO: should probably deal with WindowCreator in InitEmbedding
  //       or maybe in mozapp
  if(sWindowCreator)
    return;

  // create an nsWindowCreator and give it to the WindowWatcher service
  sWindowCreator = new WindowCreator();
  if (!sWindowCreator)
      return;// NS_ERROR_OUT_OF_MEMORY;

  nsCOMPtr<nsIWindowWatcher> wwatch(do_GetService(NS_WINDOWWATCHER_CONTRACTID));
  if (!wwatch)
      return;// NS_ERROR_UNEXPECTED;

  wwatch->SetWindowCreator(sWindowCreator);

}

MozView::~MozView()
{
  // release browser and chrome
  nsCOMPtr<nsIBaseWindow> baseWindow;
  baseWindow = do_QueryInterface(mPrivate->webBrowser);
  if(baseWindow)
    baseWindow->Destroy();
  if(mPrivate->chrome) {
    mPrivate->chrome->SetWebBrowser(NULL);
  }

  baseWindow = NULL;

  mPrivate->webBrowser = NULL;
  mPrivate->chrome = NULL;
  mPrivate->contentListener = NULL;
  delete mPrivate;
  TermEmbedding();
}

nsresult MozView::CreateBrowser(void* aParentWindow, PRInt32 x, PRInt32 y, PRInt32 width, PRInt32 height)
{
  mPrivate->parentWindow = aParentWindow;

  nsresult rv;

  nsCOMPtr<nsIBaseWindow> baseWindow;
  mPrivate->webBrowser = do_CreateInstance(NS_WEBBROWSER_CONTRACTID, &rv);
  if (NS_FAILED(rv)) {
    printf("do_CreateInstance webBrowser\n");
  }
  baseWindow = do_QueryInterface(mPrivate->webBrowser);
  rv = baseWindow->InitWindow(mPrivate->parentWindow, 0, x, y, width, height);
  if (NS_FAILED(rv)) {
    printf("InitWindow\n");
  }

  nsIWebBrowserChrome **aNewWindow = getter_AddRefs(mPrivate->chrome);
  CallQueryInterface(static_cast<nsIWebBrowserChrome*>(new WebBrowserChrome(this)), aNewWindow);
  mPrivate->webBrowser->SetContainerWindow(mPrivate->chrome);
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

  // register the content listener
  mPrivate->contentListener = new ContentListener(this, mPrivate->webNavigation);
  mPrivate->webBrowser->SetParentURIContentListener(mPrivate->contentListener); 

  SetFocus(true);

  return 0;
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
  rv = mPrivate->webNavigation->LoadURI(NS_ConvertUTF8toUTF16(uri).get(),
    nsIWebNavigation::LOAD_FLAGS_NONE, 0, 0, 0);
  return rv;
}

nsresult MozView::LoadData(const char    *base_url,
			   const char    *content_type,
			   const PRUint8 *data,
			   PRUint32       len)
{
  nsresult rv;

  nsCOMPtr<nsIWebBrowserStream> wbStream = do_QueryInterface(mPrivate->webBrowser);
  if (!wbStream) 
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsIURI> uri;
  rv = NS_NewURI(getter_AddRefs(uri), base_url);
  if (NS_FAILED(rv)) 
    return rv;

  rv = wbStream->OpenStream(uri, nsDependentCString(content_type));
  if (NS_FAILED(rv)) 
    return rv;

  rv = wbStream->AppendToStream(data, len);
  if (NS_FAILED(rv)) 
    return rv;

  rv = wbStream->CloseStream();
  if (NS_FAILED(rv)) 
    return rv;

  return NS_OK;
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

void MozView::Show()
{
  nsCOMPtr<nsIBaseWindow> baseWindow;
  baseWindow = do_QueryInterface(mPrivate->webBrowser);
  baseWindow->SetVisibility(PR_TRUE);
}

void MozView::Hide()
{
  nsCOMPtr<nsIBaseWindow> baseWindow;
  baseWindow = do_QueryInterface(mPrivate->webBrowser);
  baseWindow->SetVisibility(PR_FALSE);
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

void* MozView::GetParentWindow()
{
  return mPrivate->parentWindow;
}

void* MozView::GetNativeWindow()
{
  nsCOMPtr<nsIBaseWindow> baseWindow = do_QueryInterface(mPrivate->webBrowser);
  nsCOMPtr<nsIWidget> mozWidget;

  if (NS_SUCCEEDED(baseWindow->GetMainWidget(getter_AddRefs(mozWidget)))) {
    return mozWidget->GetNativeData(NS_NATIVE_WINDOW);
  } else {
    return NULL;
  }
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

void MozViewListener::StatusChanged(const char* newStatus, PRUint32 statusType)
{
}

void MozViewListener::LocationChanged(const char* newLocation)
{
}

PRBool MozViewListener::OpenURI(const char* newLocation)
{
  return false;
}

void MozViewListener::DocumentLoaded()
{
}

void MozViewListener::SetMozView(MozView *pAMozView)
{
  pMozView = pAMozView;
}

MozView* MozViewListener::OpenWindow(PRUint32 flags)
{
  return 0;
}
