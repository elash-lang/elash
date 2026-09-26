#pragma once

#include <elash/util/alloc.h>

ElOutOfMemCb elc_out_of_mem_cb;
void elc_register_crash_handlers();
