#include "apphelper.h"


QString AppHelper::appName()
{
    //没有必要每次都获取,只有当变量为空时才去获取一次
    static QString name;
    if (name.isEmpty())
    {
        name = qApp->applicationFilePath();
        //下面的方法主要为了过滤安卓的路径 lib程序名_armeabi-v7a/lib程序名_arm64-v8a
        QStringList list = name.split("/");
        name = list.at(list.count() - 1).split(".").at(0);
        name.replace("_armeabi-v7a", "");
        name.replace("_arm64-v8a", "");
    }

    return name;
}

QString AppHelper::appPath()
{
    static QString path;
    if (path.isEmpty())
    {
        path = qApp->applicationDirPath();
    }

    return path;
}

QString AppHelper::getUuid()
{
    QString uuid = QUuid::createUuid().toString();
    uuid.replace("{", "");
    uuid.replace("}", "");
    return uuid;
}

void AppHelper::checkPath(const QString& dirName)
{
    //相对路径需要补全完整路径
    QString path = dirName;
    if (path.startsWith("./"))
    {
        path.replace(".", "");
        path = AppHelper::appPath() + path;
    }
    else if (!path.startsWith("/") && !path.contains(":/"))
    {
        path = AppHelper::appPath() + "/" + path;
    }

    //目录不存在则新建
    QDir dir(path);
    if (!dir.exists())
    {
        dir.mkpath(path);
    }
}

void AppHelper::initDialog(QFileDialog* dialog, const QString& title, const QString& acceptName, const QString& dirName, bool native, int width, int height)
{
    //设置标题
    dialog->setWindowTitle(title);
    //设置标签文本
    dialog->setLabelText(QFileDialog::Accept, acceptName);
    dialog->setLabelText(QFileDialog::Reject, "取消(&C)");
    dialog->setLabelText(QFileDialog::LookIn, "查看");
    dialog->setLabelText(QFileDialog::FileName, "名称");
    dialog->setLabelText(QFileDialog::FileType, "类型");

    //设置默认显示目录
    if (!dirName.isEmpty())
    {
        dialog->setDirectory(dirName);
    }

    //设置对话框宽高
    if (width > 0 && height > 0)
    {
        dialog->setFixedSize(width, height);
    }

    //设置是否采用本地对话框
    dialog->setOption(QFileDialog::DontUseNativeDialog, !native);
    //设置只读可以取消右上角的新建按钮
    //dialog->setReadOnly(true);
}

QString AppHelper::getDialogResult(QFileDialog* dialog)
{
    QString result;
    if (dialog->exec() == QFileDialog::Accepted)
    {
        result = dialog->selectedFiles().first();
        if (!result.contains("."))
        {
            //自动补全拓展名 保存文件(*.txt *.exe)
            QString filter = dialog->selectedNameFilter();
            if (filter.contains("*."))
            {
                filter = filter.split("(").last();
                filter = filter.mid(0, filter.length() - 1);
                //取出第一个作为拓展名
                if (!filter.contains("*.*"))
                {
                    filter = filter.split(" ").first();
                    result = result + filter.mid(1, filter.length());
                }
            }
        }
    }
    return result;
}

QString AppHelper::getOpenFileName(const QString& filter, const QString& dirName, const QString& fileName, bool native, int width, int height)
{
    QFileDialog dialog;
    initDialog(&dialog, "打开文件", "选择(&S)", dirName, native, width, height);
    //设置文件类型
    if (!filter.isEmpty())
    {
        dialog.setNameFilter(filter);
    }

    //设置默认文件名称
    dialog.selectFile(fileName);
    return getDialogResult(&dialog);
}

QString AppHelper::getSaveFileName(const QString& filter, const QString& dirName, const QString& fileName, bool native, int width, int height)
{
    QFileDialog dialog;
    initDialog(&dialog, "保存文件", "保存(&S)", dirName, native, width, height);

    //设置文件类型
    if (!filter.isEmpty())
    {
        dialog.setNameFilter(filter);
    }

    //设置默认文件名称
    dialog.selectFile(fileName);
    //设置模态类型允许输入
    dialog.setWindowModality(Qt::WindowModal);
    //设置置顶显示
    dialog.setWindowFlags(dialog.windowFlags() | Qt::WindowStaysOnTopHint);
    return getDialogResult(&dialog);
}

QString AppHelper::getExistingDirectory(const QString& dirName, bool native, int width, int height)
{
    QFileDialog dialog;
    initDialog(&dialog, "选择目录", "选择(&S)", dirName, native, width, height);
    dialog.setOption(QFileDialog::ReadOnly);
    //设置只显示目录
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    return getDialogResult(&dialog);
}

bool AppHelper::checkIniFile(const QString& iniFile)
{
    //如果配置文件大小为0,则以初始值继续运行,并生成配置文件
    QFile file(iniFile);
    if (file.size() == 0)
    {
        return false;
    }

    //如果配置文件不完整,则以初始值继续运行,并生成配置文件
    if (file.open(QFile::ReadOnly))
    {
        bool ok = true;
        while (!file.atEnd())
        {
            QString line = file.readLine();
            line.replace("\r", "");
            line.replace("\n", "");
            QStringList list = line.split("=");

            if (list.count() == 2)
            {
                QString key = list.at(0);
                QString value = list.at(1);
                if (value.isEmpty())
                {
                    qDebug() << TIMEMS << "ini node no value" << key;
                    ok = false;
                    break;
                }
            }
        }

        if (!ok)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}

bool AppHelper::checkJsonFile(const QString& jsonFile)
{
    QFile file(jsonFile);
    if (file.size() == 0)
    {
        qDebug() << "00000000000";
        return false;
    }

    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray jsonData = file.readAll();
        file.close();

        QJsonParseError error;
        QJsonDocument::fromJson(jsonData, &error);

        if (error.error != QJsonParseError::NoError)
        {
            return false;
        }
    }
    return true;
}
