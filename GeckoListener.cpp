#include "stdafx.h"
#include "GeckoListener.h"
#include "embed.h"
#include "nsCOMPtr.h"
#include "nsIDOMEvent.h"
#include "nsIDOMHTMLAnchorElement.h"
#include "nsIDOMWindow2.h"
#include "nsIDOMEventTarget.h"
#include "nsIDOMNSEventTarget.h"
#include "nsIDOMElement.h"
#include "nsStringAPI.h"
#include "GeckoHost.h"

NS_IMPL_ISUPPORTS1(GeckoListener, nsIDOMEventListener)

HRESULT GeckoListener::Init(GeckoHost * pHost, MozView * pMozView)
{
    HRESULT hr = E_FAIL;
    
    m_pHost = pHost;
    m_pMozView = pMozView;
    nsCOMPtr<nsIDOMEventTarget> eventTarget;
    nsCOMPtr<nsIDOMWindow2> window2 = pMozView->GetDOMWindow();
    if (window2)
    {
        window2->GetWindowRoot(getter_AddRefs(eventTarget));
        if (eventTarget)
        {
            nsCOMPtr<nsIDOMNSEventTarget> nsEventTarget(do_QueryInterface(eventTarget));
            
            if (nsEventTarget)
            {
                nsEventTarget->AddEventListener(NS_LITERAL_STRING("MyEvent1"),
                                               this,
                                               PR_FALSE,
                                               PR_TRUE);
                nsEventTarget->AddEventListener(NS_LITERAL_STRING("MyEvent2"),
                                               this,
                                               PR_FALSE,
                                               PR_TRUE);
               hr = S_OK;
            }
        }
    }
    
    return hr;
}

void GeckoListener::RemoveEventListeners()
{
    nsCOMPtr<nsIDOMEventTarget> eventTarget;
    nsCOMPtr<nsIDOMWindow2> window2 = m_pMozView->GetDOMWindow();
    if (window2)
    {
        window2->GetWindowRoot(getter_AddRefs(eventTarget));
        if (eventTarget)
        {
			eventTarget->RemoveEventListener(NS_LITERAL_STRING("MyEvent1"),
										   this,
										   PR_FALSE);
			eventTarget->RemoveEventListener(NS_LITERAL_STRING("MyEvent2"),
										   this,
										   PR_FALSE);
        }
    }
}

NS_IMETHODIMP GeckoListener::HandleEvent(nsIDOMEvent * pEvent)
{
    nsCOMPtr<nsIDOMEventTarget> target;
    if (pEvent != NULL)
    {
		nsString type;
		pEvent->GetType(type);
		if (type.Compare(NS_LITERAL_STRING("MyEvent1") == 0)
		{
			m_pHost->HandleEvent1();
		}
		else if (type.Compare(NS_LITERAL_STRING("MyEvent2") == 0)
		{
            // Format of the nodes that we will receive the event is as follows:
            //
            //<div>
            //  <div>json 1</div>
            //  <div>json 2</div>
            //</div>

			// Get the original event target
			pEvent->GetTarget(getter_AddRefs(target));
			if (target)
			{
                nsCOMPtr<nsIDOMNode> nsdomRoot(do_QueryInterface(target));
                if (nsdomRoot)
                {
                    nsCOMPtr<nsIDOMNode> nsJSON1Root;
                    
                    // First child will contain a textnode with the JSON 1
                    nsdomRoot->GetFirstChild(getter_AddRefs(nsJSON1Root));
                    if (nsJSON1Root)
                    {
                        nsCOMPtr<nsIDOMNode> nsJSON1;
                        nsCOMPtr<nsIDOMNode> nsJSON2Root;
						nsString JSON1;
						nsString JSON2;
                        
                        // This child node is the actual text node for JSON 1.
                        nsJSON1Root->GetFirstChild(getter_AddRefs(nsJSON1));
                        if (nsJSON1)
                        {
                            nsJSON1->GetNodeValue(JSON1);
                        }
                        
                        // Sibling of the JSON1Root will be the <div> node for JSON2
                        nsJSON1Root->GetNextSibling(getter_AddRefs(nsJSON2Root));
                        if (nsJSON2Root)
                        {
                            nsCOMPtr<nsIDOMNode> nsJSON2;
                            
                            // This child node is the actual text node for JSON 2.
                            nsJSON2Root->GetFirstChild(getter_AddRefs(nsJSON2));
                            if (nsJSON2)
                            {
                                nsJSON2->GetNodeValue(JSON2);
                            }
                        }
                        
						m_pHost->HandleMyEvent2(JSON1.get(), JSON2.get());
                    }
                }
            }
            
			// Close our dialog
            m_pHost->Close();
        }
    }
    
    return NS_OK;
}
