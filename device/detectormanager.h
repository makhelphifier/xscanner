#ifndef _DETECTORMANAGER_H
#define _DETECTORMANAGER_H

#include <map>

class CDetector;
class CDetectorManager
{
public:
	CDetectorManager();
	~CDetectorManager();
	CDetector* CreateAndConnect(const char* pszWorkDir);
	CDetector* Create(const char* pszWorkDir);
	int Destory(int detectorId);
	static void SDKHandler(int nDetectorID, int nEventID, int nEventLevel,
		const char* pszMsg, int nParam1, int nParam2, int nPtrParamLen, void* pParam);

private:
    static std::map<int, CDetector*> Detectors;
	
};

#endif
