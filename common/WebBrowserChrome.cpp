#include "WebBrowserChrome.h"
#include "embed.h"
#include "nsIDOMWindow.h"
#include "nsStringAPI.h"
#include "nsIURI.h"
#include <iostream>

using namespace std;

WebBrowserChrome::WebBrowserChrome(MozView* pAMozView)
: mChromeFlags(0), pMozView(pAMozView)
{
    /* member initializers and constructor code */
}

WebBrowserChrome::~WebBrowserChrome()
{
    /* destructor code */
}

NS_IMPL_ADDREF(WebBrowserChrome)
NS_IMPL_RELEASE(WebBrowserChrome)

NS_INTERFACE_MAP_BEGIN(WebBrowserChrome)
   NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIWebBrowserChrome)
   NS_INTERFACE_MAP_ENTRY(nsIInterfaceRequestor)
   NS_INTERFACE_MAP_ENTRY(nsIWebBrowserChrome)
   NS_INTERFACE_MAP_ENTRY(nsIEmbeddingSiteWindow)
   NS_INTERFACE_MAP_ENTRY(nsIWebProgressListener)
   NS_INTERFACE_MAP_ENTRY(nsISupportsWeakReference)
NS_INTERFACE_MAP_END

NS_IMETHODIMP WebBrowserChrome::GetInterface(const nsIID &aIID, void** aInstancePtr)
{
    NS_ENSURE_ARG_POINTER(aInstancePtr);

    *aInstancePtr = 0;
    if (aIID.Equals(NS_GET_IID(nsIDOMWindow)))
    {
        if (mWebBrowser)
        {
            return mWebBrowser->GetContentDOMWindow((nsIDOMWindow **) aInstancePtr);
        }
        return NS_ERROR_NOT_INITIALIZED;
    }
    return QueryInterface(aIID, aInstancePtr);
}

/* void setStatus (in unsigned long statusType, in wstring status); */
NS_IMETHODIMP WebBrowserChrome::SetStatus(PRUint32 statusType, const PRUnichar *status)
{
  MozViewListener* pListener = pMozView->GetListener();
  if(pListener) {
    pListener->StatusChanged(NS_ConvertUTF16toUTF8(status).get(), statusType);
    return NS_OK;
  }
  else {
    return NS_ERROR_NOT_IMPLEMENTED;
  }
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
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void sizeBrowserTo (in long aCX, in long aCY); */
NS_IMETHODIMP WebBrowserChrome::SizeBrowserTo(PRInt32 aCX, PRInt32 aCY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void showAsModal (); */
NS_IMETHODIMP WebBrowserChrome::ShowAsModal()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean isWindowModal (); */
NS_IMETHODIMP WebBrowserChrome::IsWindowModal(PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void exitModalEventLoop (in nsresult aStatus); */
NS_IMETHODIMP WebBrowserChrome::ExitModalEventLoop(nsresult aStatus)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}


// ----- Progress Listener -----

/* void onStateChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in unsigned long aStateFlags, in nsresult aStatus); */
NS_IMETHODIMP WebBrowserChrome::OnStateChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRUint32 aStateFlags, nsresult aStatus)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onProgressChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in long aCurSelfProgress, in long aMaxSelfProgress, in long aCurTotalProgress, in long aMaxTotalProgress); */
NS_IMETHODIMP WebBrowserChrome::OnProgressChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRInt32 aCurSelfProgress, PRInt32 aMaxSelfProgress, PRInt32 aCurTotalProgress, PRInt32 aMaxTotalProgress)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onLocationChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in nsIURI aLocation); */
NS_IMETHODIMP WebBrowserChrome::OnLocationChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, nsIURI *aLocation)
{
  MozViewListener* pListener = pMozView->GetListener();
  if(pListener) {
    nsCString spec;
    aLocation->GetSpec(spec);
    pListener->LocationChanged(spec.get());
    return NS_OK;
  }
  else {
    return NS_ERROR_NOT_IMPLEMENTED;
  }
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
    return NS_ERROR_NOT_IMPLEMENTED;
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
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute wstring title; */
NS_IMETHODIMP WebBrowserChrome::GetTitle(PRUnichar * *aTitle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP WebBrowserChrome::SetTitle(const PRUnichar * aTitle)
{
    MozViewListener* pListener = pMozView->GetListener();
    if(pListener) {
        pListener->SetTitle(NS_ConvertUTF16toUTF8(aTitle).get());
        return NS_OK;
    }
    else {
        return NS_ERROR_NOT_IMPLEMENTED;
    }
}

/* [noscript] readonly attribute voidPtr siteWindow; */
NS_IMETHODIMP WebBrowserChrome::GetSiteWindow(void * *aSiteWindow)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}



