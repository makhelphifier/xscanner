#include "detectorcommand.h"
#include <tchar.h>
#include <QLibrary>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include "ieventreceiver.h"
#include "DetectorManager.h"

CDetectorManager* CDetector::m_pDetMgr = NULL;
const int CDetector::OFFSETMASK = Enm_CorrectOp_SW_PreOffset | Enm_CorrectOp_SW_PostOffset | Enm_CorrectOp_HW_PreOffset | Enm_CorrectOp_HW_PostOffset;
const int CDetector::GAINMASK = Enm_CorrectOp_SW_Gain | Enm_CorrectOp_HW_Gain;
const int CDetector::DEFECTMASK = Enm_CorrectOp_SW_Defect | Enm_CorrectOp_HW_Defect;

CDetector* CDetector::CreateDetector(const char* pszWorkDir)
{
	if (NULL == m_pDetMgr)
	{
		m_pDetMgr = new CDetectorManager();
	}
	return m_pDetMgr->Create(pszWorkDir);
}

void CDetector::DestoryDetector(int id)
{
	if (NULL != m_pDetMgr)
	{
		m_pDetMgr->Destory(id);
	}
}

CDetector::CDetector()
    : m_bInitilized(false)
    , m_nDetectorID(0)
{
}

CDetector::~CDetector()
{
	Destory();
	Deinit();
}

bool CDetector::Init()
{
    m_hModule = LoadLibrary(_T("FpdSys.DLL"));
    DWORD dwError = 1;
    dwError = GetLastError();
    qDebug()<<"LoadLibrary: "<<dwError;
    if (NULL == m_hModule)
    {
        printf("Load Dll failed!\n");
        return false;
    }
	m_fpCreate = (FnCreate)GetProcAddress(m_hModule, IRAY_FPD_PROC_NAME_CREATE);
	if (NULL == m_fpCreate)
	{
        printf("GetProcAddress:Create failed!\n");
		return false;
	}
	m_fpDestroy = (FnDestroy)GetProcAddress(m_hModule, IRAY_FPD_PROC_NAME_DESTROY);
	if (NULL == m_fpDestroy)
	{
        printf("GetProcAddress:Destroy failed!\n");
		return false;
	}
	m_fpGetAttr = (FnGetAttr)GetProcAddress(m_hModule, IRAY_FPD_PROC_NAME_GETATTR);
	if (NULL == m_fpGetAttr)
	{
        printf("GetProcAddress:GetAttr failed!\n");
		return false;
	}
	m_fpSetAttr = (FnSetAttr)GetProcAddress(m_hModule, IRAY_FPD_PROC_NAME_SETATTR);
	if (NULL == m_fpSetAttr)
	{
        printf("GetProcAddress:etAttr failed!\n");
		return false;
	}
	m_fpInvoke = (FnInvoke)GetProcAddress(m_hModule, IRAY_FPD_PROC_NAME_INVOKE);
	if (NULL == m_fpInvoke)
	{
        printf("GetProcAddress:Invoke failed!\n");
		return false;
	}

	m_pFnGetErrorInfo = (FnGetErrInfo)GetProcAddress(m_hModule, IRAY_FPD_PROC_NAME_GET_ERROR_INFO);
	if (NULL == m_pFnGetErrorInfo)
	{
        printf(" GetProcAddress:GetErrInfo Failed!");
		return false;
	}
	m_bInitilized = true;
	return true;
}

void CDetector::Deinit()
{
	if (m_hModule)
	{
		FreeLibrary(m_hModule);
		m_hModule = NULL;
	}
}

FPDRESULT CDetector::Create(const char* pszWorkDir, FnCallback fpCallback)
{
	if (!m_bInitilized)
	{
		if (!Init())
		{
			return Err_NotInitialized;
		}
	}
		

	FPDRESULT nRet = m_fpCreate(pszWorkDir, fpCallback, &m_nDetectorID);
	if (Err_OK != nRet)
	{
        printf("Create failed\n, errorCode: %d\n", nRet);
		return nRet;
	}

	return nRet;
}

FPDRESULT CDetector::Connect()
{
	FPDRESULT nRet = Invoke(Cmd_Connect);
	if (Err_TaskPending == nRet)
	{
        printf("Connect...\n");
	}
	else
	{
        printf("Connect failed, errorCode: %d\n", nRet);
	}
	return Err_OK;
}

