#ifndef MESS_NAVI_PLUGIN_UTILS_H
#define MESS_NAVI_PLUGIN_UTILS_H

typedef void* MESSNP_MapContextHandle;
typedef void(*MESSNP_PVoidVoidFunc)(void*);
typedef int(*MESSNP_CtxRetIntFunc)(MESSNP_MapContextHandle);
typedef int(*MESSNP_CtxIntIntRetIntFunc)(MESSNP_MapContextHandle, int, int);

typedef MESSNP_CtxRetIntFunc MESSNP_GetWidthFunc;
typedef MESSNP_CtxRetIntFunc MESSNP_GetHeightFunc;
typedef MESSNP_CtxIntIntRetIntFunc MESSNP_GetGridOfMapFunc;
typedef MESSNP_CtxIntIntRetIntFunc MESSNP_GetGridOfMapBlockFunc;

typedef struct MESSNP_RC_v1 {
    int r;
    int c;
} MESSNP_Position;

typedef struct MESSNP_PositionArray_v1 {
    void* ctx;
    MESSNP_PVoidVoidFunc destroy_array;
    
    MESSNP_Position *array;
    int size;
} MESSNP_PositionArray;

typedef struct MESSNP_MapInfoGetter_v1 {
    MESSNP_MapContextHandle ctx;

    MESSNP_GetWidthFunc get_width;
    MESSNP_GetHeightFunc get_height;
    MESSNP_GetGridOfMapFunc get_grid_of_map;
    MESSNP_GetGridOfMapBlockFunc get_grid_of_map_block;
} MESSNP_MapInfoGetter;

#define MESSNP_STR(x) #x

#define MESSNP_SELECT_DEST_POS_FUNC_V1 pg_mess_navi_select_dest_pos_v1
#define MESSNP_ROUTING_FUNC_V1 pg_mess_navi_routing_v1
#define MESSNP_SELECT_DEST_POS_FUNC_V1_NAME MESSNP_STR(MESSNP_SELECT_DEST_POS_FUNC_V1)
#define MESSNP_ROUTING_FUNC_V1_NAME MESSNP_STR(MESSNP_ROUTING_FUNC_V1)

typedef int(*MESSNP_NaviSelectDestPosFuncV1)(MESSNP_Position *dest_pos, const MESSNP_Position *src_pos, MESSNP_MapInfoGetter *map_info_getter);
typedef int(*MESSNP_NaviRoutingV1)(MESSNP_PositionArray *rout_pos_array, const MESSNP_Position *src_pos, const MESSNP_Position *dest_pos, MESSNP_MapInfoGetter *map_info_getter);

#ifdef MESSBASE_LIB
#include "fwd.h"
#include "Board.h"
MESSBASE_NAMESPACE_START
namespace naviplugin {

struct MapContext {
    Board *board{nullptr};
};

int get_width(MESSNP_MapContextHandle ctx);
int get_height(MESSNP_MapContextHandle ctx);
int get_grid_of_map(MESSNP_MapContextHandle ctx, int r, int c);
int get_grid_of_map_block(MESSNP_MapContextHandle ctx, int r, int c);
MESSNP_MapInfoGetter *create_default_map_info_getter();

} // namespace naviplugin
MESSBASE_NAMESPACE_END
#endif // MESSBASE_LIB

#endif // !MESS_NAVI_PLUGIN_UTILS_H
