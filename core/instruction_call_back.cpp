//
// Created by reverccqin on 25-5-17.
//

#include "instruction_call_back.h"

#include <dlfcn.h>

#include <frida-gum.h>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <sstream>
#include "common.h"
#include "instruction_tracer_manager.h"

bool get_register_value(arm64_reg reg, GumCpuContext* ctx, uint64_t& out_value) {
    uint64_t value = 0;

    // 支持 W0 ~ W30 映射为 Xn & 0xFFFFFFFF
    if (reg >= ARM64_REG_W0 && reg <= ARM64_REG_W30) {
        int idx = reg - ARM64_REG_W0;
        value = ctx->x[idx] & 0xFFFFFFFF;
    }
    // 支持 X0 ~ X30
    else if (reg >= ARM64_REG_X0 && reg <= ARM64_REG_X28) {
        int idx = reg - ARM64_REG_X0;
        value = ctx->x[idx];
    }
    else {
        switch (reg) {
            case ARM64_REG_SP: value = ctx->sp; break;
            case ARM64_REG_FP: value = ctx->fp; break;      // ARM64_REG_X29
            case ARM64_REG_LR: value = ctx->lr; break;      // ARM64_REG_X30
            default:
                return false; // 不支持的寄存器
        }
    }

    out_value = value;
    return true;
}

// 是否是原子操作指令
bool is_lse(cs_insn* insn) {
    bool skip = false;
    switch (insn->id)
    {
        case ARM64_INS_LDAXR:
        case ARM64_INS_LDAXP:
        case ARM64_INS_LDAXRB:
        case ARM64_INS_LDAXRH:
        case ARM64_INS_LDXR:
        case ARM64_INS_LDXP:
        case ARM64_INS_LDXRB:
        case ARM64_INS_LDXRH:
        case ARM64_INS_STXR:
        case ARM64_INS_STXP:
        case ARM64_INS_STXRB:
        case ARM64_INS_STXRH:
        case ARM64_INS_STLXR:
        case ARM64_INS_STLXP:
        case ARM64_INS_STLXRB:
        case ARM64_INS_STLXRH: {
            skip = true;
            break;
        }
        default:
            skip = false;
    }
    return skip;
}

// 基本块回调
void transform_callback(GumStalkerIterator *iterator, GumStalkerOutput *output, gpointer user_data) {
    const auto self = InstructionTracerManager::get_instance();
    if (self == nullptr) {
        LOGE("transform_callback data is nullptr");
        return;
    }

    cs_insn *p_insn;
    auto *it  = iterator;
    while (gum_stalker_iterator_next(it, (const cs_insn **)&p_insn)) {
        if (is_lse(p_insn) == false && self->is_address_in_module_range(p_insn->address)) {
            auto *instruction_info = new InstructionInfo(p_insn,gum_stalker_iterator_get_capstone(it));
            gum_stalker_iterator_put_callout(it,
                instruction_callback,
                instruction_info,
                [](gpointer user_data) {
                auto *ctx = static_cast<InstructionInfo *>(user_data);
                delete ctx;
            });
        }
        gum_stalker_iterator_keep(it);
    }
}