FPDRESULT CDetector::CreateAndConnect(const char* pszWorkDir, FnCallback fpCallback)
{
	FPDRESULT ret = Create(pszWorkDir, fpCallback);
	if (Err_OK != ret)
	{
		return ret;
	}

	return Connect();
}

FPDRESULT CDetector::Destory()
{
	if (m_nDetectorID > 0)
	{
		m_fpDestroy(m_nDetectorID);
		m_nDetectorID = 0;
	}
	return Err_OK;
}

FPDRESULT CDetector::Disconnect()
{
	FPDRESULT nRet = Invoke(Cmd_Disconnect);
	if (Err_TaskPending == nRet)
	{
        printf("DisConnect...\n");
	}
	else
	{
        printf("DisConnect failed, errorCode: %d\n", nRet);
	}
	return Err_OK;
}

FPDRESULT CDetector::SingleAcquire()
{
	return Invoke(Cmd_ForceSingleAcq);
}

FPDRESULT CDetector::ClearProcess()
{
    return Invoke(Cmd_Clear);
}

FPDRESULT CDetector::PrepAcquire()
{
	return Invoke(Cmd_ClearAcq);
}

FPDRESULT CDetector::StartAcquire()
{
	return Invoke(Cmd_StartAcq);
}

FPDRESULT CDetector::StopAcquire()
{
	return Invoke(Cmd_StopAcq);
}

FPDRESULT CDetector::ReadTemperature()
{
	return  Invoke(Cmd_ReadTemperature);
}

FPDRESULT CDetector::SetAttr(int nAttrID, int nValue)
{
	if (!m_bInitilized)
		return Err_NotInitialized;

	IRayVariant var;
	var.vt = IVT_INT;
	var.val.nVal = nValue;
	FPDRESULT result = m_fpSetAttr(m_nDetectorID, nAttrID, &var);
	if (Err_OK != result)
	{
        printf("Set Attribute  failed! Err = %s", GetErrorInfo(result).c_str());
	}
	return result;
}

FPDRESULT CDetector::SetAttr(int nAttrID, float fValue)
{
	if (!m_bInitilized)
		return Err_NotInitialized;

	IRayVariant var;
	var.vt = IVT_FLT;
	var.val.fVal = fValue;
	FPDRESULT result = m_fpSetAttr(m_nDetectorID, nAttrID, &var);
	if (Err_OK != result)
	{
        printf("Set Attribute  failed! Err = %s", GetErrorInfo(result).c_str());
	}
	return result;
}
FPDRESULT CDetector::SetAttr(int nAttrID, const char* strValue)
{
	if (!m_bInitilized)
		return Err_NotInitialized;

	if (!strValue)
		return Err_InvalidParamValue;

	IRayVariant var;
	var.vt = IVT_STR;
	strcpy_s(var.val.strVal, strValue);
	FPDRESULT result = m_fpSetAttr(m_nDetectorID, nAttrID, &var);
	if (Err_OK != result)
	{
        printf("Set Attribute  failed! Err = %s", GetErrorInfo(result).c_str());
	}
	return result;
}

void CDetector::GetAttr(int nAttrID, AttrResult& result)
{
	IRayVariant var;
	FPDRESULT ret = m_fpGetAttr(m_nDetectorID, nAttrID, &var);
	if (Err_OK != ret)
	{
		memset(&result, 0, sizeof(result));
		return;
	}

	if (IVT_INT == var.vt)
	{
		result.nVal = var.val.nVal;
	}
	else if (IVT_FLT == var.vt)
	{
		result.fVal = var.val.fVal;
	}
	else if (IVT_STR == var.vt)
	{
		memcpy_s(result.strVal, IRAY_MAX_STR_LEN, var.val.strVal, IRAY_MAX_STR_LEN);
	}
}


std::string CDetector::GetErrorInfo(int nErrorCode)
{
	ErrorInfo info;
	m_pFnGetErrorInfo(nErrorCode, &info);
	return info.szDescription;
}

/**
SetCaliSubset("Mode1"); 校正子目录为：pszWorkDir\Correct\Mode1
*/
FPDRESULT CDetector::SetCaliSubset(const char* pSubDir)
{
	return  Invoke(Cmd_SetCaliSubset, pSubDir);
}

FPDRESULT CDetector::SetCorrectionOption(int nValue)
{
	return  Invoke(Cmd_SetCorrectOption, nValue);
}

