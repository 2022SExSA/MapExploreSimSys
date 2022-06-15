#include "ObjectEntryEditWidget.h"

#include <QLabel>
#include <QHBoxLayout>

ObjectEntryEditWidget::ObjectEntryEditWidget(QWidget *parent) : QWidget(parent) {

}

void ObjectEntryEditWidget::init(ObjectEntryEditConfig *config) {
    Q_ASSERT(config);
    Q_ASSERT(config->type != QVariant::Invalid);
    Q_ASSERT(config->getDefultValueCallback);
    Q_ASSERT(config->initEditCallback);
    Q_ASSERT(config->checkCallback);

    m_config = config;
    m_label = new QLabel(config->label + ": ");
    m_editWdiget = config->initEditCallback(nullptr);
    m_config->entryWidget = this;
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_label);
    layout->addWidget(m_editWdiget);
    this->setLayout(layout);
    config->setValueCallback(m_editWdiget, config->getDefultValueCallback());
}

std::tuple<QVariant, const char*> ObjectEntryEditWidget::getValue() {
    Q_ASSERT(m_config);
    auto val = m_config->getValueCallback(m_editWdiget);
    auto errMsg = m_config->checkCallback(val);
    if (errMsg) return std::make_tuple(QVariant(), errMsg);
    return std::make_tuple(val, nullptr);
}

QWidget * ObjectEntryEditWidget::getEditWidget() {
    return m_editWdiget;
}

