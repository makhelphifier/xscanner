#ifndef GONGYU_H
#define GONGYU_H

#include <QWidget>
#include <QTabWidget>


class QVBoxLayout;
class QTimer;
class QLabel;
class GongYTabWidget : public QTabWidget
{
    Q_OBJECT
    Q_PROPERTY(float opacity READ opacity WRITE setOpacity)

public:
    explicit GongYTabWidget(QWidget *parent = nullptr);
    ~GongYTabWidget();

    float opacity() const;
    void setOpacity(float value);

    void setSlashMark(bool slashMark);

private slots:
    void onCurrentChanged(int index);
    void slotXrayWarning();
private:
    void setupTabs();
    QWidget* createFirstTab();
    QWidget* createSecondTab();

    void createXrayGroup(QVBoxLayout* pGroupLayout);
    void createDetectorGroup(QVBoxLayout* pGroupLayout);
    void createAcquireGroup(QVBoxLayout* pGroupLayout);

    float m_opacity;
    QFont m_font;
    QTimer* m_pTimer;
    QLabel* m_pLabel;
    bool m_slash;
};
#endif // GONGYU_H
