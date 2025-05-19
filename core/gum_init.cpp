//
// Created by reverccqin on 25-5-15.
//
#include "gum_init.h"
#include <frida-gum.h>

void __attribute__((constructor)) init_proc() {
    gum_init_embedded();
}

void __attribute__((destructor)) fini_proc() {
    gum_deinit_embedded();
}