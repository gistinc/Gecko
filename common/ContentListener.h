#ifndef __ContentListener__
#define __ContentListener__

#include "nsIURIContentListener.h"
#include "nsWeakReference.h"
#include "nsIWebNavigation.h"
#include "embed.h"

class ContentListener : public nsIURIContentListener,
			public nsSupportsWeakReference
{
 public:

  ContentListener(MozView *aOwner, nsIWebNavigation *aNavigation);
  virtual ~ContentListener();

  NS_DECL_ISUPPORTS

  NS_DECL_NSIURICONTENTLISTENER

 private:

  MozView *mOwner;
  nsCOMPtr<nsIWebNavigation> mNavigation;
};

#endif /* __ContentListener__ */
