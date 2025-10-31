#ifndef MOTIONHELPER_H
#define MOTIONHELPER_H
#include <QtCore>
#include <QtGui>
//#include <QtNetwork>

#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    #include <QtWidgets>
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    #include <QtCore5Compat>
#endif

#define TIMEMS qPrintable(QTime::currentTime().toString("HH:mm:ss zzz"))
#define STRDATETIME qPrintable(QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss"))

#define STRDATETIME1 qPrintable(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"))


class MotionHelper
{
public:
    //程序文件名称和当前所在路径
    static QString appName();
    static QString appPath();

    //获取uuid
    static QString getUuid();

    //校验目录
    static void checkPath(const QString& dirName);

    //为什么还要自定义对话框因为可控宽高和汉化对应文本等
    //初始化对话框文本
    static void initDialog(QFileDialog* dialog, const QString& title, const QString& acceptName,
                           const QString& dirName, bool native, int width, int height);
    //拿到对话框结果
    static QString getDialogResult(QFileDialog* dialog);

    //选择文件对话框
    static QString getOpenFileName(const QString& filter = QString(),
                                   const QString& dirName = QString(),
                                   const QString& fileName = QString(),
                                   bool native = false, int width = 900, int height = 600);
    //保存文件对话框
    static QString getSaveFileName(const QString& filter = QString(),
                                   const QString& dirName = QString(),
                                   const QString& fileName = QString(),
                                   bool native = false, int width = 900, int height = 600);
    //选择目录对话框
    static QString getExistingDirectory(const QString& dirName = QString(),
                                        bool native = false, int width = 900, int height = 600);

    //检查ini配置文件
    static bool checkIniFile(const QString& iniFile);

    //检查json文件
    static bool checkJsonFile(const QString& jsonFile);
};

#endif // MOTIONHELPER_H
