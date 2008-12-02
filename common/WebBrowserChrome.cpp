#include "xpcom-config.h"
#include "WebBrowserChrome.h"
#include "embed.h"
#include "nsIDOMWindow.h"
#include "nsStringAPI.h"
#include "nsIURI.h"
#include <iostream>

using namespace std;

WebBrowserChrome::WebBrowserChrome(MozView* pAMozView)
: mChromeFlags(0), pMozView(pAMozView), mSizeSet(PR_FALSE), mIsModal(PR_FALSE)
{
    /* member initializers and constructor code */
}

WebBrowserChrome::~WebBrowserChrome()
{
    /* destructor code */
}

NS_IMPL_ISUPPORTS6(WebBrowserChrome,
                   nsIWebBrowserChrome,
                   nsIWebBrowserChromeFocus,
                   nsIInterfaceRequestor,
                   nsIEmbeddingSiteWindow,
                   nsIWebProgressListener,
                   nsISupportsWeakReference)

NS_IMETHODIMP WebBrowserChrome::GetInterface(const nsIID &aIID, void** aInstancePtr)
{
    NS_ENSURE_ARG_POINTER(aInstancePtr);

    *aInstancePtr = 0;
    if (aIID.Equals(NS_GET_IID(nsIDOMWindow)))
    {
        if (!mWebBrowser)
            return NS_ERROR_NOT_INITIALIZED;

        return mWebBrowser->GetContentDOMWindow((nsIDOMWindow **) aInstancePtr);
    }
    return QueryInterface(aIID, aInstancePtr);
}

/* void setStatus (in unsigned long statusType, in wstring status); */
NS_IMETHODIMP WebBrowserChrome::SetStatus(PRUint32 statusType, const PRUnichar *status)
{
    MozViewListener* pListener = pMozView->GetListener();
    if (!pListener)
        return NS_ERROR_NOT_IMPLEMENTED;

    pListener->StatusChanged(NS_ConvertUTF16toUTF8(status).get(), statusType);
    return NS_OK;
}

/* attribute nsIWebBrowser webBrowser; */
NS_IMETHODIMP WebBrowserChrome::GetWebBrowser(nsIWebBrowser * *aWebBrowser)
{
    NS_ENSURE_ARG_POINTER(aWebBrowser);
    *aWebBrowser = mWebBrowser;
    NS_IF_ADDREF(*aWebBrowser);
    return NS_OK;
}

NS_IMETHODIMP WebBrowserChrome::SetWebBrowser(nsIWebBrowser * aWebBrowser)
{
    mWebBrowser = aWebBrowser;
    return NS_OK;
}

/* attribute unsigned long chromeFlags; */
NS_IMETHODIMP WebBrowserChrome::GetChromeFlags(PRUint32 *aChromeFlags)
{
    *aChromeFlags = mChromeFlags;
    return NS_OK;
}
NS_IMETHODIMP WebBrowserChrome::SetChromeFlags(PRUint32 aChromeFlags)
{
    mChromeFlags = aChromeFlags;
    return NS_OK;
}

/* void destroyBrowserWindow (); */
NS_IMETHODIMP WebBrowserChrome::DestroyBrowserWindow()
{
    if (mIsModal) {
        ExitModalEventLoop(NS_OK);
    }
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void sizeBrowserTo (in long aCX, in long aCY); */
NS_IMETHODIMP WebBrowserChrome::SizeBrowserTo(PRInt32 aCX, PRInt32 aCY)
{
    MozViewListener* pListener = pMozView->GetListener();
    if (!pListener)
        return NS_ERROR_NOT_IMPLEMENTED;

    pListener->SizeTo(aCX, aCY);
    mSizeSet = PR_TRUE;
    return NS_OK;
}

/* void showAsModal (); */
NS_IMETHODIMP WebBrowserChrome::ShowAsModal()
{
    MozViewListener* pListener = pMozView->GetListener();
    if (!pListener)
        return NS_ERROR_NOT_IMPLEMENTED;

    mIsModal = PR_TRUE;
    pListener->StartModal();
    return NS_OK;
}

/* boolean isWindowModal (); */
NS_IMETHODIMP WebBrowserChrome::IsWindowModal(PRBool *_retval)
{
    NS_ENSURE_ARG_POINTER(_retval);
    *_retval = mIsModal;
    return NS_OK;
}

/* void exitModalEventLoop (in nsresult aStatus); */
NS_IMETHODIMP WebBrowserChrome::ExitModalEventLoop(nsresult aStatus)
{
    MozViewListener* pListener = pMozView->GetListener();
    if (!pListener)
        return NS_ERROR_NOT_IMPLEMENTED;

    pListener->ExitModal(aStatus);
    mIsModal = PR_FALSE;
    return NS_OK;
}


// ----- Progress Listener -----

/* void onStateChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in unsigned long aStateFlags, in nsresult aStatus); */
NS_IMETHODIMP WebBrowserChrome::OnStateChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRUint32 aStateFlags, nsresult aStatus)
{
    MozViewListener* pListener = pMozView->GetListener();
    // XXX no one considered this case
    if (!pListener)
        return NS_OK;

    if ((aStateFlags & STATE_STOP) && (aStateFlags & STATE_IS_DOCUMENT)) {
        // if it was a chrome window and no one has already specified a size,
        // size to content
        if (!mSizeSet &&
            (mChromeFlags & nsIWebBrowserChrome::CHROME_OPENAS_CHROME)) {
            nsCOMPtr<nsIDOMWindow> contentWin;
            mWebBrowser->GetContentDOMWindow(getter_AddRefs(contentWin));
            if (contentWin)
                contentWin->SizeToContent();
            SetVisibility(PR_TRUE);
        }

        pListener->DocumentLoaded();
    }

    return NS_OK;
}

