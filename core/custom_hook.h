//
// Created by reverccqin on 25-5-17.
//

#ifndef CUSTOM_HOOK_H
#define CUSTOM_HOOK_H
#include <frida-gum.h>
void hook_common_enter(GumInvocationContext * ic, gpointer user_data);
void hook_common_leave(GumInvocationContext * ic, gpointer user_data);
#endif //CUSTOM_HOOK_H
