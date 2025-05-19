//
// Created by reverccqin on 25-5-17.
//

#include "init.h"
#include <frida-gum.h>
#include "common.h"
#include "instruction_tracer_manager.h"

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    smjni::jni_provider::init(env);
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void gum_trace(char* module_name, uintptr_t offset, char* trace_file_name) {
    auto instance = InstructionTracerManager::get_instance();
    if (!instance->init(module_name, offset)) {
        LOGE("init stalker fail");
        return;
    }
    instance->get_logger_manager()->set_enable_to_file(true, trace_file_name);
    bool ret = instance->run_attach();
    LOGD("run attach ret:%d", ret);
}


