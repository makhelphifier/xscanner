#include "StdAfx.h"
#include "LimitableAutoCommandEventSink.h"
#include "TubeInterfaceEventHandler.h"

ULONG LimitableAutoCommandEventSink::eventSinkID = 0;

void LimitableAutoCommandEventSink::OnIsAccessibleChanged()
{
	pTubeInterfaceEventHandler->OnIsAccessibleChanged(myEventSinkID);
}

void LimitableAutoCommandEventSink::OnMonitorValueChanged()
{
	pTubeInterfaceEventHandler->OnMonitorValueChanged(myEventSinkID);
}

void LimitableAutoCommandEventSink::OnStateChanged()
{
	pTubeInterfaceEventHandler->OnStateChanged(myEventSinkID);
}

void LimitableAutoCommandEventSink::OnIsActiveChanged()
{
	pTubeInterfaceEventHandler->OnIsActiveChanged(myEventSinkID);
}

void LimitableAutoCommandEventSink::OnLimitedKVMinChanged()
{
	pTubeInterfaceEventHandler->OnLimitedKVMinChanged(myEventSinkID);
}
