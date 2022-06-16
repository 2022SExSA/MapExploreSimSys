#undef MESSBASE_LIB
#include "navigator_plugin_utils.h"

#include <vector>

extern "C" {

void free_pos_array(void* ctx) {
    delete (std::vector<MESSNP_Position>*)ctx;
}

int MESSNP_SELECT_DEST_POS_FUNC_V1(MESSNP_Position *dest_pos, const MESSNP_Position *src_pos, MESSNP_MapInfoGetter *map_info_getter) {

    dest_pos->r = 5;
    dest_pos->c = 5;

    return 0;
}

int MESSNP_ROUTING_FUNC_V1(MESSNP_PositionArray *rout_pos_array, const MESSNP_Position *src_pos, const MESSNP_Position *dest_pos) {

    auto *pos_list = new std::vector<MESSNP_Position>;

    for (int i = 0; i < 5; ++i) {
        pos_list->push_back({i, 0});
    }
    for (int i = 0; i < 5; ++i) {
        pos_list->push_back({5, i});
    }

    // pos_list->push_back({0, 0});
    // pos_list->push_back({0, 1});
    // pos_list->push_back({1, 1});
    // pos_list->push_back({1, 2});
    // pos_list->push_back({1, 3});
    // pos_list->push_back({1, 4});
    // pos_list->push_back({1, 5});
    // pos_list->push_back({2, 5});
    // pos_list->push_back({3, 5});
    // pos_list->push_back({4, 5});
    // pos_list->push_back({5, 5});

    rout_pos_array->array = pos_list->data();
    rout_pos_array->size = pos_list->size();
    rout_pos_array->ctx = (void*)pos_list;
    rout_pos_array->destroy_array = free_pos_array;

    return 0;
}


} // extern "C"
