
#include "xraycontroller.h"
#include <QAxObject>
#include <QDebug>
#include <QVariant> // for QVariant

XrayController::XrayController(QObject *parent)
    : QObject(parent), m_loader(nullptr), m_tubeInterface(nullptr)
{
}

XrayController::~XrayController()
{
    // QAxObject 会在析构时自动释放 COM 接口
    delete m_tubeInterface;
    delete m_loader;
}

void XrayController::initialize()
{
    // 1. 创建 TubeLoader
    m_loader = new QAxObject("XRAYWorXBaseCOM.TubeLoaderCOM");
    if (m_loader->isNull()) {
        qWarning() << "XrayController: Failed to create TubeLoaderCOM. Is XRAYWorXBaseCOM.dll registered?";
        return;
    }

    // 2. 获取默认 IP
    QAxObject* defaultIp = m_loader->property("DefaultIpAddress").value<QAxObject*>();
    if (!defaultIp) {
        qWarning() << "XrayController: Failed to get DefaultIpAddress property.";
        return;
    }
    QString ip = defaultIp->property("Ip").toString();
    delete defaultIp;

    // 3. 获取 TubeInterface
    QVariant tubeInterfaceVariant = m_loader->dynamicCall("GetTubeInterface(QString)", ip);
    m_tubeInterface = tubeInterfaceVariant.value<QAxObject*>();

    if (!m_tubeInterface || m_tubeInterface->isNull()) {
        qWarning() << "XrayController: Failed to get TubeInterface for IP:" << ip;
        if(m_tubeInterface) delete m_tubeInterface;
        m_tubeInterface = nullptr;
        return;
    }

    // 4. 连接 COM 事件
    // 当 COM 接口初始化完成时，会触发 OnInitialized() 信号
    connect(m_tubeInterface, SIGNAL(OnInitialized()), this, SLOT(onComInitialized()));
    qDebug() << "XrayController: Waiting for COM interface initialization...";

    // 检查是否已经初始化
    if(m_tubeInterface->property("IsInitialized").toBool()) {
        onComInitialized();
    }
}

void XrayController::onComInitialized()
{
    qDebug() << "XrayController: COM Interface Initialized!";
    // (我们将在下一步中添加更多的事件处理器)
    // setupComEventHandlers();
}

// 这是我们发送 "X-Ray On" 命令的核心
void XrayController::turnXrayOn()
{
    if (!m_tubeInterface || !m_tubeInterface->property("IsInitialized").toBool()) {
        qWarning() << "XrayController::turnXrayOn() called, but COM is not ready.";
        return;
    }

    // 对应手册 3.5: pTubeInterface->XRayOn->PcDemandValue = true;
    QAxObject* xrayOn = m_tubeInterface->property("XRayOn").value<QAxObject*>();
    if (xrayOn) {
        qDebug() << "XrayController: Sending X-Ray ON command...";
        xrayOn->setProperty("PcDemandValue", true);
        delete xrayOn; // 释放属性对象
    } else {
        qWarning() << "XrayController: Failed to get 'XRayOn' property.";
    }
}



