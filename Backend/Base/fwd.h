#ifndef PGC2CALCU_FWD_H
#define PGC2CALCU_FWD_H

#include "pg/pgfwd.h"
#include "pg/pgfmt.h"
#include "nlohmann/json.hpp"

#define MESSBASE_NAMESPACE_START PGZXB_ROOT_NAMESPACE_START namespace messbase {
#define MESSBASE_NAMESPACE_END } PGZXB_ROOT_NAMESPACE_END

#ifdef MESS_WITH_LOG
#include <filesystem>
#define MESS_LOG(fmt, ...) \
    std::cerr << pgfmt::format( \
        "[MapExploreSimSys-Backend]{0}:{1} LOG: {2}\n", \
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
#endif

MESSBASE_NAMESPACE_START

using Json = nlohmann::json;
constexpr char ROUTLIST_NAME_FROMAT[] = "RoutList@{0}";

MESSBASE_NAMESPACE_END
#endif
