#include "irayfpdadapter.h"
#include <QMetaType>
#include <QDebug>
#include <QTimer>
#include <QDateTime>

int saveRawFile(const QImage& img, const QString& path)
{
    FILE *fpdst = NULL;
    fpdst = fopen(path.toStdString().c_str(), "wb");
    if (fpdst == NULL) {
        return -1;
    }
    int size = img.bytesPerLine() * img.height();
    ushort *data = new ushort[size];
    memcpy(data, img.constBits(), size);
    fwrite(data, sizeof(ushort), size, fpdst);
    fclose(fpdst);
    delete[] data;
    return 0;
}

IRayFpdAdapter::IRayFpdAdapter(QObject *parent) : QObject{parent}
{
    qRegisterMetaType<FPDRESULT>("FPDRESULT");
    qRegisterMetaType<ImageInfo>("ImageInfo");
    qRegisterMetaType<IRayImage>("IRayImage");
}
IRayFpdAdapter::~IRayFpdAdapter()
{
    qDebug()<<"IRayFpdAdapter::~IRayFpdAdapter()";
    if(m_pDetector == nullptr)
        return;
    m_pDetector->UnRegisterHandler(this);
    CDetector::DestoryDetector(m_pDetector->DetectorID());
}
void IRayFpdAdapter::SDKHandler(int nDetectorID, int nEventID, int nEventLevel, const char *pszMsg, int nParam1, int nParam2, int nPtrParamLen, void *pParam)
{
    Q_UNUSED(nDetectorID);
    emit sigEvent(nEventID, nEventLevel, pszMsg, nParam1, nParam2, nPtrParamLen, pParam);

//    qDebug()<<"nEventID: "<<nEventID;
    switch (nEventID) {
    case Evt_TaskResult_Succeed://任务成功
    {
        switch (nParam1) {
        case Cmd_Connect:
        {
            emit sig_didConnect(Err_OK);
            break;
        }
        case Cmd_Disconnect:
        {
            emit sig_didDisconnect(Err_OK);
            break;
        }
        case Cmd_StartAcq:  // startLightAcqusition()
        {
            m_timeCost = 0;
            m_startTimeStamp = 0;
            m_imgIndex = 0;

            switch (m_mode) {
            case IRayFPDAdapterModeNormal:
            {
                emit sig_didStartAcquiring(Err_OK);
                break;
            }
            case IRayFPDAdapterModeOffsetTemplate:
            {
                break;
            }
            case IRayFPDAdapterModeGainDefectTemplate:
            {
                emit sig_didStartLightAcqusition(Err_OK);
                m_acqMode = 1;
//                m_gainDefectImgNum = 0;
                startSelecting4ValidImages();

                break;
            }

            default:
            {
                break;
            }
            }


            break;
        }
        case Cmd_StopAcq:
        {


            switch (m_mode) {
            case IRayFPDAdapterModeNormal:
            {
                emit sig_didStopAcquiring(Err_OK);
                break;
            }
            case IRayFPDAdapterModeOffsetTemplate:
            {
                break;
            }
            case IRayFPDAdapterModeGainDefectTemplate:
            {
                emit sig_didStopAcquiring(Err_OK);
                break;
            }

            default:
            {
                break;
            }
            }

            break;
        }
        case Cmd_SetCaliSubset:
        {
            emit sig_didSetCaliSubset(Err_OK);
            break;
        }
        case Cmd_WriteUserROM:
        {
            emit sig_didWriteUserROM(Err_OK);
            break;
        }
        case Cmd_ReadTemperature:
        {

            break;
        }
        case Cmd_ChangeParamsOfCurrentAppMode:
        {
            emit sig_didChangeParamsOfCurrentAppMode(Err_OK);
            break;
        }
        case Cmd_OffsetGeneration:
        {
            switch (m_mode) {
            case IRayFPDAdapterModeNormal:
            {


                break;
            }
            case IRayFPDAdapterModeOffsetTemplate:
            {
                break;
            }
            case IRayFPDAdapterModeGainDefectTemplate:
            {

                break;
            }

            default:
            {
                break;
            }
            }
            emit sig_didOffsetGeneration(Err_OK);
            break;
        }
        case Cmd_SetCorrectOption:
        {
            emit sig_didSetCorrectOption(Err_OK);
            break;
        }
        case Cmd_GainInit:
        {
            emit sig_didGainInit(Err_OK);
            m_nCurGroupIndex = 0;
            break;
        }
        case Cmd_DefectInit:    // defectInit()
        {
            emit sig_didDefectInit(Err_OK);
            m_nCurGroupIndex = 0;
            break;
        }
        case Cmd_GainSelectAll:
        {
            emit sig_didSelect4ValidImages(Err_OK);
            break;
        }
        case Cmd_DefectSelectAll:   // select4ValidImages()
        {
            emit sig_didSelect4ValidImages(Err_OK);
            break;
        }
        case Cmd_ForceDarkContinuousAcq:    // startDarkAcqusition()
        {
            switch (m_mode) {
            case IRayFPDAdapterModeNormal:
            {
                break;
            }
            case IRayFPDAdapterModeOffsetTemplate:
            {
                break;
            }
            case IRayFPDAdapterModeGainDefectTemplate:
            {
                emit sig_didStartDarkAcqusition(Err_OK);
//                m_gainDefectImgNum = 0;
                m_acqMode = 2;



//                emit sig_didGetGroupImage(m_nCurGroupIndex, m_nTotalGroup);

                break;
            }

            default:
            {
                break;
            }
            }

            break;
        }
        case Cmd_GainGeneration:
        {
            emit sig_didGenerateTemplateFiles(Err_OK);
            finishGeneration();
            break;
        }
        case Cmd_DefectGeneration:  // generateTemplateFiles()
        {
            emit sig_didGenerateTemplateFiles(Err_OK);
            finishGeneration();
            break;
        }
        case Cmd_FinishGenerationProcess:   // finishGeneration()
        {
            emit sig_didFinishMakingTemplate(Err_OK);
            break;
        }
        default:
        {
            break;
        }
        }
        break;
    }
    case Evt_TaskResult_Failed://任务失败
    {
        switch (nParam1) {
        case Cmd_Connect:
        {
            emit sig_didConnect(nParam2);
            break;
        }
        case Cmd_Disconnect:
        {
            emit sig_didDisconnect(nParam2);
            break;
        }
        case Cmd_StartAcq:
        {
            switch (m_mode) {
            case IRayFPDAdapterModeNormal:
            {
                emit sig_didStartAcquiring(nParam2);
                break;
            }
            case IRayFPDAdapterModeOffsetTemplate:
            {
                break;
            }
            case IRayFPDAdapterModeGainDefectTemplate:
            {
                switch (m_fpdType) {
                case IRayFPDAdapterFPDTypeNDT:
                {
                    break;
                }
                case IRayFPDAdapterFPDTypeDTDI:
                {
                    emit sig_didStartLightAcqusition(nParam2);
                    break;
                }
                default:
                {
                    break;
                }
                }
                break;
            }
            default:
            {
                break;
            }
            }

            break;
        }
        case Cmd_StopAcq:
        {



            switch (m_mode) {
            case IRayFPDAdapterModeNormal:
            {
                emit sig_didStopAcquiring(nParam2);
                break;
            }
            case IRayFPDAdapterModeOffsetTemplate:
            {
                break;
            }
            case IRayFPDAdapterModeGainDefectTemplate:
            {
                emit sig_didStopAcquiring(nParam2);
                break;
            }

            default:
            {
                break;
            }
            }

            break;
        }
        case Cmd_SetCaliSubset:
        {
            emit sig_didSetCaliSubset(nParam2);
            break;
        }
        case Cmd_WriteUserROM:
        {
            emit sig_didWriteUserROM(nParam2);
            break;
        }
        case Cmd_ReadTemperature:
        {
            emit sig_didReadTemperature(nParam2, 0);
            break;
        }
        case Cmd_ChangeParamsOfCurrentAppMode:
        {
            emit sig_didChangeParamsOfCurrentAppMode(nParam2);
            break;
        }
        case Cmd_OffsetGeneration:
        {
            switch (m_mode) {
            case IRayFPDAdapterModeNormal:
            {


                break;
            }
            case IRayFPDAdapterModeOffsetTemplate:
            {
                break;
            }
            case IRayFPDAdapterModeGainDefectTemplate:
            {

                break;
            }

            default:
            {
                break;
            }
            }
            emit sig_didOffsetGeneration(nParam2);
            break;
        }
        case Cmd_SetCorrectOption:
        {
            emit sig_didSetCorrectOption(nParam2);
            break;
        }
        case Cmd_GainInit:
        {
            emit sig_didGainInit(nParam2);
            break;
        }
        case Cmd_DefectInit:    // defectInit()
        {
            emit sig_didDefectInit(nParam2);
            break;
        }
        case Cmd_GainSelectAll:
        {
            emit sig_didSelect4ValidImages(nParam2);
            break;
        }
        case Cmd_DefectSelectAll:   // select4ValidImages()
        {
            emit sig_didSelect4ValidImages(nParam2);
            break;
        }
        case Cmd_ForceDarkContinuousAcq:    // startDarkAcqusition()
        {
            switch (m_mode) {
            case IRayFPDAdapterModeNormal:
            {
                break;
            }
            case IRayFPDAdapterModeOffsetTemplate:
            {
                break;
            }
            case IRayFPDAdapterModeGainDefectTemplate:
            {
                switch (m_fpdType) {
                case IRayFPDAdapterFPDTypeNDT:
                {
                    break;
                }
                case IRayFPDAdapterFPDTypeDTDI:
                {
                    emit sig_didFinishTemplateAcquiring(nParam2);

                    break;
                }
                default:
                {
                    break;
                }
                }
                break;
            }

            default:
            {
                break;
            }
            }

            break;
        }
        case Cmd_GainGeneration:
        {
            emit sig_didGenerateTemplateFiles(nParam2);
            break;
        }
        case Cmd_DefectGeneration:
        {
            emit sig_didGenerateTemplateFiles(nParam2);
            break;
        }
        case Cmd_FinishGenerationProcess:   // finishGeneration()
        {
            emit sig_didFinishMakingTemplate(nParam2);
            break;
        }
        default:
        {
            break;
        }
        }
        break;
    }
    case Evt_Image://获得图像
    {

        qint64 timeStamp = QDateTime::currentMSecsSinceEpoch();
        if (m_startTimeStamp > 0) {
            m_timeCost += (timeStamp-m_startTimeStamp);

//            qDebug()<<++m_imgIndex<<"- time: "<<m_timeCost;
        }
        m_startTimeStamp = timeStamp;

        // return;


        IRayImage* pImg = (IRayImage*)pParam;
        switch(pImg->nBytesPerPixel) {
        case 1:
        {
//            QImage img(pImg->nWidth, pImg->nHeight, QImage::Format_Indexed8);
//            int size = pImg->nWidth * pImg->nHeight * pImg->nBytesPerPixel;
//            memcpy(img.bits(), pImg->pData, size);
//            emit sig_didGetImage(img);
            break;
        }
        case 2:
        {
            switch (m_mode) {
            case IRayFPDAdapterModeNormal:
            {
                break;
            }
            case IRayFPDAdapterModeOffsetTemplate:
            {
                break;
            }
            case IRayFPDAdapterModeGainDefectTemplate:
            {
                IRayVariantMapItem* pFirstItem = pImg->propList.pItems;
                int nCurGrayValue = 0;
                int nItemCnt = pImg->propList.nItemCount;
                while (nItemCnt--) {
                    if (Enm_ImageTag_CenterValue == pFirstItem->nMapKey) {
                        nCurGrayValue = pFirstItem->varMapVal.val.nVal;
                        break;
                    }
                    pFirstItem++;
                }
                int diff = m_ExpectedGrays[m_nCurGroupIndex] - nCurGrayValue;
                if (abs(diff) * 10 > m_ExpectedGrays[m_nCurGroupIndex]) {
                    qDebug()<<"Please ajust dose: ["<<diff<<"] gray-value";
                }


                if (m_acqMode == 1) {
                    m_gainDefectImgNum += 1;
                    if (m_gainDefectImgNum == m_nEachGroupFrames/2) {
                        emit sig_didGetLightImages(1);
                    }

                }
                else if (m_acqMode == 2) {
//                    if (m_gainDefectImgNum < m_nEachGroupFrames/2) {
//                        m_gainDefectImgNum += 1;
//                    }
//                    if (m_gainDefectImgNum == m_nEachGroupFrames/2) {
//                        m_nCurGroupIndex += 1;
//                        if (m_nCurGroupIndex < m_nTotalGroup) {
//                            startLightAcqusition();
//                        }
//                        else {
//                            m_acqMode = 0;
//                            generateTemplateFiles();
//                        }
//                    }

                }
                qDebug()<<"m_gainDefectImgNum: "<<m_gainDefectImgNum;




                break;
            }

            default:
                break;
            }




            QImage img(pImg->nWidth, pImg->nHeight, QImage::Format_Grayscale16);
            int size = pImg->nWidth * pImg->nHeight * pImg->nBytesPerPixel;
            memcpy((ushort*)img.bits(), pImg->pData, size);

            emit sig_didGetImage(img);




            break;
        }
        case 3:
        {
//            QImage img(pImg->nWidth, pImg->nHeight, QImage::Format_RGB888);
//            int size = pImg->nWidth * pImg->nHeight * pImg->nBytesPerPixel;
//            memcpy(img.bits(), pImg->pData, size);
//            emit sig_didGetImage(img);
            break;
        }
        case 4:
        {
//            QImage img(pImg->nWidth, pImg->nHeight, QImage::Format_ARGB32);
//            int size = pImg->nWidth * pImg->nHeight * pImg->nBytesPerPixel;
//            memcpy(img.bits(), pImg->pData, size);
//            emit sig_didGetImage(img);
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}



QImage IRayFpdAdapter::gray16ImgFromIRayImage(IRayImage *iRayImg)
{


    int imgWidth = iRayImg->nWidth;
    int imgHeight = iRayImg->nHeight;
    QImage img(iRayImg->nWidth, iRayImg->nHeight, QImage::Format_Grayscale16);
    for (int j = 0; j < imgHeight; ++j) {
        quint16 *dst =  reinterpret_cast<quint16*>(img.bits() + j * img.bytesPerLine());
        for (int i = 0; i < imgWidth; ++i) {
            unsigned short pixelval =  static_cast<unsigned short>(iRayImg->pData[i + j * imgWidth]);
            dst[i] = pixelval;
        }

    }
    return img;
}




IRayFpdAdapter* IRayFpdAdapter::create(const QString& workDirPath, QObject *parent)
{
    CDetector* pDetector = CDetector::CreateDetector(workDirPath.toStdString().c_str());
    if(!pDetector) {
        return nullptr;
    }
    IRayFpdAdapter* adapter = new IRayFpdAdapter(parent);
    adapter->m_pDetector = pDetector;
    pDetector->RegisterHandler(adapter);
    return adapter;
}

QString IRayFpdAdapter::getErrorInfo(int error)
{
    if(m_pDetector == nullptr) {
        return QString();
    }
    return QString::fromStdString(m_pDetector->GetErrorInfo(error));
}

FPDRESULT IRayFpdAdapter::connect()
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->Connect();
}

FPDRESULT IRayFpdAdapter::disconnect()
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->Disconnect();
}

FPDRESULT IRayFpdAdapter::cleanCache()
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->ClearProcess();
}

IRayFpdAdapter::DetectState IRayFpdAdapter::getDetectState()
{
    if(m_pDetector == nullptr) {
        return DS_UNKNOW;
    }
    AttrResult attr;
    m_pDetector->GetAttr(Attr_State, attr);
    return (IRayFpdAdapter::DetectState)attr.nVal;
}

FPDRESULT IRayFpdAdapter::prepAcquire()
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->PrepAcquire();
}