/*
FPDRESULT CDetector::Invoke(int cmdId)
{
	if (!m_bInitilized)
		return Err_NotInitialized;

	FPDRESULT result = m_fpInvoke(m_nDetectorID, cmdId, NULL, 0);
	if (Err_OK != result && Err_TaskPending != result)
	{
        printf("Invoke  failed! Err = %s", GetErrorInfo(result).c_str());
	}
	return result;
}

FPDRESULT CDetector::Invoke(int cmdId, int nValue)
{
	if (!m_bInitilized)
		return Err_NotInitialized;

	IRayCmdParam param;
	param.pt = IPT_VARIANT;
	param.var.vt = IVT_INT;
	param.var.val.nVal = nValue;
	FPDRESULT result = m_fpInvoke(m_nDetectorID, cmdId, &param, 1);
	if (Err_OK != result && Err_TaskPending != result)
	{
        printf("Invoke  failed! Err = %s", GetErrorInfo(result).c_str());
	}
	return result;
}

FPDRESULT CDetector::Invoke(int cmdId, int nPar1, int nPar2)
{
	if (!m_bInitilized)
		return Err_NotInitialized;

	IRayCmdParam param[2];
	param[0].pt = IPT_VARIANT;
	param[0].var.vt = IVT_INT;
	param[0].var.val.nVal = nPar1;
	param[1].pt = IPT_VARIANT;
	param[1].var.vt = IVT_INT;
	param[1].var.val.nVal = nPar2;
	FPDRESULT result = m_fpInvoke(m_nDetectorID, cmdId, param, 2);
	if (Err_OK != result && Err_TaskPending != result)
	{
        printf("Invoke  failed! Err = %s", GetErrorInfo(result).c_str());
	}
	return result;
}

FPDRESULT CDetector::Invoke(int cmdId, float fValue)
{
	if (!m_bInitilized)
		return Err_NotInitialized;

	IRayCmdParam param;
	param.pt = IPT_VARIANT;
	param.var.vt = IVT_FLT;
	param.var.val.fVal = fValue;
	FPDRESULT result = m_fpInvoke(m_nDetectorID, cmdId, &param, 1);
	if (Err_OK != result && Err_TaskPending != result)
	{
        printf("Invoke  failed! Err = %s", GetErrorInfo(result).c_str());
	}
	return result;
}

FPDRESULT CDetector::Invoke(int cmdId, const char* strValue)
{
	if (!m_bInitilized)
		return Err_NotInitialized;

	if (!strValue)
		return Err_InvalidParamValue;
	IRayCmdParam param;
	param.var.vt = IVT_STR;
	//memset(param[0].var.val.strVal, 0, 512);
	//memcpy_s(param[0].var.val.strVal, 512, pSubDir, strlen(pSubDir));
	strcpy_s(param.var.val.strVal, strValue);
	FPDRESULT result = m_fpInvoke(m_nDetectorID, cmdId, &param, 1);
	if (Err_OK != result && Err_TaskPending != result)
	{
        printf("Invoke  failed! Err = %s", GetErrorInfo(result).c_str());
	}
	return result;
}
*/
FPDRESULT CDetector::Invoke(int cmdId)
{
    if (!m_bInitilized)
        return Err_NotInitialized;

    ResetEvent(m_WaitAckEvent);
    m_CurCmdId = cmdId;
    FPDRESULT result = m_fpInvoke(m_nDetectorID, cmdId, NULL, 0);
    if (Err_OK != result && Err_TaskPending != result)
    {
    }
    return result;
}

FPDRESULT CDetector::Invoke(int cmdId, int nValue)
{
    if (!m_bInitilized)
        return Err_NotInitialized;

    ResetEvent(m_WaitAckEvent);
    m_CurCmdId = cmdId;
    IRayCmdParam param;
    param.pt = IPT_VARIANT;
    param.var.vt = IVT_INT;
    param.var.val.nVal = nValue;
    FPDRESULT result = m_fpInvoke(m_nDetectorID, cmdId, &param, 1);
    if (Err_OK != result && Err_TaskPending != result)
    {
    }
    return result;
}

FPDRESULT CDetector::Invoke(int cmdId, int nPar1, int nPar2)
{
    if (!m_bInitilized)
        return Err_NotInitialized;

    ResetEvent(m_WaitAckEvent);
    m_CurCmdId = cmdId;
    IRayCmdParam param[2];
    param[0].pt = IPT_VARIANT;
    param[0].var.vt = IVT_INT;
    param[0].var.val.nVal = nPar1;
    param[1].pt = IPT_VARIANT;
    param[1].var.vt = IVT_INT;
    param[1].var.val.nVal = nPar2;
    FPDRESULT result = m_fpInvoke(m_nDetectorID, cmdId, param, 2);
    if (Err_OK != result && Err_TaskPending != result)
    {
    }
    return result;
}

