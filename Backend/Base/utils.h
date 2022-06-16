#ifndef MESSBASE_UTILS_H
#define MESSBASE_UTILS_H

#include "pg/pgutil.h"

#include "fwd.h"
#include "config.h"
#include "SimpleAmqpClient/Channel.h"

#include <cstddef>
#include <dlfcn.h>
#include <sstream>
#include <unistd.h>
MESSBASE_NAMESPACE_START

template <typename T>
struct Vec2 {
    T x{0};
    T y{0};

#define DEINFE_OP_ASS(op) \
    template <typename D> \
    Vec2 &operator op (const Vec2<D>& a) { \
        x op a.x; \
        y op a.y; \
        return *this; \
    }
    DEINFE_OP_ASS(+=)
    DEINFE_OP_ASS(-=)
#undef DEINFE_OP_ASS

#define DEINFE_OP_ASS(op) \
    template <typename D> \
    Vec2 &operator op (D a) { \
        x op a; \
        y op a; \
        return *this; \
    }
    DEINFE_OP_ASS(*=)
    DEINFE_OP_ASS(/=)
#undef DEINFE_OP_ASS
};

template <typename T>
using Point = Vec2<T>;

template <typename T>
struct Rect {
    Point<T> top_left{};
    T width{0};
    T height{0};

    Point<T> center() const {
        return Point<T>{top_left} += Point<T>{width / 2, height / 2};
    }

    template <typename V>
    bool inner(const Rect<V> &other) const {
        if (top_left.x < other.top_left.x) return false;
        if (top_left.y < other.top_left.y) return false;
        if (top_left.x + width > other.top_left.x + other.width) return false;
        if (top_left.y + height > other.top_left.y + other.height) return false;
        return true;
    }

    template <typename V>
    bool surrounded(const Rect<V> &other) const {
        return other.inner(*this);
    }

    template <typename V>
    bool intersect(const Rect<V> &other) const {
        if (top_left.y > other.top_left.y + other.height) return false;
        if (top_left.x > other.top_left.x + other.width) return false;
        if (other.top_left.y > top_left.y + height) return false;
        if (other.top_left.x > top_left.x + width) return false;
        return true;
    }
};

template <typename T>
struct AABB { // Oriented Bounding Box
    Rect<T> area_{};
    T theta_{0}; // with posi-x, only 0, 90, 180, 270

    Rect<T> to_rect() const {
        int i = (int)theta_;
        Rect<T> res = area_;
        if (i == 0 || i == 180) {
            res.top_left.x += (area_.width - area_.height) / 2;
            res.top_left.y -= (area_.width - area_.height) / 2;
            std::swap(res.width, res.height);
        } else if (i == 90 || i == 270) {
        } else {
            MESS_LOG("ERR: i={0}", i);
            PGZXB_DEBUG_ASSERT(false);
        }

        return res;
    }

    template <typename V>
    bool intersect(const AABB<V> &other) const {
        return to_rect().intersect(other.to_rect());
    }
};

template <typename T>
inline Vec2<T> axis_theta_to_unit_vec2(double theta) {
    int i = (int)theta;
    if (i == 0) return {1, 0};
    if (i == 90) return {0, 1};
    if (i == 180) return {-1, 0};
    if (i == 270) return {0, -1};
    PGZXB_DEBUG_ASSERT(false);
    return {};
}

template <typename F, typename R, typename ...Args>
class FuncRef {
public:
    using Func = R(Args...);

    FuncRef(F *f) : f_(f) {

    }

    FuncRef(const FuncRef &o) = default;
    FuncRef &operator= (const FuncRef &o) = default;

    R operator() (Args ... args) {
        return (*f_)(std::forward<Args>(args)...);
    }
private:
    F *f_{nullptr};
};

struct Op {
    std::string auth_token;
    std::string op;
    std::vector<std::string> args;

    std::string to_string() const {
        std::string result = auth_token;
        result += " ";
        result += op;
        for (const auto &arg : args) {
            result += " ";
            result += arg;
        }
        return result;
    }

