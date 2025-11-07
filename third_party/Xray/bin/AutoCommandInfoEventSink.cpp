#include "StdAfx.h"
#include "AutoCommandInfoEventSink.h"
#include "TubeInterfaceEventHandler.h"

ULONG AutoCommandInfoEventSink::eventSinkID = 0;

void AutoCommandInfoEventSink::OnIsAccessibleChanged()
{
	pTubeInterfaceEventHandler->OnIsAccessibleChanged(myEventSinkID);
}
