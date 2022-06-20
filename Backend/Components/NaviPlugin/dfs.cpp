#undef MESSBASE_LIB
#include "fwd.h"
#include "navigator_plugin_utils.h"

#include <vector>
#include <queue>
#include <algorithm>
#include <time.h>
#include <stdlib.h>
#include <assert.h>

#define Call(func) \
    map_info_getter->func(map_info_getter->ctx)

#define Call2(func, ...) \
    map_info_getter->func(map_info_getter->ctx, __VA_ARGS__)

extern "C" {

static int messbase_navi_plugin_set_bfs_rand_seed() {
    srand((int)time(NULL));
    return 0;
}

static volatile int messbase_navi_plugin_bfs_srand_flag = messbase_navi_plugin_set_bfs_rand_seed();

using Vis = std::vector<std::vector<char>>;
#define OTHERS dest_r, dest_c, w, h, vis, path, res, map_info_getter

static void dfs(int r, int c, int dest_r, int dest_c, int w, int h, Vis &vis, std::vector<MESSNP_Position> &path, std::vector<MESSNP_Position> &res, MESSNP_MapInfoGetter *map_info_getter) {
    if (!res.empty()) return; // Have got result; Only get first.
    
    vis[r][c] = true;
    path.push_back({r, c});

    if (r == dest_r && c == dest_c) { // Dest
        res = path;
        return;
    }
    
    // Up
    if (r > 0 && !vis[r - 1][c] && Call2(get_grid_of_map_block, r - 1, c) == 0) {
        // vis[r - 1][c] = true;
        // path.push_back({r - 1, c});
        dfs(r - 1, c, OTHERS);
        // vis[r - 1][c] = false;
        // path.pop_back();
    }
    // Down
    if (r + 1 < h && !vis[r + 1][c] && Call2(get_grid_of_map_block, r + 1, c) == 0) {
        // vis[r + 1][c] = true;
        // path.push_back({r + 1, c});
        dfs(r + 1, c, OTHERS);
        // vis[r + 1][c] = false;
        // path.pop_back();
    }
    // Left
    if (c > 0 && !vis[r][c - 1] && Call2(get_grid_of_map_block, r, c - 1) == 0) {
        // vis[r][c - 1] = true;
        // path.push_back({r, c - 1});
        dfs(r, c - 1, OTHERS);
        // vis[r][c - 1] = false;
        // path.pop_back();
    }
    // Right
    if (c + 1 < w && !vis[r][c + 1] && Call2(get_grid_of_map_block, r, c + 1) == 0) {
        // vis[r][c + 1] = true;
        // path.push_back({r, c + 1});
        dfs(r, c + 1, OTHERS);
        // vis[r][c + 1] = false;
        // path.pop_back();
    }
    vis[r][c] = false;
    path.pop_back();
}

void messbase_navi_plugi_free_pos_array(void* ctx) {
    delete (std::vector<MESSNP_Position>*)ctx;
}

int MESSNP_SELECT_DEST_POS_FUNC_V1(MESSNP_Position *dest_pos, const MESSNP_Position *src_pos, MESSNP_MapInfoGetter *map_info_getter) {
    int r = 0 , c = 0;

    // Rand
    const int w = Call(get_width);
    const int h = Call(get_height);
    const int src_r = src_pos->r;
    const int src_c = src_pos->c;
    int cnt = 0;

    constexpr int table[] = {1, -1};

    do {
        r = src_r + table[rand() % 2] * (rand() % (h - src_r));
        c = src_c + table[rand() % 2] * (rand() % (w - src_c));
        if (r < 0) r = 0;
        if (r >= h) r = h - 1;
        if (c < 0) c = 0;
        if (c >= w) c = w - 1;
    } while (Call2(get_grid_of_map_block, r, c) != 0 && Call2(get_grid_of_map, r, c) != pg::messbase::MAP_GRID_COVERED_FLAG && cnt++ < 20);

    // Tranversal
    if (Call2(get_grid_of_map_block, r, c) != 0 || Call2(get_grid_of_map, r, c) != pg::messbase::MAP_GRID_COVERED_FLAG) {
        r = -1;
        c = -1;
        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                if (Call2(get_grid_of_map_block, i, j) == 0 && Call2(get_grid_of_map, i, j) == pg::messbase::MAP_GRID_COVERED_FLAG) {
                    r = i;
                    c = j;
                    break;
                }
            }
        }
        if (r == -1 && c == -1) {
            PGZXB_DEBUG_Print("No coverd but nonblocked");
        }
    }
   
    // Return result
    dest_pos->r = r;
    dest_pos->c = c;
    return 0;
}

int MESSNP_ROUTING_FUNC_V1(MESSNP_PositionArray *rout_pos_array, const MESSNP_Position *src_pos, const MESSNP_Position *dest_pos, MESSNP_MapInfoGetter *map_info_getter) {
    auto *pos_list = new std::vector<MESSNP_Position>;

    const int w = Call(get_width);
    const int h = Call(get_height);
    const int src_r = src_pos->r;
    const int src_c = src_pos->c;
    const int dest_r = dest_pos->r;
    const int dest_c = dest_pos->c;

    // DFS

    std::vector<MESSNP_Position> path;
    Vis vis(h, std::vector<char>(w, false));
    dfs(src_r, src_c, dest_r, dest_c, w, h, vis, path, *pos_list, map_info_getter);

    // Return result
    rout_pos_array->array = pos_list->data();
    rout_pos_array->size = pos_list->size();
    rout_pos_array->ctx = (void*)pos_list;
    rout_pos_array->destroy_array = messbase_navi_plugi_free_pos_array;
    return 0;
}


} // extern "C"