/* void onProgressChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in long aCurSelfProgress, in long aMaxSelfProgress, in long aCurTotalProgress, in long aMaxTotalProgress); */
NS_IMETHODIMP WebBrowserChrome::OnProgressChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRInt32 aCurSelfProgress, PRInt32 aMaxSelfProgress, PRInt32 aCurTotalProgress, PRInt32 aMaxTotalProgress)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onLocationChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in nsIURI aLocation); */
NS_IMETHODIMP WebBrowserChrome::OnLocationChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, nsIURI *aLocation)
{
    NS_ENSURE_ARG_POINTER(aLocation);

    MozViewListener* pListener = pMozView->GetListener();
    if (!pListener)
        return NS_ERROR_NOT_IMPLEMENTED;

    nsCString spec;
    aLocation->GetSpec(spec);
    pListener->LocationChanged(spec.get());
    return NS_OK;
}

/* void onStatusChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in nsresult aStatus, in wstring aMessage); */
NS_IMETHODIMP WebBrowserChrome::OnStatusChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, nsresult aStatus, const PRUnichar *aMessage)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onSecurityChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in unsigned long aState); */
NS_IMETHODIMP WebBrowserChrome::OnSecurityChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRUint32 aState)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

// ----- Embedding Site Window

/* void setDimensions (in unsigned long flags, in long x, in long y, in long cx, in long cy); */
NS_IMETHODIMP WebBrowserChrome::SetDimensions(PRUint32 flags, PRInt32 x, PRInt32 y, PRInt32 cx, PRInt32 cy)
{
    // TODO: currently only does size
    MozViewListener* pListener = pMozView->GetListener();
    if (!pListener)
        return NS_ERROR_NOT_IMPLEMENTED;

    pListener->SizeTo(cx, cy);
    mSizeSet = PR_TRUE;
    return NS_OK;
}

/* void getDimensions (in unsigned long flags, out long x, out long y, out long cx, out long cy); */
NS_IMETHODIMP WebBrowserChrome::GetDimensions(PRUint32 flags, PRInt32 *x, PRInt32 *y, PRInt32 *cx, PRInt32 *cy)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setFocus (); */
NS_IMETHODIMP WebBrowserChrome::SetFocus()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean visibility; */
NS_IMETHODIMP WebBrowserChrome::GetVisibility(PRBool *aVisibility)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP WebBrowserChrome::SetVisibility(PRBool aVisibility)
{
    MozViewListener* pListener = pMozView->GetListener();
    if (!pListener)
        return NS_ERROR_NOT_IMPLEMENTED;

    pListener->SetVisibility(aVisibility);
    return NS_OK;
}

/* attribute wstring title; */
NS_IMETHODIMP WebBrowserChrome::GetTitle(PRUnichar * *aTitle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP WebBrowserChrome::SetTitle(const PRUnichar * aTitle)
{
    MozViewListener* pListener = pMozView->GetListener();
    if (!pListener)
        return NS_ERROR_NOT_IMPLEMENTED;

    pListener->SetTitle(NS_ConvertUTF16toUTF8(aTitle).get());
    return NS_OK;
}

/* [noscript] readonly attribute voidPtr siteWindow; */
NS_IMETHODIMP WebBrowserChrome::GetSiteWindow(void * *aSiteWindow)
{
    NS_ENSURE_ARG_POINTER(aSiteWindow);
    *aSiteWindow = pMozView->GetParentWindow();
    return NS_OK;
}

// ----- WebBrowser Chrome Focus

/* void focusNextElement (); */
NS_IMETHODIMP WebBrowserChrome::FocusNextElement()
{
    MozViewListener* pListener = pMozView->GetListener();
    if (!pListener)
        return NS_ERROR_NOT_IMPLEMENTED;

    pListener->OnFocusChanged(PR_TRUE);
    return NS_OK;
}

/* void focusPrevElement (); */
NS_IMETHODIMP WebBrowserChrome::FocusPrevElement()
{
    MozViewListener* pListener = pMozView->GetListener();
    if (!pListener)
        return NS_ERROR_NOT_IMPLEMENTED;

    pListener->OnFocusChanged(PR_FALSE);
    return NS_OK;
}
