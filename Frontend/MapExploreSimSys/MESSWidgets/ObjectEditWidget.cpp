#include "ObjectEditWidget.h"

#include <QDebug>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialogButtonBox>

ObjectEditWidget::ObjectEditWidget(QWidget *parent) : QWidget(parent) {

}

void ObjectEditWidget::init(std::shared_ptr<ObjectEditConfig> config, std::size_t msgBufSize) {
    Q_ASSERT(config);

    if (msgBufSize != kDefaultInfoBufSize)
        m_infoBuf.recap(msgBufSize);
    QVBoxLayout *layout = new QVBoxLayout;
    QLabel *msgLabel = new QLabel;
    for (auto &cfg : *config) {
        auto oldCallback = cfg.checkCallback;
        cfg.checkCallback = [this, oldCallback, msgLabel](QVariant val) {
            auto errMsg = oldCallback(val);
            if (errMsg) {
                m_infoBuf.push_back(errMsg);
                QString info;
                for (const auto &e : m_infoBuf) {
                    info.append(e).append('\n');
                }
                msgLabel->setText(info);
            }

            return nullptr;
        };
        auto *entryWidget = new ObjectEntryEditWidget(this);
        entryWidget->init(&cfg);
        layout->addWidget(entryWidget);
        Q_ASSERT(cfg.entryWidget);
    }

    auto *buttonBox = new QDialogButtonBox(Qt::Horizontal);
    auto *okBtn = buttonBox->addButton(QDialogButtonBox::Ok);
    // auto *cancelBtn = buttonBox->addButton(QDialogButtonBox::Cancel);
    connect(okBtn, &QPushButton::clicked, [this]() {
        //for (auto &cfg : *this->m_config) {
        //    const auto [val, errMsg] = cfg.entryWidget->getValue();
        //    if (errMsg) qDebug("%s: [Error]%s", cfg.label.toStdString().c_str(), errMsg);
        //    else qDebug("%s: %d", cfg.label.toStdString().c_str(), val.toInt());
        //}
        emit confirmData(getData());
    });
    // connect(cancelBtn, &QPushButton::clicked, [this]() { this->close(); });

    layout->addWidget(buttonBox);
    layout->addWidget(msgLabel);
    this->setLayout(layout);
    this->m_config = config;
}

ObjectData ObjectEditWidget::getData() {
    ObjectData data;
    for (auto &cfg : *m_config) {
        const auto [val, err] = cfg.entryWidget->getValue();
        if (err) data[cfg.id] = cfg.getDefultValueCallback();
        else data[cfg.id] = val;
    }
    return data;
}