FPDRESULT IRayFpdAdapter::singleAcquire()
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->SingleAcquire();
}

FPDRESULT IRayFpdAdapter::startAcquire()
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    m_mode = IRayFPDAdapterModeNormal;
    m_acqMode = 0;
    return m_pDetector->StartAcquire();
}

FPDRESULT IRayFpdAdapter::stopAcquire()
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
//    AttrResult attr;
//    m_pDetector->GetAttr(Attr_)
    return m_pDetector->StopAcquire();
}

FPDRESULT IRayFpdAdapter::setCorrectionOption(int correctionOption)
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->SetCorrectionOption(correctionOption);
}

FPDRESULT IRayFpdAdapter::readTemperature()
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->ReadTemperature();
}

Enm_ProdType IRayFpdAdapter::getProdType()
{
    if(m_pDetector == nullptr) {
        return (Enm_ProdType)-1;
    }
    AttrResult attr;
    m_pDetector->GetAttr(Attr_UROM_ProductNo, attr);
    return (Enm_ProdType)attr.nVal;
}

Enm_FluroSync IRayFpdAdapter::getSyncMode()
{
    if(m_pDetector == nullptr) {
        return (Enm_FluroSync)-1;
    }
    AttrResult attr;
    m_pDetector->GetAttr(Attr_UROM_ProductNo, attr);
    if (Enm_Prd_Mercu0909F != attr.nVal) {
        return (Enm_FluroSync)-1;
    }
    m_pDetector->GetAttr(Attr_UROM_FluroSync, attr);
    return (Enm_FluroSync)attr.nVal;
}

