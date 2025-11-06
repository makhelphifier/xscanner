#include "DetectorManager.h"
#include <windows.h>
#include <vector>
#include "detectorcommand.h"

std::map<int, CDetector*> CDetectorManager::Detectors;
CDetectorManager::CDetectorManager()
{
}

CDetectorManager::~CDetectorManager()
{
    std::map<int, CDetector*>::iterator iter = Detectors.begin();
	for (; iter != Detectors.end(); ++iter)
	{
		CDetector *pCurDet = iter->second;
        pCurDet->Destory();
		delete pCurDet;
	}
	Detectors.clear();
}

void CDetectorManager::SDKHandler(int nDetectorID, int nEventID, int nEventLevel,
	const char* pszMsg, int nParam1, int nParam2, int nPtrParamLen, void* pParam)
{
	CDetector *pCurDet = Detectors[nDetectorID];
	if (!pCurDet)
		return;
	pCurDet->SDKCallback(nDetectorID, nEventID, nEventLevel, pszMsg, nParam1, nParam2, nPtrParamLen, pParam);
}

CDetector* CDetectorManager::Create(const char* pszWorkDir)
{
	CDetector* pDet = new CDetector();
	int ret = pDet->Create(pszWorkDir, SDKHandler);
	if (Err_OK == ret)
	{
        Detectors.insert(std::make_pair(pDet->DetectorID(), pDet));
	}
	else
	{
		delete pDet;
		pDet = NULL;
	}
	return pDet;
}

CDetector* CDetectorManager::CreateAndConnect(const char* pszWorkDir)
{
	CDetector* pDet = new CDetector();
	int ret = pDet->CreateAndConnect(pszWorkDir, SDKHandler);
	if (Err_OK == ret || Err_TaskPending == ret)
	{
        Detectors.insert(std::make_pair(pDet->DetectorID(), pDet));
	}
	else
	{
		delete pDet;
		pDet = NULL;
	}
	return pDet;
}

int CDetectorManager::Destory(int detectorId)
{
	CDetector *pCurDet = Detectors[detectorId];
	if (!pCurDet) 
		return -1;

	int nDetID = pCurDet->DetectorID();
    pCurDet->Destory();
    pCurDet->Deinit();
	Detectors.erase(nDetID);
	
	return 0;
}
