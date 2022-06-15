#ifndef OBJECTEDITWIDGET_H
#define OBJECTEDITWIDGET_H

#include <QWidget>
#include <queue>
#include <memory>

#include "utils.h"
#include "ObjectEntryEditWidget.h"

class ObjectEditWidget : public QWidget {
    Q_OBJECT
public:
    static constexpr std::size_t kDefaultInfoBufSize = 4;

    explicit ObjectEditWidget(QWidget *parent = nullptr);
    void init(std::shared_ptr<ObjectEditConfig> config, std::size_t msgBufSize = kDefaultInfoBufSize);

    void setData(const ObjectData &data);
    ObjectData getData();
signals:
    void confirmData(ObjectData data);
    void addData(ObjectData data);
private:
    std::shared_ptr<ObjectEditConfig> m_config{nullptr};
    CircularVector<QString> m_infoBuf{kDefaultInfoBufSize};
};

#endif // !OBJECTEDITWIDGET_H
