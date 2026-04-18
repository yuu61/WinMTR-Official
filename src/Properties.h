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

#ifndef WINMTRPROPERTIES_H_
#define WINMTRPROPERTIES_H_



//*****************************************************************************
// CLASS:  WinMTRLicense
//
//
//*****************************************************************************

class WinMTRProperties : public CDialog
{
public:
	WinMTRProperties(CWnd* pParent = NULL);

	
	enum { IDD = IDD_DIALOG_PROPERTIES };

	wchar_t	host[255]{};
	wchar_t	ip[16]{};
	wchar_t	comment[255]{};

	float	ping_last  = 0.0f;
	float	ping_best  = 0.0f;
	float	ping_avrg  = 0.0f;
	float	ping_worst = 0.0f;

	int		pck_sent = 0;
	int		pck_recv = 0;
	int		pck_loss = 0;

	CEdit	m_editHost,
			m_editIP,
			m_editComment,
			m_editSent,
			m_editRecv,
			m_editLoss,
			m_editLast,
			m_editBest,
			m_editWorst,
			m_editAvrg;
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	virtual BOOL OnInitDialog();
	
	DECLARE_MESSAGE_MAP()
};

#endif // ifndef WINMTRLICENSE_H_
