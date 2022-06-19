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

    bool set_fream_cnt(int val) {
        // set {auth-token}ExperFrCnt
        auto key = make_key(EXPERIMENT_FREAME_CNT_NAME);
        redisReply *reply = (redisReply*)redisCommand(redis_ctx_, "set %s %d", key.c_str(), (int)val);
        return reply && reply->type != REDIS_REPLY_ERROR;
    }

    int get_frame_cnt() {
        // get {auth-token}ExperFrCnt
        auto key = make_key(EXPERIMENT_FREAME_CNT_NAME);
        redisReply *reply = (redisReply*)redisCommand(redis_ctx_, "get %s", key.c_str());
        MESS_ERR_IF(!reply || reply->type == REDIS_REPLY_ERROR, "get {0} failed: errmsg={1}", key, std::string(reply->str));
        if (reply->type == REDIS_REPLY_STRING) {
            try {
                return std::stod(reply->str);
            } catch (const std::exception &) {
            }
        }
        MESS_LOG("Get frame count failed, str={0}", reply ? reply->str : std::string("<null-reply>"));
        return -1;
    }

    bool set_experiment_state(ES val) {
        // set {auth-token}ExperimentState
        auto state_key = make_key(EXPERIMENT_STATE_NAME);
        redisReply *reply = (redisReply*)redisCommand(redis_ctx_, "set %s %d", state_key.c_str(), (int)val);
        return reply && reply->type != REDIS_REPLY_ERROR;
    }

    ES get_experiment_state() {
        // get {auth-token}ExperimentState
        auto state_key = make_key(EXPERIMENT_STATE_NAME);
        redisReply *reply = (redisReply*)redisCommand(redis_ctx_, "get %s", state_key.c_str());
        MESS_ERR_IF(!reply || reply->type == REDIS_REPLY_ERROR, "get {0} failed: errmsg={1}", state_key, std::string(reply->str));
        if (reply->type == REDIS_REPLY_STRING) {
            try {
                auto i = std::stod(reply->str);
                if (i > (int)ES::MIN_VAL && i < (int)ES::MAX_VAL) return (ES)i;
            } catch (const std::exception &) {
                return ES::INVALID;
            }
        }
        MESS_LOG("Get experiment state failed", 0);
        return ES::INVALID;
    }

    std::vector<std::string> get_all_cars() {
        auto prefix = make_key(ROUTLIST_NAME_PREFIX);
        auto res = get_keys_by_prefix(make_key(ROUTLIST_NAME_PREFIX));

        const auto prefix_size = prefix.size();
        std::for_each(res.begin(), res.end(), [prefix_size](auto &e) {
            e = e.substr(prefix_size);
        });

        return res;
    }

    int get_routlist_size(const std::string &car_name) {
        // llen {auth-token}RoutList@{car-name}
        auto routlist_name = make_key(ROUTLIST_NAME_FROMAT, car_name);
        redisReply *reply = (redisReply*)redisCommand(redis_ctx_, "llen %s", routlist_name.c_str());
        if (!reply || reply->type == REDIS_REPLY_ERROR) {
            MESS_LOG("Get size of {0} failed", routlist_name);
            return -1;
        }
        if (reply->type == REDIS_REPLY_INTEGER) {
            return reply->integer;
        }
        MESS_LOG("Get size of {0} failed", routlist_name);
        return -1;
    }

    std::vector<Point<int>> get_all_pos_of_routlist(const std::string &car_name) {
        // lrange {auth-token}RoutList@{car-name} 0 -1
        auto routlist_name = make_key(ROUTLIST_NAME_FROMAT, car_name);
        redisReply *reply = (redisReply*)redisCommand(redis_ctx_, "lrange %s 0 -1", routlist_name.c_str());
        MESS_ERR_IF(!reply || reply->type == REDIS_REPLY_ERROR, "lrange {0} 0 -1 failed: errmsg={1}", routlist_name, std::string(reply->str));
        if (reply->type == REDIS_REPLY_ARRAY) {
            std::vector<Point<int>> res;
            const int len = reply->elements;
            for (int i = 0; i < len; ++i) {
                PGZXB_DEBUG_ASSERT(reply->element[i]->type == REDIS_REPLY_STRING);
                Json json = Json::parse(std::string(reply->element[i]->str));
                PGZXB_DEBUG_ASSERT(json.is_object());
                res.push_back({json["x"], json["y"]});
            }
            return res;
        }
        MESS_LOG("Empty {0}", routlist_name);
        return {};
    }

    std::optional<Point<int>> get_next_routing_position(const std::string &car_name) {
        // rpop {auth-token}RoutList@{car-name}
        auto routlist_name = make_key(ROUTLIST_NAME_FROMAT, car_name);
        redisReply *reply = (redisReply*)redisCommand(redis_ctx_, "rpop %s", routlist_name.c_str());
        MESS_ERR_IF(!reply || reply->type == REDIS_REPLY_ERROR, "rpop {0} failed: errmsg={1}", routlist_name, std::string(reply->str));
        if (reply->type == REDIS_REPLY_STRING) {
            Json json = Json::parse(std::string(reply->str));
            PGZXB_DEBUG_ASSERT(json.is_object());
            return Point<int>{json["x"], json["y"]};
        }
        MESS_ERR_IF(reply->type != REDIS_REPLY_NIL, "rpop {0} failed", routlist_name);
        MESS_LOG("Empty {0}", routlist_name);
        return std::nullopt;
    }

    long long add_position_to_routlist(const std::string &car_name, const Point<int> &pos) {
        // lpush {auth-token}_RoutList@{car-name} "{json1}" "{json2}" ...
        auto routlist_name = make_key(ROUTLIST_NAME_FROMAT, car_name);
        Json json = Json::object();
        json["x"] = pos.x;
        json["y"] = pos.y;
        std::string pos_list_str = json.dump();
        redisReply *reply = (redisReply*)redisCommand(redis_ctx_, "lpush %s %s", routlist_name.c_str(), pos_list_str.c_str());
        MESS_ERR_IF(!reply || reply->type == REDIS_REPLY_ERROR, "lpush {0} {1} failed: errmsg={2}", routlist_name, pos_list_str, std::string(reply->str));
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
        MESS_ERR_IF(!reply || reply->type == REDIS_REPLY_ERROR, "hmget {0} x y failed: errmsg={1}", position_name, std::string(reply->str));
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
        MESS_ERR_IF(!reply || reply->type == REDIS_REPLY_ERROR, "hmset %s x %d y %d failed: errmsg={1}", position_name.c_str(), pos.x, pos.y, std::string(reply->str));
        return reply && reply->type != REDIS_REPLY_ERROR;
    }

    int count_raw_grid_of_map() {
        static_assert(MAP_GRID_RAW_FLAG == 0);
        auto key = make_key(MAP_NAME);
        redisReply *covered_cnt_reply = (redisReply*)redisCommand(redis_ctx_, "bitcount %s", key.c_str());
        MESS_ERR_IF(!covered_cnt_reply || covered_cnt_reply->type != REDIS_REPLY_INTEGER, "bitount {0} faile", key);
        const auto [w, h] = get_map_size();
        return w * h - covered_cnt_reply->integer;        
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

    static std::unique_ptr<Board> new_instance(const std::string &auth_token, const std::string &ip, short port) {
        std::unique_ptr<Board> res{new Board{}};
        res->init(auth_token, ip, port);
        return res;
    }

    static Board* get_instance() {
        static Board ins;
        return &ins;
    }

    void set_map_size(int w, int h) {
        auto map_size_name = make_key(MAP_SIZE_NAME);
        [[maybe_unused]] auto *reply = (redisReply*)redisCommand(redis_ctx_, "hmset %s w %d h %d", map_size_name.c_str(), w, h);
        MESS_ERR_IF(!reply || reply->type == REDIS_REPLY_ERROR, "hmget {0} w {1} h {2} failed: errmsg={3}", map_size_name, w, h, std::string(reply->str));
    }

    std::tuple<int, int> get_map_size() {
        auto map_size_name = make_key(MAP_SIZE_NAME);
        auto cmd = std::string("hmget ").append(map_size_name).append(" w h");
        auto *map_size_reply = (redisReply*)redisCommand(redis_ctx_, cmd.c_str());
        MESS_ERR_IF(!map_size_reply || map_size_reply->type == REDIS_REPLY_ERROR, "hmget {0} w h failed: errmsg={1}", map_size_name, map_size_reply->str);
        if (map_size_reply->type != REDIS_REPLY_ARRAY || map_size_reply->elements != 2) return {-1, -1};
        if (map_size_reply->element[0]->type != REDIS_REPLY_STRING) return {-1, -1};
        if (map_size_reply->element[1]->type != REDIS_REPLY_STRING) return {-1, -1};
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
        MESS_ERR_IF(!reply || reply->type == REDIS_REPLY_ERROR, "setbit {0} {1} {2} failed: errmsg={3}", key, offset, val, std::string(reply->str));
        PGZXB_DEBUG_ASSERT(reply->type == REDIS_REPLY_INTEGER);
        return reply->integer;
    }

    int get_bit(const std::string &key, int offset) {
        // ASSERT(offset is valid);

        // getbit {key} {offset}
        auto *reply = (redisReply*)redisCommand(redis_ctx_, "getbit %s %d", key.c_str(), offset);
        MESS_ERR_IF(!reply || reply->type == REDIS_REPLY_ERROR, "getbit {0} {1} failed: errmsg={2}", key, offset, std::string(reply->str));
        PGZXB_DEBUG_ASSERT(reply->type == REDIS_REPLY_INTEGER);
        return reply->integer;
    }

    std::vector<std::string> get_keys_by_prefix(const std::string &prefix) {
        // keys {prefix}
        auto *reply = (redisReply*)redisCommand(redis_ctx_, "keys %s*", prefix.c_str());
        MESS_ERR_IF(!reply || reply->type == REDIS_REPLY_ERROR, "keys {0}* failed: errmsg={1}", prefix, std::string(reply->str));
        PGZXB_DEBUG_ASSERT(reply->type == REDIS_REPLY_ARRAY);
        
        std::vector<std::string> keys;
        for (std::size_t i = 0; i < reply->elements; ++i) {
            PGZXB_DEBUG_ASSERT(reply->element[i]->type == REDIS_REPLY_STRING);
            keys.push_back(reply->element[i]->str);
        }

        return keys;
    }

    redisContext *redis_ctx_{nullptr};
    std::string auth_token_;
};

MESSBASE_NAMESPACE_END
#endif // !MESSBASE_BOARD_H
