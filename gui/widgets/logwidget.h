#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QWidget>
#include "log4qt/level.h"
#include <QString>
class QPlainTextEdit;
class QMenu;

class LogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LogWidget(QWidget *parent = nullptr);

public slots:
    void appendLogMessage(const QString &text, int level);
    void clearLogs();
public slots:
    /**
     * @brief 公共槽函数，用于接收日志消息并追加到内部的文本框中。
     * @param message 要追加的消息。
     */
    void appendLogMessage(const QString &message);
signals:
    void logLevelChanged(Log4Qt::Level level);

private:
    void createLogLevelMenu();
    void updateLogLevelCheckState(Log4Qt::Level level);

    QPlainTextEdit *m_logDisplay;
    QMenu *m_contextMenu;
    QMenu *m_levelMenu;
    Log4Qt::Level m_currentLogLevel = Log4Qt::Level::ALL_INT;
};

#endif // LOGWIDGET_H
