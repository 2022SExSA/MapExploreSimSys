#include "navigator_plugin_utils.h"

MESSBASE_NAMESPACE_START
namespace naviplugin {

int get_width(MESSNP_MapContextHandle ctx) {
    MapContext *mctx = (MapContext*)ctx;
    PGZXB_DEBUG_ASSERT(mctx && mctx->board);
    return std::get<0>(mctx->board->get_map_size());
}

int get_height(MESSNP_MapContextHandle ctx) {
    MapContext *mctx = (MapContext*)ctx;
    PGZXB_DEBUG_ASSERT(mctx && mctx->board);
    return std::get<1>(mctx->board->get_map_size());
}

int get_grid_of_map(MESSNP_MapContextHandle ctx, int r, int c) {
    MapContext *mctx = (MapContext*)ctx;
    PGZXB_DEBUG_ASSERT(mctx && mctx->board);
    return mctx->board->get_grid_of_map(r, c);
}

int get_grid_of_map_block(MESSNP_MapContextHandle ctx, int r, int c) {
    MapContext *mctx = (MapContext*)ctx;
    PGZXB_DEBUG_ASSERT(mctx && mctx->board);
    return mctx->board->get_grid_of_map_block(r, c);
}

MESSNP_MapInfoGetter *create_default_map_info_getter() {
    static MESSNP_MapInfoGetter map_info_getter;
    static MapContext map_ctx;

    map_ctx.board = Board::get_instance();
    map_info_getter.ctx = (void*)&map_ctx;
    map_info_getter.get_width = get_width;
    map_info_getter.get_width = get_height;
    map_info_getter.get_grid_of_map = get_grid_of_map;
    map_info_getter.get_grid_of_map_block = get_grid_of_map_block;
    
    return &map_info_getter;
}

} // namespace naviplugin
MESSBASE_NAMESPACE_END