Enm_TriggerMode IRayFpdAdapter::getTriggerMode()
{
    if(m_pDetector == nullptr) {
        return (Enm_TriggerMode)-1;
    }
    AttrResult attr;
    m_pDetector->GetAttr(Attr_UROM_ProductNo, attr);
    if (Enm_Prd_Mercu0909F == attr.nVal) {
        return (Enm_TriggerMode)-1;
    }
    m_pDetector->GetAttr(Attr_UROM_TriggerMode, attr);
    return (Enm_TriggerMode)attr.nVal;
}

FPDRESULT IRayFpdAdapter::setApplicationMode(const QString &mode)
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->Invoke(Cmd_SetCaliSubset, mode.toStdString().c_str());
}

FPDRESULT IRayFpdAdapter::setSyncMode(Enm_FluroSync syncMode)
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    FPDRESULT ret = m_pDetector->SetAttr(Attr_UROM_FluroSync_W, syncMode);
    if (Err_OK != ret) {
        return ret;
    }
    return m_pDetector->Invoke(Cmd_WriteUserROM);
}

FPDRESULT IRayFpdAdapter::setTriggerMode(Enm_TriggerMode triggerMode)
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    FPDRESULT ret = m_pDetector->SetAttr(Attr_UROM_TriggerMode_W, triggerMode);
    if (Err_OK != ret) {
        return ret;
    }
    return m_pDetector->Invoke(Cmd_WriteUserROM);
}
/////////////////////////////////////////////////////////////////////////////////
FPDRESULT IRayFpdAdapter::setAttr(int attrId, int value)
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->SetAttr(attrId, value);
}

