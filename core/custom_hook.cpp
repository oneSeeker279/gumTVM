//
// Created by reverccqin on 25-5-15.
//
#include "custom_hook.h"
#include <frida-gum.h>
#include <jni.h>
#include <unistd.h>
#include "common.h"
#include "instruction_tracer_manager.h"

void hook_common_enter(GumInvocationContext * ic, gpointer user_data) {
    auto self = (InstructionTracerManager *)user_data;
    if (self->get_trace_tid() == 0) {
        LOGD("FridaStalker::frida_on_enter");
        self->set_trace_tid(getpid());
        // start trace
        auto* frida_stalker = new InstructionTracerManager();
        frida_stalker->follow();
    }
}

void hook_common_leave(GumInvocationContext * ic, gpointer user_data) {
    auto self = (InstructionTracerManager *)user_data;
    if (self->get_trace_tid() == getpid()) {
        LOGD("FridaStalker::frida_on_leave");
        // 取消hook
        GumInterceptor* insterceptor = gum_interceptor_obtain();
        gum_interceptor_detach(insterceptor, self->get_invocation_listener());
        gum_interceptor_begin_transaction(insterceptor);
        // 更新关闭trace文件
        self->get_logger_manager()->set_enable_to_file(false, "");
    }
}