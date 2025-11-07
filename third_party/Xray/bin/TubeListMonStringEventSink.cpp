#include "StdAfx.h"
#include "TubeListMonStringEventSink.h"
#include "TubeInterfaceEventHandler.h"

ULONG TubeListMonStringEventSink::eventSinkID = 0;

void TubeListMonStringEventSink::OnIsAccessibleChanged()
{
	pTubeInterfaceEventHandler->OnIsAccessibleChanged(myEventSinkID);
}

void TubeListMonStringEventSink::OnListChanged()
{
	pTubeInterfaceEventHandler->OnListChanged(myEventSinkID);
}
