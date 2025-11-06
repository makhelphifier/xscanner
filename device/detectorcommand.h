#ifndef _DETECTOR_H
#define _DETECTOR_H

#include <Windows.h>
#include <string>
#include <vector>

#include "third_party/device/detector/include/IRayFpdSys.h"
#include "third_party/device/detector/include/IRayFpdSysEx.h"
#include "winevent.h"

union AttrResult
{
	int   nVal;
	float fVal;
	char  strVal[IRAY_MAX_STR_LEN];
};

class CDetectorManager;
class iEventReceiver;
class CDetector
{
public:
	static CDetector* CreateDetector(const char* pszWorkDir);
	static void DestoryDetector(int id);

	FPDRESULT Connect();
	FPDRESULT Disconnect();
	FPDRESULT SingleAcquire();
    FPDRESULT ClearProcess();
	FPDRESULT PrepAcquire();
	FPDRESULT StartAcquire();
	FPDRESULT StopAcquire();
	FPDRESULT ReadTemperature();
	FPDRESULT SetCorrectionOption(int nValue);
	FPDRESULT SetAttr(int nAttrID, int nValue);
	FPDRESULT SetAttr(int nAttrID, float fValue);
	FPDRESULT SetAttr(int nAttrID, const char* strValue);
	void GetAttr(int nAttrID, AttrResult& result);
//	FPDRESULT Invoke(int cmdId);
//	FPDRESULT Invoke(int cmdId, int nValue);
//	FPDRESULT Invoke(int cmdId, int nPar1, int nPar2);
//	FPDRESULT Invoke(int cmdId, float fValue);
//	FPDRESULT Invoke(int cmdId, const char* strValue);
    FPDRESULT Invoke(int cmdId);
    FPDRESULT Invoke(int cmdId, int nValue);
    FPDRESULT Invoke(int cmdId, int nPar1, int nPar2);
    FPDRESULT Invoke(int cmdId, int nPar1, int nPar2, int nPar3);
    FPDRESULT Invoke(int cmdId, float fValue);
    FPDRESULT Invoke(int cmdId, const char* strValue);
    FPDRESULT Invoke(int nCmdId, int nPara1, int nPara2, const char* strPara1);
    FPDRESULT Invoke(int nCmdId, int nPara1, int nPara2, const char* strPara1, const char* strPara2);
    int WaitEvent(int timeout);
    FPDRESULT SyncInvoke(int cmdId, int timeout);
    FPDRESULT SyncInvoke(int cmdId, int nValue, int timeout);
    FPDRESULT SyncInvoke(int cmdId, int nPar1, int nPar2, int timeout);
    FPDRESULT SyncInvoke(int cmdId, int nPar1, int nPar2, int nPar3, int timeout);
    FPDRESULT SyncInvoke(int cmdId, float fValue, int timeout);
    FPDRESULT SyncInvoke(int cmdId, const char* strValue, int timeout);
    FPDRESULT SyncInvoke(int nCmdId, int nPara1, int nPara2, const char* strPara, int timeout);
    FPDRESULT SyncInvoke(int nCmdId, int nPara1, int nPara2, const char* strPara1, const char* strPara2, int timeout);
    std::string GetErrorInfo(int nErrorCode);
	FPDRESULT SetCaliSubset(const char* pSubDir);
	int DetectorID(){ return m_nDetectorID; }
	void SDKCallback(int nDetectorID, int nEventID, int nEventLevel,
		const char* pszMsg, int nParam1, int nParam2, int nPtrParamLen, void* pParam);
	void RegisterHandler(iEventReceiver* p);
	void UnRegisterHandler(iEventReceiver* p);

	static const int OFFSETMASK;
	static const int GAINMASK;
	static const int DEFECTMASK;
private:
	CDetector();
	~CDetector();
	FPDRESULT Create(const char* pszWorkDir, FnCallback fpCallback);
	FPDRESULT CreateAndConnect(const char* pszWorkDir, FnCallback fpCallback);
	FPDRESULT Destory();
	static CDetectorManager* m_pDetMgr;
	friend class CDetectorManager;

	bool Init();
	void Deinit();

	bool m_bInitilized;
	HMODULE m_hModule;
	FnCreate  m_fpCreate;
	FnDestroy m_fpDestroy;
	FnGetAttr m_fpGetAttr;
	FnSetAttr m_fpSetAttr;
	FnInvoke  m_fpInvoke;
	FnGetErrInfo m_pFnGetErrorInfo;
	int m_nDetectorID;
    HEVENT m_WaitAckEvent;
    int m_CurCmdId;
    int m_nLastError;
    std::vector<iEventReceiver*> m_receivers;
};

#endif