FPDRESULT IRayFpdAdapter::setAttr(int attrId, float value)
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->SetAttr(attrId, value);
}

FPDRESULT IRayFpdAdapter::setAttr(int attrId, const QString& value)
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->SetAttr(attrId, value.toStdString().c_str());
}

void IRayFpdAdapter::getAttr(int attrId, AttrResult &result)
{
    if(m_pDetector == nullptr) {
        return;
    }
    m_pDetector->GetAttr(attrId, result);
}

FPDRESULT IRayFpdAdapter::invoke(int cmdId)
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->Invoke(cmdId);
}

FPDRESULT IRayFpdAdapter::invoke(int cmdId, int value)
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->Invoke(cmdId, value);
}

FPDRESULT IRayFpdAdapter::invoke(int cmdId, int par1, int par2)
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->Invoke(cmdId, par1, par2);
}
FPDRESULT IRayFpdAdapter::invoke(int cmdId, int par1, int par2, int par3)
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->Invoke(cmdId, par1, par2, par3);
}

FPDRESULT IRayFpdAdapter::invoke(int cmdId, float value)
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->Invoke(cmdId, value);
}

FPDRESULT IRayFpdAdapter::invoke(int cmdId, const QString& value)
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->Invoke(cmdId, value.toStdString().c_str());
}

