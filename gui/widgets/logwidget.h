#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QWidget>
#include "log4qt/level.h"

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