    static std::optional<Op> from_string(const std::string &str) { // tk op arg0 arg1 ...
        std::istringstream iss(str);
        Op op;
        if (!(iss >> op.auth_token)) return std::nullopt;
        if (!(iss >> op.op)) return std::nullopt;
        
        std::string temp;
        while (iss >> temp) {
            op.args.push_back(std::move(temp));
        }

        return op;
    }
};

class Dll {
public:
    Dll(const std::string &path, bool *ok = nullptr) {
        if (ok) *ok = true;
        dll_handle_ = ::dlopen(path.c_str(), RTLD_LAZY);
        if (!dll_handle_ && ok) *ok = false;
        MESS_LOG_IF(!dll_handle_, "Dll {0} open failed", path);
    }

    ~Dll() {
        ::dlclose(dll_handle_);
    }

    template <typename Func>
    Func find_function(const std::string &symbol) {
        return (Func)::dlsym(dll_handle_, symbol.c_str());
    }
private:
    void *dll_handle_;
};

using NaviPlugin = Dll;

class C2Call {
public:
    C2Call() = default;

    C2Call(const MQConfig &config) :config_(config) {
        auto input_q = config_.mq_name + ".input";
        AmqpClient::Channel::OpenOpts opts;
        opts.host = config_.mq_host;
        opts.port = config_.mq_port;
        opts.auth = AmqpClient::Channel::OpenOpts::BasicAuth{config_.mq_username, config_.mq_password};
        channel_ = AmqpClient::Channel::Open(opts);

        channel_->DeclareExchange(input_q);
        channel_->DeclareQueue(input_q, false, true, false, false);
        channel_->BindQueue(input_q, input_q);
    }

    void input(const std::string &input) {
        auto input_q = config_.mq_name + ".input";
        channel_->BasicPublish(input_q, "", AmqpClient::BasicMessage::Create(input));
    }
private:
    MQConfig config_;
    AmqpClient::Channel::ptr_t channel_;
};

inline void launch_component(const std::string& file, const std::string &config_json) {
    if (::fork() == 0) {
        MESS_LOG("Launching {0}", file);
        ::execlp(file.c_str(), file.c_str(), config_json.c_str(), NULL);
        MESS_ERR("Launch {0} failed, config=[\n{1}\n]", file, config_json);
    }
}

inline Json make_response_json_data(int err_code, const Json &data) {
    // {
    //     "code": <code:int>,
    //     "msg": "msg",
    //     "data": json-object
    // }
    Json resp /* Json::object() */;
    resp["code"] = err_code;
    resp["msg"]  = "";/* err_code_to_zhCN_str(err_code); */
    resp["data"] = data;
    return resp;
}

inline Json make_response_json_data(int err_code, const std::string &msg, const Json &data) {
    // {
    //     "code": <code:int>,
    //     "msg": "msg",
    //     "data": json-object
    // }
    Json resp /* Json::object() */;
    resp["code"] = err_code;
    resp["msg"]  = msg;/* err_code_to_zhCN_str(err_code); */
    resp["data"] = data;
    return resp;
}

inline const char * get_navi_plugin_by_id(const std::string &id) {
    static const char *NAME_TABLE[4][2] = {
        {"demo", "/home/pgzxb/Documents/DevWorkspace/2022SACourseWorkspace/MapExploreSimSys/Backend/Build/Components/NaviPlugin/libmess_navi_plugin_demo.so"},
        {"A*",   "/home/pgzxb/Documents/DevWorkspace/2022SACourseWorkspace/MapExploreSimSys/Backend/Build/Components/NaviPlugin/libmess_navi_plugin_a_star.so"},
        {"DFS",  "/home/pgzxb/Documents/DevWorkspace/2022SACourseWorkspace/MapExploreSimSys/Backend/Build/Components/NaviPlugin/libmess_navi_plugin_dfs.so"},
        {"BFS",  "/home/pgzxb/Documents/DevWorkspace/2022SACourseWorkspace/MapExploreSimSys/Backend/Build/Components/NaviPlugin/libmess_navi_plugin_bfs.so"},
    };

    for (const auto &e : NAME_TABLE) {
        if (id == e[0]) {
            return e[1];
        }
    }

    return nullptr;
}

short get_available_port();

MESSBASE_NAMESPACE_END
#endif
