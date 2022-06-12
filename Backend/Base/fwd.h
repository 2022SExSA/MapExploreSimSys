#ifndef PGC2CALCU_FWD_H
#define PGC2CALCU_FWD_H

#include "pg/pgfwd.h"
#include "pg/pgfmt.h"
#include "nlohmann/json.hpp"

#define MESSBASE_NAMESPACE_START PGZXB_ROOT_NAMESPACE_START namespace messbase {
#define MESSBASE_NAMESPACE_END } PGZXB_ROOT_NAMESPACE_END

#ifdef MESS_WITH_LOG
#include <filesystem>

#define MESS_LOG_MODULE(name) \
    MESSBASE_NAMESPACE_START \
    namespace { \
    bool log_flag_##__LINE__ = mess_set_log_module_name(name); \
    } \
    MESSBASE_NAMESPACE_END

#define MESS_LOG(fmt, ...) \
    std::cerr << pgfmt::format( \
        "[MESS/{0}] {1}:{2} LOG: {3}\n", \
        pg::messbase::mess_log_module_name, \
        std::filesystem::relative(__FILE__).string(), \
        __LINE__, \
        pgfmt::format(fmt, __VA_ARGS__))
#define MESS_LOG_IF(cond, fmt, ...) if (cond) MESS_LOG(fmt, __VA_ARGS__)
#define MESS_ERR(fmt, ...) MESS_LOG(fmt, __VA_ARGS__); exit(-1)
#define MESS_ERR_IF(cond, fmt, ...) if (cond) { MESS_ERR(fmt, __VA_ARGS__); } PGZXB_PASS
#else
#define MESS_LOG(...) pgzxb_unused(__VA_ARGS__)
#define MESS_LOG_IF(cond, fmt, ...) pgzxb_unused(cond, fmt, __VA_ARGS__)
#define MESS_ERR(fmt, ...) pgzxb_unused(fmt, __VA_ARGS__)
#define MESS_ERR_IF(cond, fmt, ...) pgzxb_unused(cond, fmt, __VA_ARGS__)
#endif

MESSBASE_NAMESPACE_START

extern const char *mess_log_module_name;

using Json = nlohmann::json;
constexpr char ROUTLIST_NAME_FROMAT[]     = "RoutList@{0}";  // RoutList@Car001
constexpr char CAR_POSITION_NAME_FORMAT[] = "Position@{0}";  // Position@@Car001
constexpr char MAP_NAME[]                 = "Map";           // Map
constexpr char MAP_BLOCK_NAME[]           = "MapBlock";      // MapBlock
constexpr char MAP_SIZE_NAME[]            = "MapSize";       // MapSize

template <typename ...T>
inline void pgzxb_unused(T &&... t) {
    (void(t), ...);
}

bool mess_set_log_module_name(const char * name);

MESSBASE_NAMESPACE_END
#endif
