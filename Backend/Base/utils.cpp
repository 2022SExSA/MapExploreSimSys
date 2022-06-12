#include "fwd.h"

MESSBASE_NAMESPACE_START

const char *mess_log_module_name{"Default"};

bool mess_set_log_module_name(const char * name) {
    mess_log_module_name = name;
    return name;
}

MESSBASE_NAMESPACE_END
