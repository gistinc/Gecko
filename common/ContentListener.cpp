#include "xpcom-config.h"
#include "mozilla-config.h"

#include "nsXULAppAPI.h"
#include "nsXPCOMGlue.h"

#include "nsCOMPtr.h"
#include "nsStringAPI.h"
#include "nsILocalFile.h"
#include "nsIURI.h"
#include "nsIWebNavigationInfo.h"
#include "nsServiceManagerUtils.h"
#include "nsDocShellCID.h"

#include "embed.h"
#include "ContentListener.h"

ContentListener::ContentListener(MozView *aOwner, nsIWebNavigation *aNavigation)
{
  mOwner = aOwner;
  mNavigation = aNavigation;
}

ContentListener::~ContentListener()
{
}

NS_IMPL_ISUPPORTS2(ContentListener,
                   nsIURIContentListener,
                   nsISupportsWeakReference)

NS_IMETHODIMP
ContentListener::OnStartURIOpen(nsIURI     *aURI,
				PRBool     *aAbortOpen)
{
  nsresult rv;
  nsCAutoString specString;
  rv = aURI->GetSpec(specString);

  if (NS_FAILED(rv))
    return rv;

  MozViewListener *listener = mOwner->GetListener ();
  if (listener) {      
      *aAbortOpen = listener->OpenURI (specString.get());
  } else
      *aAbortOpen = false;

  return NS_OK;
}

NS_IMETHODIMP
ContentListener::DoContent(const char         *aContentType,
			   PRBool             aIsContentPreferred,
			   nsIRequest         *aRequest,
			   nsIStreamListener **aContentHandler,
			   PRBool             *aAbortProcess)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
ContentListener::IsPreferred(const char        *aContentType,
			     char             **aDesiredContentType,
			     PRBool            *aCanHandleContent)
{
    return CanHandleContent(aContentType, PR_TRUE, aDesiredContentType,
			    aCanHandleContent);
}

NS_IMETHODIMP
ContentListener::CanHandleContent(const char        *aContentType,
				  PRBool           aIsContentPreferred,
				  char             **aDesiredContentType,
				  PRBool            *_retval)
{
    *_retval = PR_FALSE;
    *aDesiredContentType = nsnull;

    if (aContentType) {
	nsCOMPtr<nsIWebNavigationInfo> webNavInfo(do_GetService(NS_WEBNAVIGATION_INFO_CONTRACTID));
	if (webNavInfo) {
	    PRUint32 canHandle;
	    nsresult rv =
		webNavInfo->IsTypeSupported(nsDependentCString(aContentType),
					    mNavigation ? mNavigation.get() : nsnull,
					    &canHandle);
	    NS_ENSURE_SUCCESS(rv, rv);
	    *_retval = (canHandle != nsIWebNavigationInfo::UNSUPPORTED);
	}
    }
    return NS_OK;
}

NS_IMETHODIMP
ContentListener::GetLoadCookie(nsISupports **aLoadCookie)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
ContentListener::SetLoadCookie(nsISupports *aLoadCookie)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
ContentListener::GetParentContentListener(nsIURIContentListener **aParent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
ContentListener::SetParentContentListener(nsIURIContentListener *aParent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

