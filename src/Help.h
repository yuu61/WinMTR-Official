#ifndef HELP_H_
#define HELP_H_

// Help dialog

class Help : public CDialog {
	DECLARE_DYNAMIC(Help)

public:
	explicit Help(CWnd* pParent = nullptr);
	~Help() override = default;

	Help(const Help&)            = delete;
	Help& operator=(const Help&) = delete;
	Help(Help&&)                 = delete;
	Help& operator=(Help&&)      = delete;

	// Dialog Data
	enum { IDD = IDD_DIALOG_HELP };

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

#endif // HELP_H_
