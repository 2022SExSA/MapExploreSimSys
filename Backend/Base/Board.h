#ifndef MESSBASE_BOARD_H
#define MESSBASE_BOARD_H

#include <optional>
#include <string>
#include <string_view>

#include <hiredis/hiredis.h>
#include <hiredis/read.h>

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
        auto routlist_name = make_key(ROUTLIST_NAME_FROMAT, car_name);
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
        // lpush {auth-token}_RoutList@{car-name} "{json1}" "{json2}" ...
        auto routlist_name = make_key(ROUTLIST_NAME_FROMAT, car_name);
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

    std::optional<Point<int>> get_current_position_of_car(const std::string &car_name) {
        // hmget {auth-token}_Position@{car-name} x y
        auto position_name = make_key(CAR_POSITION_NAME_FORMAT, car_name);
        redisReply *reply = (redisReply*)redisCommand(redis_ctx_, "hmget %s x y", position_name.c_str());
        MESS_ERR_IF(!reply || reply->type == REDIS_REPLY_ERROR, "hmget {0} x y failed: errmsg={1}", position_name, reply->str);
        if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
            if (reply->element[0]->type == REDIS_REPLY_STRING && reply->element[1]->type == REDIS_REPLY_STRING) {
                return Point<int>{std::atoi(reply->element[0]->str), std::atoi(reply->element[1]->str)};
            } else {
                MESS_LOG("x, y is not string", 1);
            }
        }
        return std::nullopt;
    }

    bool set_position_of_car(const std::string &car_name, const Point<int> &pos) {
        // hmset {auth-token}_Position@{car-name} x {pos.x} y {pos.y}
        auto position_name = make_key(CAR_POSITION_NAME_FORMAT, car_name);
        redisReply *reply = (redisReply*)redisCommand(redis_ctx_, "hmset %s x %d y %d", position_name.c_str(), pos.x, pos.y);
        MESS_ERR_IF(!reply || reply->type == REDIS_REPLY_ERROR, "hmset %s x %d y %d failed: errmsg={1}", position_name.c_str(), pos.x, pos.y, reply->str);
        return reply && reply->type != REDIS_REPLY_ERROR;
    }

    int get_grid_of_map(int r, int c) {
        auto [w, h] = get_map_size();
        if (r < 0 || r > h || c < 0 || c > w) return -1;
        return get_bit(make_key(MAP_NAME), r * w + c);
    }

    int set_grid_of_map(int r, int c, int val) {
        auto [w, h] = get_map_size();
        if (r < 0 || r > h || c < 0 || c > w) return -1;
        return set_bit(make_key(MAP_NAME), r * w + c, val);
    }

    int get_grid_of_map_block(int r, int c) {
        auto [w, h] = get_map_size();
        if (r < 0 || r > h || c < 0 || c > w) return -1;
        return get_bit(make_key(MAP_BLOCK_NAME), r * w + c);
    }

    int set_grid_of_map_block(int r, int c, int val) {
        auto [w, h] = get_map_size();
        if (r < 0 || r > h || c < 0 || c > w) return -1;
        return set_bit(make_key(MAP_BLOCK_NAME), r * w + c, val);
    }

    const std::string &get_auth_token() const {
        return auth_token_;
    }

    static Board* get_instance() {
        static Board ins;
        return &ins;
    }

    std::tuple<int, int> get_map_size() {
        auto map_size_name = make_key(MAP_SIZE_NAME);
        auto *map_size_reply = (redisReply*)redisCommand(redis_ctx_, "hmget %s w h", map_size_name.c_str());
        MESS_ERR_IF(!map_size_reply || map_size_reply->type == REDIS_REPLY_ERROR, "hmget {0} w h failed: errmsg={1}", map_size_name, map_size_reply->str);
        PGZXB_DEBUG_ASSERT(map_size_reply->type == REDIS_REPLY_ARRAY);
        PGZXB_DEBUG_ASSERT(map_size_reply->element[0]->type == REDIS_REPLY_STRING);
        PGZXB_DEBUG_ASSERT(map_size_reply->element[1]->type == REDIS_REPLY_STRING);
        return {std::atoi(map_size_reply->element[0]->str), std::atoi(map_size_reply->element[1]->str)};
    }
private:
    Board() = default;

    template <typename ...Args>
    std::string make_key(std::string_view fmt, Args &&... args) {
        return make_key_impl(pgfmt::format(fmt.data(), std::forward<Args>(args)...));
    }

    std::string make_key_impl(std::string &&primal_key) {
        return auth_token_ + "_" + std::move(primal_key);
    }

    int set_bit(const std::string &key, int offset, int val) {
        // ASSERT(offset is valid);

        // setbit {key} {offset} {val}
        auto *reply = (redisReply*)redisCommand(redis_ctx_, "setbit %s %d %d", key.c_str(), offset, val);
        MESS_ERR_IF(!reply || reply->type == REDIS_REPLY_ERROR, "setbit {0} {1} {2} failed: errmsg={3}", key, offset, val, reply->str);
        PGZXB_DEBUG_ASSERT(reply->type == REDIS_REPLY_INTEGER);
        return reply->integer;
    }

    int get_bit(const std::string &key, int offset) {
        // ASSERT(offset is valid);

        // getbit {key} {offset}
        auto *reply = (redisReply*)redisCommand(redis_ctx_, "getbit %s %d", key.c_str(), offset);
        MESS_ERR_IF(!reply || reply->type == REDIS_REPLY_ERROR, "getbit {0} {1} failed: errmsg={2}", key, offset, reply->str);
        PGZXB_DEBUG_ASSERT(reply->type == REDIS_REPLY_INTEGER);
        return reply->integer;
    }

    redisContext *redis_ctx_{nullptr};
    std::string auth_token_;
};

MESSBASE_NAMESPACE_END
#endif // !MESSBASE_BOARD_H
