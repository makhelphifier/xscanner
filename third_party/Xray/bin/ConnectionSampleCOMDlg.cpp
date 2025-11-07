// ConnectionSampleCOMDlg.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "stdlib.h"
#include "ConnectionSampleCOM.h"
#include "ConnectionSampleCOMDlg.h"
#include "TubeInterfaceEventHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CConnectionSampleCOMDlg-Dialogfeld




CConnectionSampleCOMDlg::CConnectionSampleCOMDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConnectionSampleCOMDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	errorWriteIndex = 0;
	tubeInterfaceEventHandler = new TubeInterfaceEventHandler();
	tubeInterfaceEventHandler->LinkDialog(this);
	m_nTimerID = 3456;
}

void CConnectionSampleCOMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_HighVoltage);
	DDX_Control(pDX, IDC_EDIT2, m_TargetCurrent);
	DDX_Control(pDX, IDC_EDIT3, m_HighVoltagePolling);
	DDX_Control(pDX, IDC_EDIT4, m_TargetCurrentPolling);
	DDX_Control(pDX, IDC_BU_XRAYOFF, m_buXrayOff);
	DDX_Control(pDX, IDC_BU_XRAYON, m_buXrayOn);
	DDX_Control(pDX, IDC_XRAYSTATE, m_lblXrayState);
	DDX_Control(pDX, IDC_EDITERR1, m_edErr1);
	DDX_Control(pDX, IDC_EDITERR2, m_edErr2);
	DDX_Control(pDX, IDC_EDITERR3, m_edErr3);
	DDX_Control(pDX, IDC_EDITERR4, m_edErr4);
	DDX_Control(pDX, IDC_EDINTERLOCK, m_edInterlock);
	DDX_Control(pDX, IDC_EDVACUUM, m_edVacuum);
	DDX_Control(pDX, IDC_EDCOOLING1, m_edCooling1);
	DDX_Control(pDX, IDC_EDCOOLING2, m_edCooling2);
	DDX_Control(pDX, IDC_BUSTARTUP, m_buStartUp);
	DDX_Control(pDX, IDC_EDSTARTUP, m_edStartUpState);
	DDX_Control(pDX, IDC_EDHIVODEM, m_edHighVoltDem);
	DDX_Control(pDX, IDC_EDTACUDEM, m_edTargetCurrentDem);
	DDX_Control(pDX, IDC_FLASHOVER, m_edFlashover);
	DDX_Control(pDX, IDC_COMBO1, cbModes);
}

BEGIN_MESSAGE_MAP(CConnectionSampleCOMDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BU_XRAYOFF, &CConnectionSampleCOMDlg::OnBnClickedBuXrayoff)
	ON_BN_CLICKED(IDC_BU_XRAYON, &CConnectionSampleCOMDlg::OnBnClickedBuXrayon)
	ON_BN_CLICKED(IDC_BUSTARTUP, &CConnectionSampleCOMDlg::OnBnClickedBustartup)
	ON_EN_CHANGE(IDC_EDHIVODEM, &CConnectionSampleCOMDlg::OnEnChangeEdhivodem)
	ON_EN_CHANGE(IDC_EDTACUDEM, &CConnectionSampleCOMDlg::OnEnChangeEdtacudem)
	ON_BN_CLICKED(IDC_BUSTARTPOLLING, &CConnectionSampleCOMDlg::OnBnClickedBustartpolling)
	ON_BN_CLICKED(IDC_BUSTOPPOLLING, &CConnectionSampleCOMDlg::OnBnClickedBustoppolling)
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_COMBO1, &CConnectionSampleCOMDlg::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()


// CConnectionSampleCOMDlg-Meldungshandler

BOOL CConnectionSampleCOMDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	//  wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	SetIcon(m_hIcon, TRUE);			// Großes Symbol verwenden
	SetIcon(m_hIcon, FALSE);		// Kleines Symbol verwenden

	// TODO: Hier zusätzliche Initialisierung einfügen

	return TRUE;  // Geben Sie TRUE zurück, außer ein Steuerelement soll den Fokus erhalten
}

// Wenn Sie dem Dialogfeld eine Schaltfläche "Minimieren" hinzufügen, benötigen Sie 
//  den nachstehenden Code, um das Symbol zu zeichnen. Für MFC-Anwendungen, die das 
//  Dokument/Ansicht-Modell verwenden, wird dies automatisch ausgeführt.

void CConnectionSampleCOMDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // Gerätekontext zum Zeichnen

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Symbol in Clientrechteck zentrieren
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Symbol zeichnen
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// Die System ruft diese Funktion auf, um den Cursor abzufragen, der angezeigt wird, während der Benutzer
//  das minimierte Fenster mit der Maus zieht.
HCURSOR CConnectionSampleCOMDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CConnectionSampleCOMDlg::OnBnClickedBuXrayoff()
{
	tubeInterfaceEventHandler->SwitchXRayOff();
}

void CConnectionSampleCOMDlg::OnBnClickedBuXrayon()
{
	tubeInterfaceEventHandler->SwitchXRayOn();
}

void CConnectionSampleCOMDlg::OnBnClickedBustartup()
{
	tubeInterfaceEventHandler->StartUp();
}

void CConnectionSampleCOMDlg::OnEnChangeEdhivodem()
{
	wchar_t stringBuffer[80];
	m_edHighVoltDem.GetWindowTextW(stringBuffer, 80);
	const wchar_t* stringPtr = stringBuffer;
	float floatValue = (float)_wtof(stringPtr);
	tubeInterfaceEventHandler->SetHighVoltage(floatValue);
}

void CConnectionSampleCOMDlg::OnEnChangeEdtacudem()
{
	wchar_t stringBuffer[80];
	m_edTargetCurrentDem.GetWindowTextW(stringBuffer, 80);
	const wchar_t* stringPtr = stringBuffer;
	float floatValue = (float)_wtof(stringPtr);
	tubeInterfaceEventHandler->SetTargetCurrent(floatValue);
}

void CConnectionSampleCOMDlg::OnBnClickedBustartpolling()
{
	SetTimer(m_nTimerID, 1000, NULL);
}

void CConnectionSampleCOMDlg::OnBnClickedBustoppolling()
{
	KillTimer(m_nTimerID);
}

void CConnectionSampleCOMDlg::OnTimer(UINT_PTR nIDEvent)
{
	float highVoltageMonitor = tubeInterfaceEventHandler->GetHighVoltageMonitor();
	float targetCurrentMonitor = tubeInterfaceEventHandler->GetTargetCurrentMonitor();
	if (m_HighVoltagePolling)
		m_HighVoltagePolling.SetWindowTextW((LPCTSTR)FloatToCStringW(highVoltageMonitor));
	if (m_TargetCurrentPolling)
		m_TargetCurrentPolling.SetWindowTextW((LPCTSTR)FloatToCStringW(targetCurrentMonitor));
}

void CConnectionSampleCOMDlg::OnCbnSelchangeCombo1()
{
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.
}
