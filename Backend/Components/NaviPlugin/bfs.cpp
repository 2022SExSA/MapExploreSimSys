#include <iostream>
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
        if (r == -1 && c != -1) {
            PGZXB_DEBUG_Print("No nonblock");
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

    // BFS
    struct RC { int r = -1, c = -1; };
    std::queue<RC> q;
    std::vector<std::vector<RC>> pre(h, std::vector<RC>(w, RC{-1, -1}));
    std::vector<std::vector<char>> vis(h, std::vector<char>(w, false));
    q.push({src_r, src_c});
    while (!q.empty()) {
        const int size = q.size();
        for (int i = 0; i < size; ++i) {
            RC p = q.front(); q.pop();
            int r = p.r, c = p.c;
            vis[r][c] = true;
            if (r == dest_r && c == dest_c) {
                decltype(q)().swap(q); // Clear q
                break;
            }
            // Up
            if (r > 0 && !vis[r - 1][c] && Call2(get_grid_of_map_block, r - 1, c) == 0) {
                q.push({r - 1, c});
                pre[r - 1][c] = {r, c};
                vis[r - 1][c] = true;
            }
            // Down
            if (r + 1 < h && !vis[r + 1][c] && Call2(get_grid_of_map_block, r + 1, c) == 0) {
                q.push({r + 1, c});
                pre[r + 1][c] = {r, c};
                vis[r + 1][c] = true;
            }
            // Left
            if (c > 0 && !vis[r][c - 1] && Call2(get_grid_of_map_block, r, c - 1) == 0) {
                q.push({r, c - 1});
                pre[r][c - 1] = {r, c};
                vis[r][c - 1] = true;
            }
            // Right
            if (c + 1 < w && !vis[r][c + 1] && Call2(get_grid_of_map_block, r, c + 1) == 0) {
                q.push({r, c + 1});
                pre[r][c + 1] = {r, c};
                vis[r][c + 1] = true;
            }
        }
    }

    // Gen route
    int r = dest_r, c = dest_c;
    pos_list->push_back({r, c});
    while ((r != src_r || c != src_c) && r != -1 && c != -1) {
        auto pre_pos = pre[r][c];
        r = pre_pos.r;
        c = pre_pos.c;
        pos_list->push_back({r, c});
    }
    std::reverse(pos_list->begin(), pos_list->end());

    // Return result
    rout_pos_array->array = pos_list->data();
    rout_pos_array->size = pos_list->size();
    rout_pos_array->ctx = (void*)pos_list;
    rout_pos_array->destroy_array = messbase_navi_plugi_free_pos_array;
    return 0;
}


} // extern "C"
