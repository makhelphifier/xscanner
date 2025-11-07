// ConnectionSampleCOMDlg.h : Headerdatei
//

#pragma once
#include "afxwin.h"

class TubeInterfaceEventHandler;

// CConnectionSampleCOMDlg-Dialogfeld
class CConnectionSampleCOMDlg : public CDialog
{
private:
	//Member
	TubeInterfaceEventHandler* tubeInterfaceEventHandler;
	USHORT errorWriteIndex;
	UINT_PTR m_nTimerID;

// Konstruktion
public:
	CConnectionSampleCOMDlg(CWnd* pParent = NULL);	// Standardkonstruktor

// Dialogfelddaten
	enum { IDD = IDD_CONNECTIONSAMPLECOM_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterstützung


// Implementierung
protected:
	HICON m_hIcon;

	// Generierte Funktionen für die Meldungstabellen
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
	CEdit m_HighVoltage;
	CEdit m_HighVoltagePolling;
	CEdit m_TargetCurrent;
	CEdit m_TargetCurrentPolling;
public:

	CStringW FloatToCStringW(float value)
	{
		CStringW str;
		str.Format(_T("%f"), value);
		return str;
	}

	CStringW UlongToCStringW(ULONG value)
	{
		CStringW str;
		str.Format(_T("%i"), value);
		return str;
	}

	void SetHighVoltageMonitor(float value)
	{
		if (m_HighVoltage)
			m_HighVoltage.SetWindowTextW((LPCTSTR)FloatToCStringW(value));
	}

	void SetTargetCurrentMonitor(float value)
	{
		if (m_TargetCurrent)
			m_TargetCurrent.SetWindowTextW((LPCTSTR)FloatToCStringW(value));
	}

	void SetFlashoverCount(unsigned long count)
	{
		if (m_edFlashover)
			m_edFlashover.SetWindowTextW((LPCTSTR)UlongToCStringW(count));
	}

	void SetError(ULONG errorCode)
	{
		if (errorWriteIndex == 0)
			m_edErr1.SetWindowTextW((LPCTSTR)UlongToCStringW(errorCode));
		else if (errorWriteIndex == 1)
			m_edErr2.SetWindowTextW((LPCTSTR)UlongToCStringW(errorCode));
		else if (errorWriteIndex == 2)
			m_edErr3.SetWindowTextW((LPCTSTR)UlongToCStringW(errorCode));
		else
			m_edErr4.SetWindowTextW((LPCTSTR)UlongToCStringW(errorCode));
		errorWriteIndex = (++errorWriteIndex) % 4;
	}

	CButton m_buXrayOff;
	CButton m_buXrayOn;
	CStatic m_lblXrayState;
	afx_msg void OnBnClickedBuXrayoff();
	afx_msg void OnBnClickedBuXrayon();
	CEdit m_edErr1;
	CEdit m_edErr2;
	CEdit m_edErr3;
	CEdit m_edErr4;
	CEdit m_edInterlock;
	CEdit m_edVacuum;
	CEdit m_edCooling1;
	CEdit m_edCooling2;
	CButton m_buStartUp;
	afx_msg void OnBnClickedBustartup();
	CEdit m_edStartUpState;
	CEdit m_edHighVoltDem;
	CEdit m_edTargetCurrentDem;
	afx_msg void OnEnChangeEdhivodem();
	afx_msg void OnEnChangeEdtacudem();
	afx_msg void OnBnClickedBustartpolling();
	afx_msg void OnBnClickedBustoppolling();
	CEdit m_edFlashover;
	CComboBox cbModes;
	afx_msg void OnCbnSelchangeCombo1();
};