FPDRESULT IRayFpdAdapter::setCaliSubset(const QString& subDir)
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->SetCaliSubset(subDir.toStdString().c_str());
}


    /*
bool IRayFpdAdapter::generateGainDefectTemplate(IRayFPDAdapterFPDType type)
{

    do {
        m_hNextStep = CreateEvent(NULL, false, false, NULL);
        m_hErrorEvent = CreateEvent(NULL, false, false, NULL);
        HEVENT hEvents[2] = { m_hNextStep, m_hErrorEvent };
        int result;
        AttrResult attr;
        m_pDetector->GetAttr(Attr_DefectTotalFrames, attr);
        m_nEachGroupFrames = attr.nVal / m_nTotalGroup;

        if (Err_OK != (result = defectInit())) {
            qDebug()<<"DefectInit failed! err="<<m_pDetector->GetErrorInfo(result).c_str();
            break;
        }
        qDebug()<<"Make sure the difference bewteen real-gray-value and expected-gray-value is less than 10%%";

        while (m_nCurGroupIndex < m_nTotalGroup) {
            qDebug()<<"--------------------[group:"<<m_nCurGroupIndex + 1<<", expect gray:"<<m_ExpectedGrays[m_nCurGroupIndex]<<"]------------------";
            qDebug()<<"Press [Enter] to start collect light images";
            getchar();

            startLightAcqusition();
            qDebug()<<"Acquiring light...";

            Sleep(100);
            qDebug()<<"Press [Enter] to select valid images";
            getchar();

            m_nStartFrames = getValidFrames();
            select4ValidImages(m_nCurGroupIndex, m_nEachGroupFrames);

            m_timer.Init(TimeProc, 100);
            int wait = WaitForMultipleObjects(2, hEvents, false, WAIT_FOREVER);
            if (WAIT_OBJECT_0 + 1 == wait) {
                m_timer.Close();
                qDebug()<<"Press [Enter] to restart";
                getchar();
                continue;
            }

            qDebug()<<"Stop expose and press [Enter] to start collect dark images";
            getchar();

            m_nStartFrames = getValidFrames();
            Sleep(100);
            startDarkAcqusition();
            qDebug()<<"Acquiring dark...\n";
            s_timer.Init(TimeProc, 100);
            wait = WaitForSingleObject(m_hNextStep, WAIT_FOREVER);
            m_nCurGroupIndex += 1;
            Sleep(100);
        };
        qDebug()<<"Press [Enter] to generte template files";
        getchar();
        qDebug()<<"Generating ...\n";
        if (Err_OK == generateTemplateFiles()) {
            qDebug()<<"Generate gain+defect succeed\n";
        }
        else {
            qDebug()<<"Generate gain+defect failed\n";
        }

        finishGeneration();

    } while (false);



    return true;
}
*/




