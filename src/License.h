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

class License : public CDialog {
public:
	explicit License(CWnd* pParent = nullptr);


	enum { IDD = IDD_DIALOG_LICENSE };

protected:
	void DoDataExchange(CDataExchange* pDX) override;

	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()
};

#endif // ifndef LICENSE_H_
