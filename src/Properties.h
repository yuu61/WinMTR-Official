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

class HopStatistics;

//*****************************************************************************
// CLASS:  Properties
//
//
//*****************************************************************************

class Properties : public CDialog
{
public:
	Properties(CWnd* pParent = NULL);

	enum { IDD = IDD_DIALOG_PROPERTIES };

	void PopulateFrom(const HopStatistics& stats, int hop);

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

#endif // ifndef PROPERTIES_H_