FPDRESULT IRayFpdAdapter::startLightAcqusition()
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }

    if (m_nCurGroupIndex < m_nTotalGroup) {
        m_acqMode = 1;
        m_gainDefectImgNum = 0;
        return m_pDetector->Invoke(Cmd_StartAcq);
    }
    return Err_Unknown;
}

FPDRESULT IRayFpdAdapter::startSelecting4ValidImages()
{
    if (m_nCurGroupIndex < m_nTotalGroup) {
//        m_nStartFrames = getValidFrames();
        m_acqMode = 1;
        m_gainDefectImgNum = 0;
        return select4ValidImages(m_nCurGroupIndex, m_nEachGroupFrames);
    }
    return -1;
}


//FPDRESULT IRayFpdAdapter::startSelectingValidFrames()
//{
//    if (m_nCurGroupIndex < m_nTotalGroup) {
//        m_nStartFrames = getValidFrames();
//        Sleep(100);
//        return startDarkAcqusition();
//    }
//    return Err_Unknown;
//}

FPDRESULT IRayFpdAdapter::startDarkAcqusition()
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    if (m_nCurGroupIndex < m_nTotalGroup) {
        m_acqMode = 2;
        m_gainDefectImgNum = 0;
        return m_pDetector->Invoke(Cmd_ForceDarkContinuousAcq, 0);
    }
    return Err_Unknown;

}