FPDRESULT CDetector::Invoke(int cmdId, float fValue)
{
    if (!m_bInitilized)
        return Err_NotInitialized;

    ResetEvent(m_WaitAckEvent);
    m_CurCmdId = cmdId;
    IRayCmdParam param;
    param.pt = IPT_VARIANT;
    param.var.vt = IVT_FLT;
    param.var.val.fVal = fValue;
    FPDRESULT result = m_fpInvoke(m_nDetectorID, cmdId, &param, 1);
    if (Err_OK != result && Err_TaskPending != result)
    {
    }
    return result;
}

FPDRESULT CDetector::Invoke(int cmdId, const char* strValue)
{
    if (!m_bInitilized)
        return Err_NotInitialized;

    if (!strValue)
        return Err_InvalidParamValue;

    ResetEvent(m_WaitAckEvent);
    m_CurCmdId = cmdId;
    IRayCmdParam param;
    param.pt = IPT_VARIANT;
    param.var.vt = IVT_STR;
    strncpy(param.var.val.strVal, strValue, IRAY_MAX_STR_LEN-1);
    FPDRESULT result = m_fpInvoke(m_nDetectorID, cmdId, &param, 1);
    if (Err_OK != result && Err_TaskPending != result)
    {
        printf("Invoke  failed! Err = %s", GetErrorInfo(result).c_str());
    }
    return result;
}

FPDRESULT CDetector::Invoke(int nCmdId, int nPara1, int nPara2, const char* strPara1)
{
    if (!m_bInitilized)
        return Err_NotInitialized;

    ResetEvent(m_WaitAckEvent);
    m_CurCmdId = nCmdId;
    IRayCmdParam param[3];
    param[0].pt = IPT_VARIANT;
    param[0].var.vt = IVT_INT;
    param[0].var.val.nVal = nPara1;

    param[1].pt = IPT_VARIANT;
    param[1].var.vt = IVT_INT;
    param[1].var.val.nVal = nPara2;

    param[2].pt = IPT_VARIANT;
    param[2].var.vt = IVT_STR;
    strncpy(param[2].var.val.strVal, strPara1,IRAY_MAX_STR_LEN - 1);

    FPDRESULT result = m_fpInvoke(m_nDetectorID, nCmdId, param, 3);
    if (Err_OK != result && Err_TaskPending != result)
    {
        //print("Invoke  failed! Err = %s", GetErrorInfo(result).c_str());
    }
    return result;
}


FPDRESULT CDetector::Invoke(int nCmdId, int nPara1, int nPara2, const char* strPara1, const char* strPara2)
{
    if (!m_bInitilized)
        return Err_NotInitialized;

    ResetEvent(m_WaitAckEvent);
    m_CurCmdId = nCmdId;
    IRayCmdParam param[4];
    param[0].pt = IPT_VARIANT;
    param[0].var.vt = IVT_INT;
    param[0].var.val.nVal = nPara1;

    param[1].pt = IPT_VARIANT;
    param[1].var.vt = IVT_INT;
    param[1].var.val.nVal = nPara2;

    param[2].pt = IPT_VARIANT;
    param[2].var.vt = IVT_STR;
    strncpy(param[2].var.val.strVal, strPara1, IRAY_MAX_STR_LEN-1);

    param[3].pt = IPT_VARIANT;
    param[3].var.vt = IVT_STR;
    strncpy(param[3].var.val.strVal, strPara2, IRAY_MAX_STR_LEN-1);

    FPDRESULT result = m_fpInvoke(m_nDetectorID, nCmdId, param, 4);
    if (Err_OK != result && Err_TaskPending != result)
    {
        //print("Invoke  failed! Err = %s", GetErrorInfo(result).c_str());
    }
    return result;
}

