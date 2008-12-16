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
 *   Tobias Hunger <tobias.hunger@gmail.com>
 *   Steffen Imhof <steffen.imhof@googlemail.com>
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
#include "nsIWindowCreator2.h"
#include "nsIWindowWatcher.h"

// Non-Frozen
#include "nsIBaseWindow.h"
#include "nsIDocShellTreeItem.h"
#include "nsIScriptContext.h"
#include "nsIScriptGlobalObject.h"
#include "nsIScriptObjectPrincipal.h"

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

nsresult MozApp::SetCharPref(const char *aName, const char *aValue)
{
    nsresult rv;

    nsCOMPtr<nsIPref> pref(do_GetService(NS_PREF_CONTRACTID, &rv));
    NS_ENSURE_SUCCESS(rv, rv);

    rv = pref->SetCharPref(aName, aValue);

    return rv;
}

nsresult MozApp::SetBoolPref(const char *aName, PRBool aValue)
{
    nsresult rv;

    nsCOMPtr<nsIPref> pref (do_GetService(NS_PREF_CONTRACTID, &rv));
    NS_ENSURE_SUCCESS(rv, rv);

    rv = pref->SetBoolPref(aName, aValue);

    return rv;
}

nsresult MozApp::SetIntPref(const char *aName, int aValue)
{
    nsresult rv;

    nsCOMPtr<nsIPref> pref(do_GetService(NS_PREF_CONTRACTID, &rv));
    NS_ENSURE_SUCCESS(rv, rv);

    rv = pref->SetIntPref(aName, aValue);

    return rv;
}

nsresult MozApp::GetCharPref(const char *aName, char **aValue)
{
    nsresult rv;

    nsCOMPtr<nsIPref> pref(do_GetService(NS_PREF_CONTRACTID, &rv));
    NS_ENSURE_SUCCESS(rv, rv);

    rv = pref->GetCharPref(aName, aValue);

    return rv;
}

nsresult MozApp::GetBoolPref(const char *aName, PRBool *aValue)
{
    nsresult rv;

    nsCOMPtr<nsIPref> pref(do_GetService(NS_PREF_CONTRACTID, &rv));
    NS_ENSURE_SUCCESS(rv, rv);

    rv = pref->GetBoolPref(aName, aValue);

    return rv;
}

nsresult MozApp::GetIntPref(const char *aName, int *aValue)
{
    nsresult rv;

    nsCOMPtr<nsIPref> pref(do_GetService(NS_PREF_CONTRACTID, &rv));
    NS_ENSURE_SUCCESS(rv, rv);

    rv = pref->GetIntPref(aName, aValue);

    return rv;
}

class MozView::Private{
public:
    Private() :
        mListener(0),
        mParentWindow(0),
        mParentView(0)
    { }

    MozViewListener* mListener;
    void* mParentWindow;
    MozView* mParentView;

    nsCOMPtr<nsIWebBrowser> mWebBrowser;
    nsCOMPtr<nsIWebNavigation> mWebNavigation;
    nsCOMPtr<nsIWebBrowserChrome> mChrome;
    nsCOMPtr<nsIURIContentListener> mContentListener;
};

class WindowCreator : public nsIWindowCreator2
{
public:
    WindowCreator() {};
    virtual ~WindowCreator() {};

    NS_DECL_ISUPPORTS
    NS_DECL_NSIWINDOWCREATOR
    NS_DECL_NSIWINDOWCREATOR2
};

NS_IMPL_ISUPPORTS2(WindowCreator, nsIWindowCreator, nsIWindowCreator2)

NS_IMETHODIMP
WindowCreator::CreateChromeWindow(nsIWebBrowserChrome *aParent,
                                  PRUint32 aChromeFlags,
                                  nsIWebBrowserChrome **_retval)
{
    NS_ENSURE_ARG_POINTER(_retval);

    // get the listener from parent
    // we assume parent is a WebBrowserChrome
    WebBrowserChrome* chrome = static_cast<WebBrowserChrome*>(aParent);

    MozViewListener* pListener = chrome->GetMozView()->GetListener();
    if (!pListener)
      return NS_ERROR_FAILURE;

    MozView* mozView = pListener->OpenWindow(aChromeFlags);
    if (!mozView)
        return NS_ERROR_FAILURE;

    *_retval = mozView->mPrivate->mChrome;
    NS_IF_ADDREF(*_retval);
    return NS_OK;
}

NS_IMETHODIMP
WindowCreator::CreateChromeWindow2(nsIWebBrowserChrome *aParent,
                                   PRUint32 aChromeFlags,
                                   PRUint32 /*aContextFlags*/,
                                   nsIURI * /*aUri*/, PRBool * /*aCancel*/,
                                   nsIWebBrowserChrome **_retval)
{
    return CreateChromeWindow(aParent, aChromeFlags, _retval);
}

