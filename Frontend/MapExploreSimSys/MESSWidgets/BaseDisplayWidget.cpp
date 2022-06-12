#include "BaseDisplayWidget.h"

#include <QPainter>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

BaseDisplayWidget::BaseDisplayWidget(QWidget *parent) : QWidget(parent) {

}

void BaseDisplayWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);

    for (const auto &order : rendering_orders_) {
        const auto &[op, args] = order;
        if (op == RenderOrder::Code::CLEAR) (void(0));
        else if (op == RenderOrder::Code::DRAW) {
            if (args.size() == 6) {
                int resID = args[0];
                auto iter = img_cache_.find(resID);
                if (iter != img_cache_.end()) {
                    QRect area;
                    area.setX(args[1]);      // lt-x
                    area.setY(args[2]);      // lt-y
                    area.setWidth(args[3]);  // w
                    area.setHeight(args[4]); // h
                    int theta = args[5];     // theta
                    if (resID == 0) { // 0 is background
                        double widthFactor = this->width() / (double)area.width();
                        double heightFactor = this->height() / (double)area.height();
                        expend_factor_ = std::min(widthFactor, heightFactor);
                    }
                    QRect trueArea;
                    trueArea.setX(area.x() * expend_factor_);
                    trueArea.setY(area.y() * expend_factor_);
                    trueArea.setWidth(area.width() * expend_factor_);
                    trueArea.setHeight(area.height() * expend_factor_);

                    const double width = trueArea.width();
                    const double height = trueArea.height();
                    const double center_xpos = trueArea.x() + width / 2.f;
                    const double center_ypos = trueArea.y() + height / 2.f;
                    p.save();
                    p.translate(center_xpos, center_ypos);
                    p.rotate(theta / 1000.0 + 90.0);
                    p.translate(-center_xpos, -center_ypos);
                    p.drawImage(trueArea, iter->second, iter->second.rect());
                    p.restore();
                } else if (resID != -1) {
                    qWarning("Resource %d not found", resID);
                }
            }
        }
    }
    rendering_orders_.clear();
}

void BaseDisplayWidget::addResource(int id, const QImage &img) {
    if (img_cache_.count(id)) {
        qInfo("Multi add resource %d", id);
    }
    img_cache_[id] = std::move(img);
}

void BaseDisplayWidget::display(const std::vector<RenderOrder> &orders) {
    rendering_orders_ = std::move(orders);
    this->update(); // trigger painEvent(...)
}

// RenderOrder
void RenderOrder::fromJson(const QJsonValue & json) {
    // Unsafe
    Q_ASSERT(json.isObject());
    auto obj = json.toObject();
    Q_ASSERT(obj.contains("op"));
    Q_ASSERT(obj.contains("args"));

    auto op = obj["op"].toString();
    for (std::size_t i = 0; i < std::size(CODE2STRING); ++i) {
        if (op == CODE2STRING[i]) {
            this->code = (Code)i;
            break;
        }
    }

    auto args = obj["args"].toArray();
    for (const auto &arg : args) {
        this->args.push_back(arg.toInt());
    }
}