FPDRESULT CDetector::Invoke(int cmdId, int nPar1, int nPar2, int nPar3)
{
    if (!m_bInitilized)
        return Err_NotInitialized;

    ResetEvent(m_WaitAckEvent);
    m_CurCmdId = cmdId;
    IRayCmdParam param[3];
    param[0].pt = IPT_VARIANT;
    param[0].var.vt = IVT_INT;
    param[0].var.val.nVal = nPar1;
    param[1].pt = IPT_VARIANT;
    param[1].var.vt = IVT_INT;
    param[1].var.val.nVal = nPar2;
    param[2].pt = IPT_VARIANT;
    param[2].var.vt = IVT_INT;
    param[2].var.val.nVal = nPar3;
    FPDRESULT result = m_fpInvoke(m_nDetectorID, cmdId, param, 3);
    if (Err_OK != result && Err_TaskPending != result)
    {
    }
    return result;
}
int CDetector::WaitEvent(int timeout)
{
    int wait = WaitForSingleObject(m_WaitAckEvent, timeout);
    if (WAIT_TIMEOUT == wait)
        return Err_TaskTimeOut;
    else
        return m_nLastError;
}

FPDRESULT CDetector::SyncInvoke(int cmdId, int timeout)
{
    m_nLastError = Err_TaskTimeOut;
    int result = Invoke(cmdId);
    if (Err_TaskPending == result)
    {
        result = WaitEvent(timeout);
    }
    return result;
}

FPDRESULT CDetector::SyncInvoke(int cmdId, int nValue, int timeout)
{
    m_nLastError = Err_TaskTimeOut;
    int result = Invoke(cmdId, nValue);
    if (Err_TaskPending == result)
    {
        result = WaitEvent(timeout);
    }
    return result;
}
FPDRESULT CDetector::SyncInvoke(int cmdId, int nPar1, int nPar2, int timeout)
{
    m_nLastError = Err_TaskTimeOut;
    int result = Invoke(cmdId, nPar1, nPar2);
    if (Err_TaskPending == result)
    {
        result = WaitEvent(timeout);
    }
    return result;
}
FPDRESULT CDetector::SyncInvoke(int cmdId, float fValue, int timeout)
{
    m_nLastError = Err_TaskTimeOut;
    int result = Invoke(cmdId, fValue);
    if (Err_TaskPending == result)
    {
        result = WaitEvent(timeout);
    }
    return result;
}
FPDRESULT CDetector::SyncInvoke(int cmdId, const char* strValue, int timeout)
{
    m_nLastError = Err_TaskTimeOut;
    int result = Invoke(cmdId, strValue);
    if (Err_TaskPending == result)
    {
        result = WaitEvent(timeout);
    }
    return result;
}

FPDRESULT CDetector::SyncInvoke(int cmdId, int nPar1, int nPar2, int nPar3, int timeout)
{
    m_nLastError = Err_TaskTimeOut;
    int result = Invoke(cmdId, nPar1, nPar2, nPar3);
    if (Err_TaskPending == result)
    {
        result = WaitEvent(timeout);
    }
    return result;
}

FPDRESULT CDetector::SyncInvoke(int nCmdId, int nPara1, int nPara2, const char* strPara1, int timeout)
{
    int result = Invoke(nCmdId, nPara1,nPara2,strPara1);
    if (Err_TaskPending == result)
    {
        result = WaitEvent(timeout);
    }
    return result;
}

FPDRESULT CDetector::SyncInvoke(int nCmdId, int nPara1, int nPara2, const char* strPara1, const char* strPara2, int timeout)
{
    int result = Invoke(nCmdId, nPara1, nPara2, strPara1,strPara2);
    if (Err_TaskPending == result)
    {
        result = WaitEvent(timeout);
    }
    return result;
}

void CDetector::SDKCallback(int nDetectorID, int nEventID, int nEventLevel,
	const char* pszMsg, int nParam1, int nParam2, int nPtrParamLen, void* pParam)
{
    std::vector<iEventReceiver*>::iterator iter = m_receivers.begin();
	for (; iter != m_receivers.end(); ++iter)
	{
		iEventReceiver* pCB = *iter;
		pCB->SDKHandler(nDetectorID, nEventID, nEventLevel,
			pszMsg, nParam1, nParam2, nPtrParamLen, pParam);
	}
}

void CDetector::RegisterHandler(iEventReceiver* p)
{
	if (m_receivers.size() == 0)
	{
		m_receivers.push_back(p);
		return;
	}
    std::vector<iEventReceiver*>::iterator iter = std::find(m_receivers.begin(), m_receivers.end(), p);
	if (iter == m_receivers.end())
	{
		m_receivers.push_back(p);
	}
}

void CDetector::UnRegisterHandler(iEventReceiver* p)
{
	if (m_receivers.size() == 0)
	{
		return;
	}
    std::vector<iEventReceiver*>::iterator iter = std::find(m_receivers.begin(), m_receivers.end(), p);
	if (iter != m_receivers.end())
	{
		m_receivers.erase(iter);
	}
}
