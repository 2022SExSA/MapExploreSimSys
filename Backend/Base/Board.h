#ifndef MESSBASE_BOARD_H
#define MESSBASE_BOARD_H

#include <hiredis/hiredis.h>
#include <hiredis/read.h>
#include <optional>

#include "fwd.h"
#include "utils.h"
MESSBASE_NAMESPACE_START
class Board {
public:
    ~Board() {
        redisFree(redis_ctx_);
    }

    void init(const std::string &auth_token, const std::string &ip, short port) {
        MESS_LOG("Initing Global (Redis)Board ip={0}, port={1}", ip, port);
        redis_ctx_ = redisConnect(ip.c_str(), port);
        MESS_ERR_IF(!redis_ctx_  || redis_ctx_->err, "Connect redis faile: errmsg=\"{0}\"", redis_ctx_->errstr);
        MESS_LOG("Board Inited", 1);
        this->auth_token_ = auth_token;
    }

    std::optional<Point<int>> get_next_routing_position(const std::string &car_name) {
        // rpop {auth-token}RoutList@{car-name}
        auto routlist_name = make_key(pgfmt::format(ROUTLIST_NAME_FROMAT, car_name));
        redisReply *reply = (redisReply*)redisCommand(redis_ctx_, "rpop %s", routlist_name.c_str());
        MESS_ERR_IF(!reply || reply->type == REDIS_REPLY_ERROR, "rpop {0} failed: errmsg={1}", routlist_name, reply->str);
        if (reply->type == REDIS_REPLY_STRING) {
            Json json = Json::parse(reply->str);
            PGZXB_DEBUG_ASSERT(json.is_object());
            return Point<int>{json["x"], json["y"]};
        }
        MESS_ERR_IF(reply->type != REDIS_REPLY_NIL, "rpop {0} failed", routlist_name);
        return std::nullopt;
    }

    long long add_position_to_routlist(const std::string &car_name, const Point<int> &pos) {
        return add_position_to_routlist(car_name, std::vector<Point<int>>{pos,});
    }

    long long add_position_to_routlist(const std::string &car_name, const std::vector<Point<int>> &pos_list) {
        if (pos_list.empty()) return -1;
        // lpush {auth-token}RoutList@{car-name} "{json1}" "{json2}" ...
        auto routlist_name = make_key(pgfmt::format(ROUTLIST_NAME_FROMAT, car_name));
        std::string pos_list_str;
        for (const auto &p : pos_list) {
            Json json = Json::object();
            json["x"] = p.x;
            json["y"] = p.y;
            pos_list_str.append("\"")
                        .append(json.dump())
                        .append("\" ");
        }
        redisReply *reply = (redisReply*)redisCommand(redis_ctx_, "lpush %s \"%s\"", routlist_name.c_str(), pos_list_str.c_str());
        MESS_ERR_IF(!reply || reply->type == REDIS_REPLY_ERROR, "lpush {0} {1} failed: errmsg={2}", routlist_name, pos_list_str, reply->str);
        if (reply->type == REDIS_REPLY_INTEGER) {
            return reply->integer;
        }
        PGZXB_DEBUG_ASSERT(false);
        return -1;
    }

    const std::string &get_auth_token() const {
        return auth_token_;
    }

    static Board &get_instance() {
        static Board ins;
        return ins;
    }
private:
    Board() = default;

    std::string make_key(std::string &&primal_key) {
        return auth_token_ + "_" + std::move(primal_key);
    }

    redisContext *redis_ctx_{nullptr};
    std::string auth_token_;
};

MESSBASE_NAMESPACE_END
#endif // !MESSBASE_BOARD_H
