#ifndef OBJECTEDITENTRYWIDGET_H
#define OBJECTEDITENTRYWIDGET_H

#include <functional>

#include <QMainWindow>
#include <QVariant>

class QLabel;

class ObjectEntryEditWidget;
struct ObjectEntryEditConfig;

using ObjectData = QVariantMap;
using ObjectEditConfig = QMap<QString, ObjectEntryEditConfig>;
using GetDefultValueCallback = std::function<QVariant()>;
using CheckEntryValueValidityCallback = std::function<const char *(const QVariant&)>;

/** EditWidget
 * QVariant::String : QLineEdit
 * QVariant::Int    : QSpinBox
 * QVariant::Double : QDoubleSpinBox
*/
using InitEditWidgetCallback = std::function<QWidget*(QWidget*)>;
using SetValueOfEditWidgetCallback = std::function<void(QWidget*, const QVariant&)>;
using GetValueFromEditWidgetCallback = std::function<QVariant(QWidget*)>;

struct ObjectEntryEditConfig {
    QString id;
    QString label;
    QVariant::Type type{QVariant::Invalid};
    GetDefultValueCallback getDefultValueCallback{nullptr};
    CheckEntryValueValidityCallback checkCallback{nullptr};
    InitEditWidgetCallback initEditCallback{nullptr};
    SetValueOfEditWidgetCallback setValueCallback{nullptr};
    GetValueFromEditWidgetCallback getValueCallback{nullptr};
    ObjectEntryEditWidget *entryWidget{nullptr};
};

class ObjectEntryEditWidget : public QWidget {
    Q_OBJECT
public:
    explicit ObjectEntryEditWidget(QWidget *parent = nullptr);
    void init(ObjectEntryEditConfig *config);
    std::tuple<QVariant, const char*> getValue();
private:
    QLabel *m_label{nullptr};
    QWidget *m_editWdiget{nullptr};
    ObjectEntryEditConfig *m_config{nullptr};
};

#endif // OBJECTEDITENTRYWIDGET_H
