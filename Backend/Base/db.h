#ifndef MESSBASE_DB_H
#define MESSBASE_DB_H

#include "fwd.h"
#include "config.h"
MESSBASE_NAMESPACE_START

// Experiment Table:
//  user_id    started_at stoped_at   config_path   rendering_orders_path
//  VARCHAR(64) INT     INT     VARCHAR(512)   VARCHAR(512)   

namespace db_model {

struct Exeriment {
    std::time_t started_at{0};
    std::time_t stoped_at{0};
    std::string user_id;
    std::string config_path;
    std::string rendering_orders_path;

    XPACK(O(started_at, stoped_at, user_id, config_path, rendering_orders_path));
};

}; // namespace db_model

class DAO {
public:
    virtual void try_create_db() = 0;
    virtual void add_experiment(const db_model::Exeriment &e) = 0;
    virtual std::vector<db_model::Exeriment> get_all_experiments() = 0;

    static std::unique_ptr<DAO> create_sqlite_ins(const std::string &path);
};

MESSBASE_NAMESPACE_END
#endif // !MESSBASE_DB_H
