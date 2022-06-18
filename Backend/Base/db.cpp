#include "db.h"
#include "SQLiteCpp/Database.h"
#include "fwd.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <filesystem>

#ifdef SQLITECPP_ENABLE_ASSERT_HANDLER
namespace SQLite {
/// definition of the assertion handler enabled when SQLITECPP_ENABLE_ASSERT_HANDLER is defined in the project (CMakeList.txt)
void assertion_failed(const char* apFile, const long apLine, const char* apFunc, const char* apExpr, const char* apMsg) {
    // Print a message to the standard error output stream, and abort the program.
    std::cerr << apFile << ":" << apLine << ":" << " error: assertion failed (" << apExpr << ") in " << apFunc << "() with message \"" << apMsg << "\"\n";
    std::abort();
}
}
#endif

MESSBASE_NAMESPACE_START
namespace details {

// Experiment Table:
//  user_id    started_at stoped_at   config_path   rendering_orders_path
//  VARCHAR(64) INT     INT     VARCHAR(512)   VARCHAR(512)   

class SQLiteDAO : public DAO {
public:
    SQLiteDAO(const std::string &path)
        : path_(path), db_(path, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE) {
    }

    void try_create_db() override {
        try {
            if (!db_.tableExists("Experiment")) {
                db_.exec("CREATE TABLE Experiment ("
                            "user_id CHAR(64),"
                            "started_at INT,"
                            "stoped_at INT,"
                            "config_path VARCHAR(512),"
                            "rendering_orders_path VARCHAR(512)"
                        ")");
            } else {
                MESS_LOG("SQLite DB {0} exsits", path_);
            }
        } catch (const std::exception &e) {
            MESS_LOG("err={0}", e.what());
        }
        
    }

    void add_experiment(const db_model::Exeriment &e) override {
        try {
            SQLite::Statement query(db_, "INSERT INTO Experiment VALUES(?, ?, ?, ?, ?)");
            query.bind(1, e.user_id);
            query.bind(2, e.started_at);
            query.bind(3, e.stoped_at);
            query.bind(4, e.config_path);
            query.bind(5, e.rendering_orders_path);
            query.exec();
            MESS_LOG("err={0}", db_.getErrorMsg());
        } catch (const std::exception &e) {
            MESS_LOG("err={0}", e.what());
        }
    }

    std::vector<db_model::Exeriment> get_all_experiments() override {
        try {
            std::vector<db_model::Exeriment> result;
            SQLite::Statement query(db_, "SELECT * FROM Experiment");

            while (query.executeStep()) {
                db_model::Exeriment e;
                e.user_id = query.getColumn("user_id").getString();
                e.started_at = query.getColumn("started_at").getInt64();
                e.stoped_at = query.getColumn("stoped_at").getInt64();
                e.config_path = query.getColumn("config_path").getString();
                e.rendering_orders_path = query.getColumn("rendering_orders_path").getString();
                result.push_back(std::move(e));
            }

            return result;
        } catch (const std::exception &e) {
            MESS_LOG("err={0}", e.what());
        }
        return {};
    }
private:
    std::string path_;
    SQLite::Database db_;
};

};

std::unique_ptr<DAO> DAO::create_sqlite_ins(const std::string &path) {
    auto ins = std::unique_ptr<DAO>{new details::SQLiteDAO(path)};
    ins->try_create_db();
    return ins;
}

MESSBASE_NAMESPACE_END
