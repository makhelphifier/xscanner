#include "logwidget.h"
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QMenu>
#include <QActionGroup>
#include <QColor>
#include <QTextCharFormat>
#include <QTextBlock>

LogWidget::LogWidget(QWidget *parent) : QWidget(parent)
{
    m_logDisplay = new QPlainTextEdit(this);
    m_logDisplay->setReadOnly(true);
    m_logDisplay->document()->setMaximumBlockCount(2000);
    m_logDisplay->setStyleSheet("background-color: white; color: black; font-family: 'Microsoft YaHei'; font-size: 12pt;"); // 设置白底黑字 + 微软雅黑 + 12pt字号
    m_logDisplay->setContextMenuPolicy(Qt::CustomContextMenu);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_logDisplay);
    setLayout(layout);

    // 创建右键菜单
    m_contextMenu = new QMenu(this);
    m_contextMenu->addAction("清空日志", this, &LogWidget::clearLogs);
    m_levelMenu = m_contextMenu->addMenu("日志级别");
    createLogLevelMenu();

    connect(m_logDisplay, &QPlainTextEdit::customContextMenuRequested, this, [this](const QPoint &pos){
        m_contextMenu->exec(m_logDisplay->mapToGlobal(pos));
    });
}
void LogWidget::appendLogMessage(const QString &text, int level)
{
    // 过滤：如果传入 level < 当前过滤级别，则不显示
    if (level < m_currentLogLevel.toInt()) {
        return;  // 跳过低级别日志
    }

    QColor color;
    switch (level) {
    case Log4Qt::Level::DEBUG_INT:
        color = QColor("#333333");  // DEBUG: 深灰色，高对比
        break;
    case Log4Qt::Level::INFO_INT:
        color = QColor("#006400");  // INFO: 深绿色，醒目且柔和
        break;
    case Log4Qt::Level::WARN_INT:
        color = QColor("#FF8C00");  // WARN: 橙色，替换黄色，高可见度
        break;
    case Log4Qt::Level::ERROR_INT:
        color = Qt::red;            // ERROR: 红色，保持原样
        break;
    case Log4Qt::Level::FATAL_INT:
        color = QColor("#8B0000");  // FATAL: 深红色，更突出
        break;
    default:
        color = Qt::black;          // 其他默认为黑色
        break;
    }

    QTextCharFormat format;
    format.setForeground(color);
    m_logDisplay->setCurrentCharFormat(format);
    m_logDisplay->appendPlainText(text.trimmed());
}


void LogWidget::clearLogs()
{
    m_logDisplay->clear();
}


void LogWidget::createLogLevelMenu()
{
    QActionGroup *levelGroup = new QActionGroup(this);
    levelGroup->setExclusive(true);

    auto addLevelAction = [&](const QString &name, Log4Qt::Level level) {
        QAction *action = new QAction(name, this);
        action->setCheckable(true);
        action->setData(QVariant::fromValue(level));
        m_levelMenu->addAction(action);
        levelGroup->addAction(action);
    };

    addLevelAction("DEBUG", Log4Qt::Level::DEBUG_INT);
    addLevelAction("INFO", Log4Qt::Level::INFO_INT);
    addLevelAction("WARNING", Log4Qt::Level::WARN_INT);
    addLevelAction("ERROR", Log4Qt::Level::ERROR_INT);
    addLevelAction("CRITICAL", Log4Qt::Level::FATAL_INT);

    connect(levelGroup, &QActionGroup::triggered, this, [this](QAction *action){
        Log4Qt::Level selectedLevel = action->data().value<Log4Qt::Level>();
        m_currentLogLevel = selectedLevel;
        emit logLevelChanged(selectedLevel);
    });

    updateLogLevelCheckState(Log4Qt::Level::ALL_INT);
}

void LogWidget::updateLogLevelCheckState(Log4Qt::Level level)
{
    for(QAction* action : m_levelMenu->actions()) {
        if (action->data().value<Log4Qt::Level>() == level) {
            action->setChecked(true);
            break;
        }
    }
}


void LogWidget::appendLogMessage(const QString &message)
{
    if (m_logDisplay)
    {
       m_logDisplay->appendPlainText(message);
    }
}
