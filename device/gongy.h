#ifndef GONGY_H
#define GONGY_H

#include <QWidget>
#include <QTabWidget>

class QVBoxLayout;
class QTimer;
class QLabel;
class XraySource;
class QLineEdit;
class QCheckBox;
class QPushButton;

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
    void slotGetXrayStatus(bool warmup, bool locked, bool on, int kv, int ua);
    void slotXonClick();
    void slotXoffClick();
    void slotVoltageChanged();
    void slotCurrentChanged();
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
    bool m_online;

    QLineEdit* m_pVolCurrentLineEdit = nullptr;
    QLineEdit* m_pVolTargetLineEdit = nullptr;
    QLineEdit* m_pCurrentCurrentLineEdit = nullptr;
    QLineEdit* m_pCurrentTargetLineEdit = nullptr;
    QCheckBox* m_pXray = nullptr;
    QCheckBox* m_pWarmUp = nullptr;
    QCheckBox* m_pInterLock = nullptr;
    QCheckBox* m_pConnect = nullptr;
    QPushButton* m_pButtonXon = nullptr;
    QPushButton* m_pButtonXoff = nullptr;

    XraySource* m_pXraySource;
};
#endif // GONGY_H
