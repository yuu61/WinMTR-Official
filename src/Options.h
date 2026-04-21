//*****************************************************************************
// FILE:            Options.h
//
//
// DESCRIPTION:
//
//
// NOTES:
//
//
//*****************************************************************************

#ifndef OPTIONS_H_
#define OPTIONS_H_


//*****************************************************************************
// CLASS:  Options
//
//
//*****************************************************************************

class Options : public CDialog {
public:
	explicit Options(CWnd* pParent = nullptr);

	void SetUseDNS(BOOL udns) { useDNS = udns; }
	void SetInterval(double i) { interval = i; }
	void SetPingSize(int ps) { pingsize = ps; }
	void SetMaxLRU(int mlru) { maxLRU = mlru; }

	[[nodiscard]] double GetInterval() const { return interval; }
	[[nodiscard]] int GetPingSize() const { return pingsize; }
	[[nodiscard]] int GetMaxLRU() const { return maxLRU; }
	[[nodiscard]] BOOL GetUseDNS() const { return useDNS; }

	enum { IDD = IDD_DIALOG_OPTIONS };
	CEdit m_editSize;
	CEdit m_editInterval;
	CEdit m_editMaxLRU;
	CButton m_checkDNS;

protected:
	void DoDataExchange(CDataExchange* pDX) override;

	BOOL OnInitDialog() override;
	void OnOK() override;

	afx_msg void OnLicense();

	DECLARE_MESSAGE_MAP()

private:
	double interval = 0.0;
	int pingsize = 0;
	int maxLRU = 0;
	BOOL useDNS = FALSE;
};

#endif // ifndef OPTIONS_H_
