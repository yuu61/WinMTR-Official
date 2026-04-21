//*****************************************************************************
// FILE:            Properties.h
//
//
// DESCRIPTION:
//
//
// NOTES:
//
//
//*****************************************************************************

#ifndef PROPERTIES_H_
#define PROPERTIES_H_

#include <ws2tcpip.h>

class HopStatistics;

// Buffer sizes for the host / comment fields displayed by the dialog. IP uses
// INET6_ADDRSTRLEN directly; these are the UI-side text limits.
constexpr size_t kPropertiesHostMaxLen = 255;
constexpr size_t kPropertiesCommentMaxLen = 255;

//*****************************************************************************
// CLASS:  Properties
//
//
//*****************************************************************************

class Properties : public CDialog {
public:
	explicit Properties(CWnd* pParent = nullptr);

	enum { IDD = IDD_DIALOG_PROPERTIES };

	void PopulateFrom(const HopStatistics& stats, int hop);

	wchar_t host[kPropertiesHostMaxLen]{};
	wchar_t ip[INET6_ADDRSTRLEN]{};
	wchar_t comment[kPropertiesCommentMaxLen]{};

	float ping_last = 0.0f;
	float ping_best = 0.0f;
	float ping_avrg = 0.0f;
	float ping_worst = 0.0f;

	int pck_sent = 0;
	int pck_recv = 0;
	int pck_loss = 0;

	CEdit m_editHost;
	CEdit m_editIP;
	CEdit m_editComment;
	CEdit m_editSent;
	CEdit m_editRecv;
	CEdit m_editLoss;
	CEdit m_editLast;
	CEdit m_editBest;
	CEdit m_editWorst;
	CEdit m_editAvrg;

protected:
	void DoDataExchange(CDataExchange* pDX) override;

	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()
};

#endif // ifndef PROPERTIES_H_