FPDRESULT IRayFpdAdapter::nextStep()
{
    m_nCurGroupIndex += 1;
    emit sig_didGetGroupImage(m_nCurGroupIndex, m_nTotalGroup);

    return Err_Unknown;
}

FPDRESULT IRayFpdAdapter::generateTemplateFiles()
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    FPDRESULT result = Err_Unknown;
    switch (m_fpdType) {
    case IRayFPDAdapterFPDTypeNDT:
    {
        result = m_pDetector->Invoke(Cmd_GainGeneration);
        break;
    }
    case IRayFPDAdapterFPDTypeDTDI:
    {
        result = m_pDetector->Invoke(Cmd_DefectGeneration);
        break;
    }
    default:
    {
        break;
    }
    }

    finishGeneration();

    return result;

}

FPDRESULT IRayFpdAdapter::startMakingNDTOffsetTemplate()
{
    m_mode = IRayFPDAdapterModeOffsetTemplate;
    m_fpdType = IRayFPDAdapterFPDTypeNDT;
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->Invoke(Cmd_OffsetGeneration);
}

FPDRESULT IRayFpdAdapter::startMakingDTDIOffsetTemplate()
{
    m_mode = IRayFPDAdapterModeOffsetTemplate;
    m_fpdType = IRayFPDAdapterFPDTypeDTDI;
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->Invoke(Cmd_OffsetGeneration);
}




FPDRESULT IRayFpdAdapter::startMakingNDTGainDefectTemplate()
{
    m_mode = IRayFPDAdapterModeGainDefectTemplate;
    m_fpdType = IRayFPDAdapterFPDTypeNDT;

//    m_nCurGroupIndex = 0;

    AttrResult attr;
    m_pDetector->GetAttr(Attr_GainTotalFrames, attr);
    m_nEachGroupFrames = attr.nVal / m_nTotalGroup;

    return gainInit();
}

FPDRESULT IRayFpdAdapter::startMakingDTDIGainDefectTemplate()
{
    m_mode = IRayFPDAdapterModeGainDefectTemplate;
    m_fpdType = IRayFPDAdapterFPDTypeDTDI;

//    m_nCurGroupIndex = 0;

    AttrResult attr;
    m_pDetector->GetAttr(Attr_DefectTotalFrames, attr);
    m_nEachGroupFrames = attr.nVal / m_nTotalGroup;

    return defectInit();
}

FPDRESULT IRayFpdAdapter::stopMakingGainDefectTemplate()
{
    m_mode = IRayFPDAdapterModeNormal;
    return stopAcquire();
}

FPDRESULT IRayFpdAdapter::gainInit()
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->Invoke(Cmd_GainInit);
}

FPDRESULT IRayFpdAdapter::defectInit()
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    return m_pDetector->Invoke(Cmd_DefectInit);
}


FPDRESULT IRayFpdAdapter::select4ValidImages(int groupindex, int eachGroupFrames)
{
    if(m_pDetector == nullptr) {
        return Err_NotInitialized;
    }
    switch (m_fpdType) {
    case IRayFPDAdapterFPDTypeNDT:
    {
        FPDRESULT result = m_pDetector->Invoke(Cmd_GainSelectAll, groupindex, eachGroupFrames);


        return result;
    }
    case IRayFPDAdapterFPDTypeDTDI:
    {
        FPDRESULT result = m_pDetector->Invoke(Cmd_DefectSelectAll, groupindex, eachGroupFrames);

        return result;
    }
    default:
    {
        return Err_Unknown;
    }
    }

    return Err_Unknown;
}


FPDRESULT IRayFpdAdapter::finishGeneration()
{
    return m_pDetector->Invoke(Cmd_FinishGenerationProcess);
}

FPDRESULT IRayFpdAdapter::getValidFrames()
{
    AttrResult attr;

    switch (m_fpdType) {
    case IRayFPDAdapterFPDTypeNDT:
    {
        m_pDetector->GetAttr(Attr_GainValidFrames, attr);
        break;
    }
    case IRayFPDAdapterFPDTypeDTDI:
    {
        m_pDetector->GetAttr(Attr_DefectValidFrames, attr);
        break;
    }
    default:
    {
        break;
    }
    }

    return attr.nVal;
}
