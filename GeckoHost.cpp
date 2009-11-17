// GeckoHost.cpp : implementation file
//

#include "stdafx.h"
#include "GeckoHost.h"
#include "nsCOMPtr.h"

// GeckoHost dialog

IMPLEMENT_DYNAMIC(GeckoHost, CDialog)

GeckoHost::GeckoHost(CString strRemoteUrl, CWnd* pParent /*=NULL*/)
	: CDialog(GeckoHost::IDD, pParent),
	m_strRemoteUrl(strRemoteUrl),
	m_pGeckoListener(NULL),
	m_pmozView(NULL)
{
}

GeckoHost::~GeckoHost()
{
}

BEGIN_MESSAGE_MAP(GeckoHost, CDialog)
END_MESSAGE_MAP()


// GeckoHost message handlers
BOOL GeckoHost::OnInitDialog()
{
	HRESULT hr = S_OK;
	CStringA strURL8;
	CDialog::OnInitDialog();
	RECT rcClient;
	GetClientRect(&rcClient);

	// Ensure we've loaded the xulrunner dlls
	CHRg(InitXul(), "InitXul");

	// Create the embedding host classes (our modified mozembed)
	if (m_pmozView == NULL)
	{
		m_pmozView = new MozView();
		m_pGeckoListener = new GeckoListener();
	}

	// Create the browser, parented to m_hWnd
	m_pmozView->CreateBrowser(m_hWnd, rcClient.left, rcClient.top,
		rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);

	// Load our URL
	strURL8 = ToUTF8(m_strRemoteUrl);
	m_pmozView->LoadURI(strURL8);
	// Do this after the LoadURI to ensure the DOM is created before
	// we start registering to it for events
	m_pGeckoListener->Init(this, m_pmozView);
	return TRUE;

Error:
	return FALSE;
}

// Close the window
void GeckoHost::Close()
{
	if (m_pmozView != NULL)
	{
		m_pmozView->Stop();
		if (m_pGeckoListener != NULL)
		{
			m_pGeckoListener->RemoveEventListeners();
		}
		delete m_pmozView;
		m_pmozView = NULL;
	}
	EndDialog(IDOK);
}
void GeckoHost::HandleEvent1()
{
	// Do stuff here
}

BOOL GeckoHost::HandleEvent2(LPCWSTR pcwszJSON1, LPCWSTR pcwszJSON2)
{
	// Do different stuff here
}

