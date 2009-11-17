#pragma once
#include "nsIDOMEventListener.h"
#include "embed.h"
#include "GeckoHost.h"

class GeckoHost;

class GeckoListener : public nsIDOMEventListener
{
public:
    HRESULT Init(GeckoHost * pHost, MozView * pMozView);
    void RemoveEventListeners();
    
    NS_DECL_ISUPPORTS
    NS_DECL_NSIDOMEVENTLISTENER
    
private:
    GeckoHost * m_pHost;
    MozView * m_pMozView;
};