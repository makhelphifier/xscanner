#ifndef XRAYCONTROLLER_H
#define XRAYCONTROLLER_H

#include <QObject>

// 前向声明
class QAxObject;

class XrayController : public QObject
{
    Q_OBJECT
public:
    explicit XrayController(QObject *parent = nullptr);
    ~XrayController();

public slots:
    // 1. 初始化 COM
    void initialize();
    // 2. 发送 "X-Ray On" 命令
    void turnXrayOn();

private slots:
    // COM 事件：当 COM 初始化完成时
    void onComInitialized();

private:
    void setupComEventHandlers();

    QAxObject* m_loader;        // XRAYWorXBaseCOM.TubeLoaderCOM
    QAxObject* m_tubeInterface; // ITubeInterface
};

#endif // XRAYCONTROLLER_H








