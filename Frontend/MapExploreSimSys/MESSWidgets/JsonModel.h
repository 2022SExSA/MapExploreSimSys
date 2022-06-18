#ifndef JSONMODEL_H
#define JSONMODEL_H

#include "qjsonmodel.h"
#include "dll_export.h"

//    QJsonModel * model = new QJsonModel;
//    QTreeView * view = new QTreeView;
//    view->setModel(model);
//    model->load("example.json")

class JsonModel : public QJsonModel {
public:
    using QJsonModel::QJsonModel;
};

#endif // JSONMODEL_H
