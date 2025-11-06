#ifndef _EVENTRECEIVER_H
#define _EVENTRECEIVER_H

class iEventReceiver
{
public:
	virtual void SDKHandler(int nDetectorID, int nEventID, int nEventLevel,
		const char* pszMsg, int nParam1, int nParam2, int nPtrParamLen, void* pParam) = 0;
};

#endif