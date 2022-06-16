#ifndef DISPLAYWIDGET_H
#define DISPLAYWIDGET_H

#include <QWidget>
#include <unordered_map>

#include "dll_export.h"

struct RenderOrder {
    enum class Code : int {
        NOP, CLEAR, DRAW,
    };

    static constexpr const char *CODE2STRING[] = {
            "N", "C", "D", // nop, clear, draw
    };

    Code code{Code::NOP};
    std::vector<int> args;

    void fromJson(const QJsonValue &json);
};

class MESSWIDGETS_EXPORT BaseDisplayWidget : public QWidget {
    Q_OBJECT
public:
    using ImageMap = std::unordered_map<int, QImage>;

    explicit BaseDisplayWidget(QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *event) override;
public slots:
    void reset();
    void addResource(int id, const QImage &img);
    void display(const std::vector<RenderOrder> &orders);
private:
    ImageMap img_cache_;
    std::vector<RenderOrder> rendering_orders_;
    double expend_factor_{1.f};
};

#endif // DISPLAYWIDGET_H