MozView::MozView()
{
    InitEmbedding();
    mPrivate = new Private();

    // TODO: should probably deal with WindowCreator in InitEmbedding
    //       or maybe in mozapp
    if (sWindowCreator)
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
    baseWindow = do_QueryInterface(mPrivate->mWebBrowser);
    if (baseWindow)
        baseWindow->Destroy();
    if (mPrivate->mChrome) {
        mPrivate->mChrome->SetWebBrowser(0);
    }

    baseWindow = 0;

    mPrivate->mWebBrowser = 0;
    mPrivate->mChrome = 0;
    mPrivate->mContentListener = 0;
    delete mPrivate;
    TermEmbedding();
}

nsresult MozView::CreateBrowser(void* aParentWindow,
                                PRInt32 aX, PRInt32 aY,
                                PRInt32 aWidth, PRInt32 aHeight,
                                PRUint32 aChromeFlags)
{
    mPrivate->mParentWindow = aParentWindow;

    nsresult rv;

    nsCOMPtr<nsIBaseWindow> baseWindow;
    mPrivate->mWebBrowser = do_CreateInstance(NS_WEBBROWSER_CONTRACTID, &rv);
    if (NS_FAILED(rv)) {
        printf("do_CreateInstance webBrowser\n");
    }
    baseWindow = do_QueryInterface(mPrivate->mWebBrowser);
    rv = baseWindow->InitWindow(mPrivate->mParentWindow, 0, aX, aY, aWidth, aHeight);
    if (NS_FAILED(rv)) {
        printf("InitWindow failed.\n");
    }

    nsIWebBrowserChrome **aNewWindow = getter_AddRefs(mPrivate->mChrome);
    CallQueryInterface(static_cast<nsIWebBrowserChrome*>(new WebBrowserChrome(this)), aNewWindow);

    mPrivate->mWebBrowser->SetContainerWindow(mPrivate->mChrome);
    mPrivate->mChrome->SetWebBrowser(mPrivate->mWebBrowser);
    mPrivate->mChrome->SetChromeFlags(aChromeFlags);

    if (aChromeFlags & (nsIWebBrowserChrome::CHROME_OPENAS_CHROME |
        nsIWebBrowserChrome::CHROME_OPENAS_DIALOG) ) {
        nsCOMPtr<nsIDocShellTreeItem> docShellItem(do_QueryInterface(baseWindow));
        docShellItem->SetItemType(nsIDocShellTreeItem::typeChromeWrapper);
    }

    rv = baseWindow->Create();
    if (NS_FAILED(rv)) {
        printf("Creation of basewindow failed.\n");
    }
    rv = baseWindow->SetVisibility(PR_TRUE);
    if (NS_FAILED(rv)) {
        printf("SetVisibility failed.\n");
    }

    // register the progress listener
    nsCOMPtr<nsIWebProgressListener> listener = do_QueryInterface(mPrivate->mChrome);
    nsCOMPtr<nsIWeakReference> thisListener(do_GetWeakReference(listener));
    mPrivate->mWebBrowser->AddWebBrowserListener(thisListener, NS_GET_IID(nsIWebProgressListener));

    mPrivate->mWebNavigation = do_QueryInterface(mPrivate->mWebBrowser);

    // register the content listener
    mPrivate->mContentListener = new ContentListener(this, mPrivate->mWebNavigation);
    mPrivate->mWebBrowser->SetParentURIContentListener(mPrivate->mContentListener);

    SetFocus(true);

    return 0;
}

nsresult MozView::SetPositionAndSize(PRInt32 aX, PRInt32 aY,
                                     PRInt32 aWidth, PRInt32 aHeight)
{
    nsresult rv;
    nsCOMPtr<nsIBaseWindow> baseWindow;
    baseWindow = do_QueryInterface(mPrivate->mWebBrowser);
    rv = baseWindow->SetPositionAndSize(aX, aY, aWidth, aHeight, PR_TRUE);
    if (NS_FAILED(rv))
        return 1;

    return 0;
}

nsresult MozView::LoadURI(const char *aUri)
{
    nsresult rv =
      mPrivate->mWebNavigation->LoadURI(NS_ConvertUTF8toUTF16(aUri).get(),
        nsIWebNavigation::LOAD_FLAGS_NONE, 0, 0, 0);
    return rv;
}

nsresult MozView::LoadData(const char *aBaseUrl,
                           const char *aContentType,
                           const PRUint8 *aData,
                           PRUint32 aLen)
{
    nsresult rv;

    nsCOMPtr<nsIWebBrowserStream> wbStream = do_QueryInterface(mPrivate->mWebBrowser);
    if (!wbStream)
      return NS_ERROR_FAILURE;

    nsCOMPtr<nsIURI> uri;
    rv = NS_NewURI(getter_AddRefs(uri), aBaseUrl);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = wbStream->OpenStream(uri, nsDependentCString(aContentType));
    NS_ENSURE_SUCCESS(rv, rv);

    rv = wbStream->AppendToStream(aData, aLen);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = wbStream->CloseStream();
    NS_ENSURE_SUCCESS(rv, rv);

    return NS_OK;
}

nsresult MozView::Stop()
{
    nsresult rv = mPrivate->mWebNavigation->Stop(nsIWebNavigation::STOP_ALL);
    return rv;
}

