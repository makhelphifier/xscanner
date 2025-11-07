// ConnectionSampleCOM.h : Hauptheaderdatei für die PROJECT_NAME-Anwendung
//

#pragma once

#ifndef __AFXWIN_H__
	#error "\"stdafx.h\" vor dieser Datei für PCH einschließen"
#endif

#include "resource.h"		// Hauptsymbole

// CConnectionSampleCOMApp:
// Siehe ConnectionSampleCOM.cpp für die Implementierung dieser Klasse
//

class CConnectionSampleCOMApp : public CWinApp
{
public:
	CConnectionSampleCOMApp();

// Überschreibungen
	public:
	virtual BOOL InitInstance();

// Implementierung

	DECLARE_MESSAGE_MAP()
};

extern CConnectionSampleCOMApp theApp;