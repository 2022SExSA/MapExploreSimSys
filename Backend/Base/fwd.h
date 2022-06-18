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
#define MESS_LOG(...) PGZXB_PASS
#define MESS_LOG_IF(cond, fmt, ...) PGZXB_PASS
#define MESS_ERR(fmt, ...) PGZXB_PASS
#define MESS_ERR_IF(cond, fmt, ...) PGZXB_PASS
#define MESS_LOG_MODULE(name) namespace{}
#endif

MESSBASE_NAMESPACE_START

extern const char *mess_log_module_name;

using Json = nlohmann::json;

enum class ExperimentState {
    MIN_VAL,
    INVALID,
    INIT,
    RUNNING,
    PAUSE,
    EXIT,
    MAX_VAL
};
using ES = ExperimentState;

constexpr char ROUTLIST_NAME_PREFIX[] = "CarRoutList@";
constexpr char CAR_POSITION_NAME_PREFIX[] = "CarPosition@";

constexpr char ROUTLIST_NAME_FROMAT[]     = "CarRoutList@{0}"; // CarRoutList@Car001
constexpr char CAR_POSITION_NAME_FORMAT[] = "CarPosition@{0}"; // CarPosition@@Car001
constexpr char MAP_NAME[]                 = "Map";            // Map
constexpr char MAP_BLOCK_NAME[]           = "MapBlock";       // MapBlock
constexpr char MAP_SIZE_NAME[]            = "MapSize";        // MapSize

constexpr char EXPERIMENT_STATE_NAME[]    = "ExperState";
constexpr char EXPERIMENT_FREAME_CNT_NAME[]    = "ExperFrCnt";

constexpr int MAP_GRID_COVERED_FLAG = 1;
constexpr int MAP_GRID_RAW_FLAG = 0;

constexpr int D2I_FACTOR = 1000;

template <typename ...T>
inline void pgzxb_unused(T &&... t) {
    (void(t), ...);
}

bool mess_set_log_module_name(const char * name);

MESSBASE_NAMESPACE_END
#endif