nsresult MozView::Reload()
{
    nsresult rv = mPrivate->mWebNavigation->Reload(nsIWebNavigation::LOAD_FLAGS_NONE);
    return rv;
}

nsresult MozView::GoBack()
{
    nsresult rv = mPrivate->mWebNavigation->GoBack();
    return rv;
}

nsresult MozView::GoForward()
{
    nsresult rv = mPrivate->mWebNavigation->GoForward();
    return rv;
}

PRBool MozView::CanGoBack()
{
    PRBool allowBack;
    mPrivate->mWebNavigation->GetCanGoBack(&allowBack);
    return allowBack;
}

PRBool MozView::CanGoForward()
{
    PRBool allowForward;
    mPrivate->mWebNavigation->GetCanGoForward(&allowForward);
    return allowForward;
}

nsresult MozView::SetFocus(PRBool aFocus)
{
    nsCOMPtr<nsIWebBrowserFocus> browserFocus;
    browserFocus = do_QueryInterface(mPrivate->mWebBrowser);
    if (aFocus)
      browserFocus->Activate();
    else
      browserFocus->Deactivate();
    return NS_OK;
}

void MozView::Show()
{
    nsCOMPtr<nsIBaseWindow> baseWindow;
    baseWindow = do_QueryInterface(mPrivate->mWebBrowser);
    baseWindow->SetVisibility(PR_TRUE);
}

void MozView::Hide()
{
    nsCOMPtr<nsIBaseWindow> baseWindow;
    baseWindow = do_QueryInterface(mPrivate->mWebBrowser);
    baseWindow->SetVisibility(PR_FALSE);
}

void MozView::SetListener(MozViewListener *aNewListener)
{
    // XXX shouldn't you tell the old listener it's done?
    mPrivate->mListener = aNewListener;
    if (aNewListener)
        mPrivate->mListener->SetMozView(this);
}

MozViewListener* MozView::GetListener()
{
    return mPrivate->mListener;
}

void* MozView::GetParentWindow()
{
    return mPrivate->mParentWindow;
}

void* MozView::GetNativeWindow()
{
    nsCOMPtr<nsIBaseWindow> baseWindow = do_QueryInterface(mPrivate->mWebBrowser);
    nsCOMPtr<nsIWidget> mozWidget;

    nsresult rv = baseWindow->GetMainWidget(getter_AddRefs(mozWidget));
    if (NS_FAILED(rv))
        return 0;

    return mozWidget->GetNativeData(NS_NATIVE_WINDOW);
}

void MozView::SetParentView(MozView* aParent)
{
    mPrivate->mParentView = aParent;
}

MozView* MozView::GetParentView()
{
    return mPrivate->mParentView;
}

nsresult MozView::GetInterfaceRequestor(nsIInterfaceRequestor** aRequestor)
{
    NS_ENSURE_ARG_POINTER(aRequestor);
    return CallQueryInterface(mPrivate->mWebBrowser, aRequestor);
}

// XXX using c++ new as an allocator is generally BAD
char* MozView::EvaluateJavaScript(const char* aScript)
{
    nsCOMPtr<nsIScriptGlobalObject> sgo =
      do_GetInterface(mPrivate->mWebBrowser);
    nsCOMPtr<nsIScriptContext> ctx = sgo->GetContext();
    nsString retval;
    nsCOMPtr<nsIScriptObjectPrincipal> sgoPrincipal = do_QueryInterface(sgo);
    ctx->EvaluateString(NS_ConvertUTF8toUTF16(aScript), sgo->GetGlobalJSObject(),
      sgoPrincipal->GetPrincipal(),
      "mozembed", 0, nsnull, &retval, nsnull);

    NS_ConvertUTF16toUTF8 retvalUtf8(retval);
    char* temp = new char[retvalUtf8.Length() + 1];
    strncpy(temp, retvalUtf8.get(), retvalUtf8.Length());
    temp[retvalUtf8.Length()] = 0;

    return temp;
}

// ---- MozViewListener ---
MozViewListener::MozViewListener() : mMozView(0)
{
}

MozViewListener::~MozViewListener()
{
}

void MozViewListener::SetTitle(const char * /*aNewTitle*/)
{
}

void MozViewListener::StatusChanged(const char * /*aNewStatus*/, PRUint32 /*aStatusType*/)
{
}

void MozViewListener::LocationChanged(const char * /*aNewLocation*/)
{
}

PRBool MozViewListener::OpenURI(const char* /*aNewLocation*/)
{
    return PR_FALSE;
}

void MozViewListener::DocumentLoaded()
{
}

void MozViewListener::SetMozView(MozView *aMozView)
{
    mMozView = aMozView;
}

MozView* MozViewListener::OpenWindow(PRUint32 /*aFlags*/)
{
    return 0;
}

void MozViewListener::SizeTo(PRUint32 /*aWidth*/, PRUint32 /*aHeight*/)
{
}

void MozViewListener::SetVisibility(PRBool /*aVisible*/)
{
}

void MozViewListener::StartModal()
{
}

void MozViewListener::ExitModal(nsresult /*aResult*/)
{
}

void MozViewListener::OnFocusChanged(PRBool /*aForward*/)
{
}
