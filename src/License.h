//*****************************************************************************
// FILE:            License.h
//
//
// DESCRIPTION:
//   
//
// NOTES:
//    
//
//*****************************************************************************

#ifndef LICENSE_H_
#define LICENSE_H_



//*****************************************************************************
// CLASS:  License
//
//
//*****************************************************************************

class License : public CDialog
{
public:
	License(CWnd* pParent = NULL);

	
	enum { IDD = IDD_DIALOG_LICENSE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	virtual BOOL OnInitDialog();
	
	DECLARE_MESSAGE_MAP()
};

#endif // ifndef LICENSE_H_
