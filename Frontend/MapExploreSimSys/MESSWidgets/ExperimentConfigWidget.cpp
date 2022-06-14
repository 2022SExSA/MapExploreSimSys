#include "ExperimentConfigWidget.h"
#include "ui_ExperimentConfigWidget.h"
#include "ObjectEditWidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSpinBox>

static ObjectEntryEditConfig createIntegerEditEntryConfig(
    const QString &id,
    const QString &label,
    qlonglong defaultValue,
    qlonglong min,
    qlonglong max
);

ExperimentConfigWidget::ExperimentConfigWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ExperimentConfigWidget) {
    ui->setupUi(this);

    ObjectEditConfig car_config;
    car_config["init_pos_r"] = createIntegerEditEntryConfig("init_pos_r", "初始行", 0, 0, 10000);
    car_config["init_pos_c"] = createIntegerEditEntryConfig("init_pos_c", "初始列", 0, 0, 10000);
    car_config["lignt_r"] = createIntegerEditEntryConfig("lignt_r", "视野半径", 0, 0, 10000);
    ui->carEdit->init(std::make_shared<ObjectEditConfig>(std::move(car_config)));

//    ObjectEditConfig navi_config;
//    navi_config["plugin_id"] = createStringEditEntryConfig("plugin_id", "初始行", "", );
//    ui->carEdit->init(std::make_shared<ObjectEditConfig>(std::move(navi_config)));
}

ExperimentConfigWidget::~ExperimentConfigWidget() {
    delete ui;
}

void ExperimentConfigWidget::on_pushButtonImportMap_clicked() {
    auto file_name = QFileDialog::getOpenFileName(this, "导入地图", QString(), "地图(*.json)");
    QFile file(file_name);
    bool ok = file.open(QIODevice::ReadOnly);
    if (!ok) {
        QMessageBox::warning(this, "错误", "打开错误");
        return;
    }

    auto map_str = file.readAll().toStdString();
    try {
        xpack::json::decode(map_str, data_.map_config);
        QMessageBox::information(this, "提示", "解析成功");
    } catch (std::exception &s) {
        QMessageBox::warning(this, "错误", QString("地图文件不合法!\n%1").arg(s.what()));
    }
}

// Utils to create ObjectEditWidget
static ObjectEntryEditConfig createIntegerEditEntryConfig(
    const QString &id,
    const QString &label,
    qlonglong defaultValue,
    qlonglong min,
    qlonglong max
) {
    ObjectEntryEditConfig cfg;
    cfg.id = id;
    cfg.label = id;
    cfg.type = QVariant::LongLong;
    cfg.getDefultValueCallback = [defaultValue]() -> QVariant {
       return defaultValue;
    };
    cfg.initEditCallback = [](QWidget *parent) -> QWidget * {
       return new QSpinBox(parent);
    };
    cfg.setValueCallback = [](QWidget *w, const QVariant &val) {
       Q_ASSERT(w && dynamic_cast<QSpinBox*>(w));
       Q_ASSERT(val.type() == QVariant::LongLong);
       QSpinBox *s = dynamic_cast<QSpinBox*>(w);
       s->setValue(val.toLongLong());
    };
    cfg.getValueCallback = [](QWidget *w) -> QVariant {
       Q_ASSERT(w && dynamic_cast<QSpinBox*>(w));
       QSpinBox *s = dynamic_cast<QSpinBox*>(w);
       return s->value();
    };
    cfg.checkCallback = [label, min, max](const QVariant &val) -> const char * {
        static QString msg = QString("%1应该在%2~%3间").arg(label).arg(min).arg(max);
        auto i = val.toLongLong();
        if (i < min || i > max) {
           return msg.toStdString().c_str();
        }
        return nullptr;
    };

    return cfg;
}
