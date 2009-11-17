#pragma once
#include "stdafx.h"
#include "embed.h"
#include "GeckoListener.h"

// GeckoHost dialog

class GeckoHost : public CDialog
{
	DECLARE_DYNAMIC(GeckoHost)

public:
	GeckoHost(CString strRemoteUrl, CWnd* pParent = NULL); 
	virtual ~GeckoHost();

// Dialog Data
	enum { IDD = IDD_GECKOHOST };

	// Dispatch function
	void HandleEvent1();
	BOOL HandleEvent2(LPCWSTR pcwszJSON1, LPCWSTR pcwszJSON2);
	void Close();

protected:
	virtual BOOL OnInitDialog();

private:
	CString m_strRemoteUrl;

	DECLARE_MESSAGE_MAP()

	MozView		* m_pmozView;
	GeckoListener * m_pGeckoListener;
};
