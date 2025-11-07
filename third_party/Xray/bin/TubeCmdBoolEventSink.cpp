#include "StdAfx.h"
#include "TubeCmdBoolEventSink.h"
#include "TubeInterfaceEventHandler.h"

ULONG TubeCmdBoolEventSink::eventSinkID = 0;

void TubeCmdBoolEventSink::OnIsAccessibleChanged()
{
	pTubeInterfaceEventHandler->OnIsAccessibleChanged(myEventSinkID);
}

void TubeCmdBoolEventSink::OnMonitorValueChanged()
{
	pTubeInterfaceEventHandler->OnMonitorValueChanged(myEventSinkID);
}

void TubeCmdBoolEventSink::OnStateChanged()
{
	pTubeInterfaceEventHandler->OnStateChanged(myEventSinkID);
}