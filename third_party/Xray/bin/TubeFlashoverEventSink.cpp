#include "StdAfx.h"
#include "TubeFlashoverEventSink.h"
#include "TubeInterfaceEventHandler.h"

ULONG TubeFlashoverEventSink::eventSinkID = 0;

void TubeFlashoverEventSink::OnIsAccessibleChanged()
{
	pTubeInterfaceEventHandler->OnIsAccessibleChanged(myEventSinkID);
}

void TubeFlashoverEventSink::OnCountChanged()
{
	pTubeInterfaceEventHandler->OnFlashoverChanged();
}

void TubeFlashoverEventSink::OnLockingXrayChanged()
{
	pTubeInterfaceEventHandler->OnLockingXrayChanged();
}
