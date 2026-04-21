#ifndef HELP_H_
#define HELP_H_

// Help dialog

class Help : public CDialog
{
	DECLARE_DYNAMIC(Help)

public:
	Help(CWnd* pParent = NULL);   // standard constructor
	virtual ~Help();

// Dialog Data
	enum { IDD = IDD_DIALOG_HELP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

#endif // HELP_H_