void instruction_callback(GumCpuContext *context, void *user_data) {
    const auto ctx = context;
    auto insn_info = (InstructionInfo *)user_data;
    if (insn_info == nullptr) {
        LOGE("instruction_callback data is nullptr");
        return;
    }
    auto self = InstructionTracerManager::get_instance();

    std::stringstream outinfo;
    std::stringstream postOutput;
    std::stringstream regOutput;
    // 遍历操作数并记录写入的寄存器状态
    if (self->write_reg_list.num) {
        for (int i = 0; i < self->write_reg_list.num; i++) {
            uint64_t reg_value = 0;
            if (get_register_value(self->write_reg_list.regs[i], ctx, reg_value)) {
                const char* reg_name = cs_reg_name( insn_info->handle, self->write_reg_list.regs[i]);
                postOutput << reg_name << "=0x" << std::hex << reg_value << " ";
                postOutput.flush();
                regOutput << self->get_logger_manager()->dump_reg_value(reg_value, reg_name);
            }
        }
        self->write_reg_list.num = 0;
    }
    if (!postOutput.str().empty()) {
        outinfo << "\t w[" << postOutput.str() << "]" << std::endl << regOutput.str();
    }

    // 输出当前指令地址和反汇编信息
    outinfo << "0x" << std::left << std::setw(8) << std::hex << (ctx->pc - self->get_module_range().base) << "   "
    << std::left << insn_info->insn_copy.mnemonic << "\t"
    << insn_info->insn_copy.op_str;
    // 针对立即数跳转指令需要计算出其对应偏移
    if (cs_insn_group(insn_info->handle, &insn_info->insn_copy, CS_GRP_JUMP) ||
        cs_insn_group(insn_info->handle, &insn_info->insn_copy, CS_GRP_CALL) ||
        cs_insn_group(insn_info->handle, &insn_info->insn_copy, CS_GRP_RET)) {
        if (insn_info->detail_copy->arm64.operands[0].type == CS_OP_IMM) {
            outinfo << "(0x" << std::hex << insn_info->detail_copy->arm64.operands[0].imm - self->get_module_range().base << ")";
        }
    }
    outinfo << "   ;";


    std::stringstream memory_access_info;
    for (int i = 0; i < insn_info->detail_copy->arm64.op_count; i++) {
        cs_arm64_op &op = insn_info->detail_copy->arm64.operands[i];
        if (op.access & CS_AC_READ && op.type == ARM64_OP_REG) {
            uint64_t reg_value = 0;
            if (get_register_value(op.reg, ctx, reg_value)) {
                const char* reg_name = cs_reg_name( insn_info->handle, op.reg);
                outinfo << std::right << reg_name << " = 0x"
                        << std::left << std::hex << reg_value << ", ";
            }
        } else if (op.access & CS_AC_WRITE && op.type == ARM64_OP_REG) {
            self->write_reg_list.regs[self->write_reg_list.num++] = op.reg;
        } else if ((op.access & CS_AC_WRITE) && op.type == ARM64_OP_MEM) {
            uint64_t base = 0;
            uint64_t index = 0;
            bool found = true;
            if (op.mem.base != ARM64_REG_INVALID) {
                found = get_register_value(op.mem.base, ctx, base);
            }
            if (op.mem.index != ARM64_REG_INVALID) {
                found = get_register_value(op.mem.index, ctx, index);
            }
            if (found == true) {
                uintptr_t addr = base + index + op.mem.disp;
                memory_access_info << "   mem[w]:0x" << std::hex << addr;
            }
        } else if ((op.access & CS_AC_READ) && op.type == ARM64_OP_MEM) {
            uint64_t base = 0;
            uint64_t index = 0;
            bool found = true;
            if (op.mem.base != ARM64_REG_INVALID) {
                found = get_register_value(op.mem.base, ctx, base);
            }
            if (op.mem.index != ARM64_REG_INVALID) {
                found = get_register_value(op.mem.index, ctx, index);
            }
            if (found == true) {
                uintptr_t addr = base + index + op.mem.disp;
                memory_access_info << "   mem[r]:0x" << std::hex << addr;
            }
        }
    }
    outinfo << std::endl;

    // 解析函数调用信息
    uintptr_t jmp_addr = 0;
    if (insn_info->insn_copy.id == ARM64_INS_BL &&
        insn_info->detail_copy->arm64.operands[0].type == CS_OP_IMM) {
        jmp_addr = insn_info->detail_copy->arm64.operands[0].imm;
    } else if (insn_info->insn_copy.id == ARM64_INS_BLR &&
        insn_info->detail_copy->arm64.operands[0].type == CS_OP_REG) {
        get_register_value(insn_info->detail_copy->arm64.operands[0].reg, ctx, jmp_addr);
    } else if (insn_info->insn_copy.id == ARM64_INS_BR && self->is_plt_jmp &&
        insn_info->detail_copy->arm64.operands[0].type == CS_OP_REG) {
        self->is_plt_jmp = false;
        get_register_value(insn_info->detail_copy->arm64.operands[0].reg, ctx, jmp_addr);
    }
    if (jmp_addr != 0 && !self->is_address_in_module_range(jmp_addr)) {
        Dl_info dlInfo;
        if(dladdr(reinterpret_cast<const void *>(jmp_addr), &dlInfo) && dlInfo.dli_fname != nullptr){
            const char * soName = strrchr(dlInfo.dli_fname, '/') + 1;
            const char * symName =  gum_symbol_name_from_address((gpointer)jmp_addr);
            outinfo << "call addr: " << std::hex << jmp_addr << " [" << soName << "!" << symName << "]" << std::endl;
        }
    }
    // 开启打印 plt表的外部函数调用会很耗时
    /*
    else if (jmp_addr != 0 &&
        (jmp_addr - self->get_module_range().base) <= self->get_plt_range().second &&
        (jmp_addr - self->get_module_range().base) >= self->get_plt_range().first) {
        self->is_plt_jmp = true;
    }
    */

    // 打印内存读写信息
    if (!memory_access_info.str().empty()) {
        outinfo << memory_access_info.str() << std::endl;
    }

    // 写入日志文件
    self->get_logger_manager()->write_info(outinfo);
}

// typedef void (* GumEventSinkCallback) (const GumEvent * event, GumCpuContext * cpu_context, gpointer user_data);
void event_sink_callback(const GumEvent * event, GumCpuContext * cpu_context, gpointer user_data) {
    std::stringstream outinfo;
    auto self = InstructionTracerManager::get_instance();
    switch (event->type) {
        case GUM_CALL:
            if (!self->is_address_in_module_range((uintptr_t)event->call.target) && self->is_address_in_module_range(cpu_context->pc)) {
                outinfo << "call addr: " << "event_sink_callback" << std::endl;
            }
            break;
        default:
            break;
    }
    // 写入日志文件
    self->get_logger_manager()->write_info(outinfo);
}