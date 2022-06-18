 #include "ExperimentConfigWidget.h"
#include "ui_ExperimentConfigWidget.h"
#include "ObjectEditWidget.h"

#include <ctime>

#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
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
    car_config["light_r"] = createIntegerEditEntryConfig("light_r", "视野半径", 0, 0, 10000);
    ui->carEdit->init(std::make_shared<ObjectEditConfig>(std::move(car_config)));
    connect(ui->carEdit, &ObjectEditWidget::addData, [this](ObjectData data) {
        auto index = data_.car_components_config.size();
        data_.car_components_config.push_back({});
        auto &config = data_.car_components_config.back();
        config.init_pos_r = data["init_pos_r"].toInt();
        config.init_pos_c = data["init_pos_c"].toInt();
        config.light_r = data["light_r"].toInt();
        ui->listWidgetCars->addItem(
            QString("小车#%1: 位置=(%2, %3), 视野=%4")
                    .arg(index)
                    .arg(config.init_pos_r)
                    .arg(config.init_pos_c)
                    .arg(config.light_r));
    });
    connect(ui->carEdit, &ObjectEditWidget::confirmData, [this](ObjectData data) {
        auto index = current_editing_car_index_;
        if (index == -1) return;
        auto &config = data_.car_components_config[index];
        config.init_pos_r = data["init_pos_r"].toInt();
        config.init_pos_c = data["init_pos_c"].toInt();
        config.light_r = data["light_r"].toInt();
        ui->listWidgetCars->item(index)->setText(
                QString("小车#%1: 位置=(%2, %3), 视野=%4")
                        .arg(index)
                        .arg(config.init_pos_r)
                        .arg(config.init_pos_c)
                        .arg(config.light_r));
    });
    connect(ui->listWidgetCars, &QListWidget::currentRowChanged, [this](int row) {
        auto &config = data_.car_components_config[row];
        ObjectData data;
        data["init_pos_r"] = config.init_pos_r;
        data["init_pos_c"] = config.init_pos_c;
        data["light_r"] = config.light_r;
        ui->carEdit->setData(data);
        current_editing_car_index_ = row;
    });

    QStringList items_text;
    items_text << "demo" << "A*" << "DFS" << "BFS";
    ui->comboBoxNaviAlgo->addItems(items_text);
    connect(ui->listWidgetNavigators, &QListWidget::currentRowChanged, [this](int row) {
        ui->comboBoxNaviAlgo->setCurrentText(data_.navigator_components_config[row].plugin_id.c_str());
        current_editing_navi_index_ = row;
    });
    connect(ui->comboBoxNaviAlgo, &QComboBox::currentTextChanged, [this](const QString &) {
        if (current_editing_navi_index_ != -1) {
            auto &config = data_.navigator_components_config[current_editing_navi_index_];
            config.plugin_id = ui->comboBoxNaviAlgo->currentText().toStdString();
            ui->listWidgetNavigators->item(current_editing_navi_index_)->setText(
                        QString("导航器#%1: %2").arg(current_editing_navi_index_).arg(config.plugin_id.c_str()));
        }
    });
}

ExperimentConfigWidget::~ExperimentConfigWidget() {
    delete ui;
}

std::string ExperimentConfigWidget::getData() {
    return xpack::json::encode(getInteralData());
}

void ExperimentConfigWidget::showData() {
    clearShow();
    ui->spinBoxFPS->setValue(data_.fps);

    auto &cars_config = data_.car_components_config;
    auto &navigators_config = data_.navigator_components_config;

    for (std::size_t i = 0; i < cars_config.size(); ++i) {
        const auto &config = cars_config[i];
        ui->listWidgetCars->addItem(
                QString("小车#%1: 位置=(%2, %3), 视野=%4")
                        .arg(i)
                        .arg(config.init_pos_r)
                        .arg(config.init_pos_c)
                        .arg(config.light_r));
    }

    for (std::size_t i = 0; i < navigators_config.size(); ++i) {
        const auto &config = navigators_config[i];
        ui->listWidgetNavigators->addItem(
                    QString("导航器#%1: %2").arg(i).arg(config.plugin_id.c_str()));
    }
}

void ExperimentConfigWidget::clearShow() {
    ui->listWidgetCars->clear();
    ui->listWidgetNavigators->clear();
}

