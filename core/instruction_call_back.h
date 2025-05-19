//
// Created by reverccqin on 25-5-17.
//

#ifndef INSTRUCTION_CALL_BACK_H
#define INSTRUCTION_CALL_BACK_H
#include <frida-gum.h>
struct InstructionInfo {
    cs_insn insn_copy{};
    cs_detail *detail_copy = nullptr;
    csh handle = 0;

    InstructionInfo(cs_insn *insn, csh _handle) {
        // 拷贝基本字段
        insn_copy = *insn;
        handle = _handle;
        // 如果 detail 不为空，执行深拷贝
        if (insn->detail != nullptr) {
            detail_copy = (cs_detail *)malloc(sizeof(cs_detail));
            memcpy(detail_copy, insn->detail, sizeof(cs_detail));
            insn_copy.detail = detail_copy;
        } else {
            insn_copy.detail = nullptr;
        }
    }
    ~InstructionInfo() {
        if (detail_copy != nullptr) {
            free(detail_copy);
        }
    }
};

void transform_callback(GumStalkerIterator *iterator, GumStalkerOutput *output, gpointer user_data);
void instruction_callback(GumCpuContext *context, void *user_data);
void event_sink_callback(const GumEvent * event, GumCpuContext * cpu_context, gpointer user_data);
#endif //INSTRUCTION_CALL_BACK_H
