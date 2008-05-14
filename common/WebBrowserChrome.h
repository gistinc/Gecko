#ifndef __WebBrowserChrome_h_
#define __WebBrowserChrome_h_

#include "nsCOMPtr.h"
#include "nsIWebBrowser.h"
#include "nsIWebBrowserChrome.h"
#include "nsIWebProgressListener.h"
#include "nsIEmbeddingSiteWindow.h"
#include "nsIInterfaceRequestor.h"
#include "nsWeakReference.h"

class MozView;

class WebBrowserChrome : public nsIWebBrowserChrome,
    public nsIWebProgressListener,
    public nsIEmbeddingSiteWindow,
    public nsIInterfaceRequestor,
    public nsSupportsWeakReference
{
public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSIWEBBROWSERCHROME
    NS_DECL_NSIWEBPROGRESSLISTENER
    NS_DECL_NSIEMBEDDINGSITEWINDOW
    NS_DECL_NSIINTERFACEREQUESTOR

    WebBrowserChrome(MozView* pAMozView);

    virtual ~WebBrowserChrome();

protected:
    /* additional members */
    nsCOMPtr<nsIWebBrowser> mWebBrowser;
    PRUint32 mChromeFlags;
    MozView* pMozView;

};

#endif /* __WebBrowserChrome_h_ */