bool ExperimentConfigWidget::checkData() {
    // Bad proctice: cross gui and check-logic
    QString msg;
    int cnt = 0;

#define POS_CHK_MSG QString("小车%1初始位置(行:%2, 列:%3)超出边界(高:%4, 宽:%5)\n") \
    .arg(cnt).arg(car.init_pos_r).arg(car.init_pos_c).arg(data_.map_config.size.height).arg(data_.map_config.size.width)

    if (data_.navigator_components_config.empty()) {
        msg.append("至少要包括一个导航器\n");
    }

    for (const auto &car : data_.car_components_config) {
        ++cnt;
        if (car.init_pos_r < 0 || car.init_pos_r > data_.map_config.size.height)
            msg.append(POS_CHK_MSG);
        else if (car.init_pos_c < 0 || car.init_pos_c > data_.map_config.size.width)
            msg.append(POS_CHK_MSG);
    }

    if (!msg.isEmpty()) QMessageBox::warning(this, "配置非法", msg);
    return msg.isEmpty();
}

bool ExperimentConfigWidget::setDataFromJSON(const QJsonValue &json) {
    Q_ASSERT(json.isObject());
    auto json_obj = json.toObject();
    QJsonDocument json_doc(json_obj);
    ExperimentConfig data;
    try {
        xpack::json::decode(json_doc.toJson().data(), data);
        data_ = data;
    } catch (const std::exception &e) {
        return false;
    }
    return true;
}

void ExperimentConfigWidget::setData(const ExperimentConfig &data) {
    data_ = data;
}

const ExperimentConfig &ExperimentConfigWidget::getInteralData() {
    // Get & Set FPS
    data_.fps = ui->spinBoxFPS->value();
    return data_;
}

void ExperimentConfigWidget::setAuthToken(const std::string & token) {
    // Set random token (time)
    data_.auth_token = token;
}

void ExperimentConfigWidget::on_pushButtonImportMap_clicked() {
    auto file_name = QFileDialog::getOpenFileName(this, "导入地图", QString(), "地图(*.json)");
    if (file_name.isNull()) return;

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

void ExperimentConfigWidget::on_pushButton_clicked() {
    auto index = data_.navigator_components_config.size();
    data_.navigator_components_config.push_back({});
    auto &config = data_.navigator_components_config.back();
    config.plugin_id = ui->comboBoxNaviAlgo->currentText().toStdString();
    ui->listWidgetNavigators->addItem(
                QString("导航器#%1: %2").arg(index).arg(config.plugin_id.c_str()));
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
    cfg.label = label;
    cfg.type = QVariant::Int;
    cfg.getDefultValueCallback = [defaultValue]() -> QVariant {
       return defaultValue;
    };
    cfg.initEditCallback = [](QWidget *parent) -> QWidget * {
       return new QSpinBox(parent);
    };
    cfg.setValueCallback = [](QWidget *w, const QVariant &val) {
       Q_ASSERT(w && dynamic_cast<QSpinBox*>(w));
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

void ExperimentConfigWidget::on_pushButtonExport_clicked() {
    auto file_name = QFileDialog::getSaveFileName(this, {}, {}, "配置文件(*.json)");
    QFile file(file_name);
    bool ok = file.open(QIODevice::WriteOnly);
    if (!ok) {
        QMessageBox::warning(this, "错误", QString("打开%1失败").arg(file_name));
        return;
    }

    auto json = xpack::json::encode(getInteralData());
    auto len = file.write(json.c_str(), json.size());
    if (len == -1 || len != (qint64)json.size()) QMessageBox::warning(this, "错误", QString("文件%1写入失败, 请重试").arg(file_name));
}

void ExperimentConfigWidget::on_pushButtonImport_clicked() {
    auto file_name = QFileDialog::getOpenFileName(this, {}, {}, "配置文件(*.json)");
    if (file_name.isNull()) return;
    QFile file(file_name);
    bool ok = file.open(QIODevice::ReadOnly);
    if (!ok) {
        QMessageBox::warning(this, "错误", QString("打开%1失败").arg(file_name));
        return;
    }

    auto json = file.readAll();
    try {
        xpack::json::decode(json.toStdString(), data_);
        QMessageBox::information(this, "解析成功",
            QString("地图: %1x%2; 小车数量: %3; 导航器数量: %4")
                         .arg(data_.map_config.size.width)
                         .arg(data_.map_config.size.height)
                         .arg(data_.car_components_config.size())
                         .arg(data_.navigator_components_config.size()));
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "错误", QString("配置文件%1解析失败\n%2")
                             .arg(file_name).arg(e.what()));
    }
    showData();
}
