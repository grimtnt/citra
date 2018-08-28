// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <cstring>
#include <dynarmic/A32/a32.h>
#include <dynarmic/A32/context.h>
#include "common/assert.h"
#include "core/arm/dynarmic/arm_dynarmic.h"
#include "core/arm/dynarmic/arm_dynarmic_cp15.h"
#include "core/arm/dyncom/arm_dyncom_interpreter.h"
#include "core/core.h"
#include "core/core_timing.h"
#include "core/hle/kernel/svc.h"
#include "core/memory.h"
#include "core/settings.h"

static std::unordered_map<u64, u64> custom_ticks_map = {
    {0x000400000008C300, 570},   {0x000400000008C400, 570},   {0x000400000008C500, 570},
    {0x0004000000126A00, 570},   {0x0004000000126B00, 570},   {0x0004000200120C01, 570},
    {0x000400000F700E00, 18000}, {0x0004000000055D00, 17000}, {0x0004000000055E00, 17000},
    {0x000400000011C400, 17000}, {0x000400000011C500, 17000}, {0x0004000000164800, 17000},
    {0x0004000000175E00, 17000}, {0x00040000001B5000, 17000}, {0x00040000001B5100, 17000},
    {0x00040000001BC500, 27000}, {0x00040000001BC600, 27000}, {0x000400000016E100, 27000},
    {0x0004000000055F00, 27000}, {0x0004000000076500, 27000}, {0x0004000000076400, 27000},
    {0x00040000000D0000, 27000},
};

class DynarmicThreadContext final : public ARM_Interface::ThreadContext {
public:
    DynarmicThreadContext() {
        Reset();
    }
    ~DynarmicThreadContext() override = default;

    void Reset() override {
        ctx.Regs() = {};
        ctx.SetCpsr(0);
        ctx.ExtRegs() = {};
        ctx.SetFpscr(0);
        fpexc = 0;
    }

    u32 GetCpuRegister(size_t index) const override {
        return ctx.Regs()[index];
    }
    void SetCpuRegister(size_t index, u32 value) override {
        ctx.Regs()[index] = value;
    }
    u32 GetCpsr() const override {
        return ctx.Cpsr();
    }
    void SetCpsr(u32 value) override {
        ctx.SetCpsr(value);
    }
    u32 GetFpuRegister(size_t index) const override {
        return ctx.ExtRegs()[index];
    }
    void SetFpuRegister(size_t index, u32 value) override {
        ctx.ExtRegs()[index] = value;
    }
    u32 GetFpscr() const override {
        return ctx.Fpscr();
    }
    void SetFpscr(u32 value) override {
        ctx.SetFpscr(value);
    }
    u32 GetFpexc() const override {
        return fpexc;
    }
    void SetFpexc(u32 value) override {
        fpexc = value;
    }

private:
    friend class ARM_Dynarmic;

    Dynarmic::A32::Context ctx;
    u32 fpexc;
};

class DynarmicUserCallbacks final : public Dynarmic::A32::UserCallbacks {
public:
    explicit DynarmicUserCallbacks(ARM_Dynarmic& parent) : parent(parent) {
        switch (Settings::values.ticks_mode) {
        case Settings::TicksMode::Custom: {
            custom_ticks = Settings::values.ticks;
            use_custom_ticks = true;
            break;
        }
        case Settings::TicksMode::Auto: {
            u64 program_id{};
            Core::System::GetInstance().GetAppLoader().ReadProgramId(program_id);
            auto itr = custom_ticks_map.find(program_id);
            if (itr != custom_ticks_map.end()) {
                custom_ticks = itr->second;
                use_custom_ticks = true;
            } else {
                custom_ticks = 0;
                use_custom_ticks = false;
            }
            break;
        }
        case Settings::TicksMode::Accurate: {
            custom_ticks = 0;
            use_custom_ticks = false;
            break;
        }
        }
    }
    ~DynarmicUserCallbacks() = default;

    std::uint8_t MemoryRead8(VAddr vaddr) override {
        return Memory::Read8(vaddr);
    }
    std::uint16_t MemoryRead16(VAddr vaddr) override {
        return Memory::Read16(vaddr);
    }
    std::uint32_t MemoryRead32(VAddr vaddr) override {
        return Memory::Read32(vaddr);
    }
    std::uint64_t MemoryRead64(VAddr vaddr) override {
        return Memory::Read64(vaddr);
    }

    void MemoryWrite8(VAddr vaddr, std::uint8_t value) override {
        Memory::Write8(vaddr, value);
    }
    void MemoryWrite16(VAddr vaddr, std::uint16_t value) override {
        Memory::Write16(vaddr, value);
    }
    void MemoryWrite32(VAddr vaddr, std::uint32_t value) override {
        Memory::Write32(vaddr, value);
    }
    void MemoryWrite64(VAddr vaddr, std::uint64_t value) override {
        Memory::Write64(vaddr, value);
    }

    void InterpreterFallback(VAddr pc, size_t num_instructions) override {
        parent.interpreter_state->Reg = parent.jit->Regs();
        parent.interpreter_state->Cpsr = parent.jit->Cpsr();
        parent.interpreter_state->Reg[15] = pc;
        parent.interpreter_state->ExtReg = parent.jit->ExtRegs();
        parent.interpreter_state->VFP[VFP_FPSCR] = parent.jit->Fpscr();
        parent.interpreter_state->NumInstrsToExecute = num_instructions;

        InterpreterMainLoop(parent.interpreter_state.get());

        bool is_thumb = (parent.interpreter_state->Cpsr & (1 << 5)) != 0;
        parent.interpreter_state->Reg[15] &= (is_thumb ? 0xFFFFFFFE : 0xFFFFFFFC);

        parent.jit->Regs() = parent.interpreter_state->Reg;
        parent.jit->SetCpsr(parent.interpreter_state->Cpsr);
        parent.jit->ExtRegs() = parent.interpreter_state->ExtReg;
        parent.jit->SetFpscr(parent.interpreter_state->VFP[VFP_FPSCR]);
    }

    void CallSVC(std::uint32_t swi) override {
        Kernel::CallSVC(swi);
    }

    void ExceptionRaised(VAddr pc, Dynarmic::A32::Exception exception) override {
        ASSERT_MSG(false, "ExceptionRaised(exception = {}, pc = {:08X}, code = {:08X})",
                   static_cast<size_t>(exception), pc, MemoryReadCode(pc));
    }

    void AddTicks(std::uint64_t ticks) override {
        CoreTiming::AddTicks(use_custom_ticks ? custom_ticks : ticks);
    }
    std::uint64_t GetTicksRemaining() override {
        s64 ticks = CoreTiming::GetDowncount();
        return static_cast<u64>(ticks <= 0 ? 0 : ticks);
    }

    ARM_Dynarmic& parent;
    u64 custom_ticks{};
    bool use_custom_ticks = false;
};

ARM_Dynarmic::ARM_Dynarmic(PrivilegeMode initial_mode)
    : cb(std::make_unique<DynarmicUserCallbacks>(*this)) {
    interpreter_state = std::make_shared<ARMul_State>(initial_mode);
    PageTableChanged();
}

ARM_Dynarmic::~ARM_Dynarmic() = default;

void ARM_Dynarmic::Run() {
    ASSERT(Memory::GetCurrentPageTable() == current_page_table);

    jit->Run();
}

void ARM_Dynarmic::SetPC(u32 pc) {
    jit->Regs()[15] = pc;
}

u32 ARM_Dynarmic::GetPC() const {
    return jit->Regs()[15];
}

u32 ARM_Dynarmic::GetReg(int index) const {
    return jit->Regs()[index];
}

void ARM_Dynarmic::SetReg(int index, u32 value) {
    jit->Regs()[index] = value;
}

u32 ARM_Dynarmic::GetVFPReg(int index) const {
    return jit->ExtRegs()[index];
}

void ARM_Dynarmic::SetVFPReg(int index, u32 value) {
    jit->ExtRegs()[index] = value;
}

u32 ARM_Dynarmic::GetVFPSystemReg(VFPSystemRegister reg) const {
    if (reg == VFP_FPSCR) {
        return jit->Fpscr();
    }

    // Dynarmic does not implement and/or expose other VFP registers, fallback to interpreter state
    return interpreter_state->VFP[reg];
}

void ARM_Dynarmic::SetVFPSystemReg(VFPSystemRegister reg, u32 value) {
    if (reg == VFP_FPSCR) {
        jit->SetFpscr(value);
    }

    // Dynarmic does not implement and/or expose other VFP registers, fallback to interpreter state
    interpreter_state->VFP[reg] = value;
}

u32 ARM_Dynarmic::GetCPSR() const {
    return jit->Cpsr();
}

void ARM_Dynarmic::SetCPSR(u32 cpsr) {
    jit->SetCpsr(cpsr);
}

u32 ARM_Dynarmic::GetCP15Register(CP15Register reg) {
    return interpreter_state->CP15[reg];
}

void ARM_Dynarmic::SetCP15Register(CP15Register reg, u32 value) {
    interpreter_state->CP15[reg] = value;
}

std::unique_ptr<ARM_Interface::ThreadContext> ARM_Dynarmic::NewContext() const {
    return std::make_unique<DynarmicThreadContext>();
}

void ARM_Dynarmic::SaveContext(const std::unique_ptr<ThreadContext>& arg) {
    DynarmicThreadContext* ctx = dynamic_cast<DynarmicThreadContext*>(arg.get());
    ASSERT(ctx);

    jit->SaveContext(ctx->ctx);
    ctx->fpexc = interpreter_state->VFP[VFP_FPEXC];
}

void ARM_Dynarmic::LoadContext(const std::unique_ptr<ThreadContext>& arg) {
    const DynarmicThreadContext* ctx = dynamic_cast<DynarmicThreadContext*>(arg.get());
    ASSERT(ctx);

    jit->LoadContext(ctx->ctx);
    interpreter_state->VFP[VFP_FPEXC] = ctx->fpexc;
}

void ARM_Dynarmic::PrepareReschedule() {
    if (jit->IsExecuting()) {
        jit->HaltExecution();
    }
}

void ARM_Dynarmic::ClearInstructionCache() {
    // TODO: Clear interpreter cache when appropriate.
    for (const auto& j : jits) {
        j.second->ClearCache();
    }
}

void ARM_Dynarmic::InvalidateCacheRange(u32 start_address, size_t length) {
    jit->InvalidateCacheRange(start_address, length);
}

void ARM_Dynarmic::PageTableChanged() {
    current_page_table = Memory::GetCurrentPageTable();

    auto iter = jits.find(current_page_table);
    if (iter != jits.end()) {
        jit = iter->second.get();
        return;
    }

    auto new_jit = MakeJit();
    jit = new_jit.get();
    jits.emplace(current_page_table, std::move(new_jit));
}

void ARM_Dynarmic::SyncSettings() {
    cb->custom_ticks = Settings::values.ticks;
    cb->use_custom_ticks = Settings::values.ticks_mode != Settings::TicksMode::Accurate;
    switch (Settings::values.ticks_mode) {
    case Settings::TicksMode::Custom: {
        cb->custom_ticks = Settings::values.ticks;
        cb->use_custom_ticks = true;
        break;
    }
    case Settings::TicksMode::Auto: {
        u64 program_id{};
        Core::System::GetInstance().GetAppLoader().ReadProgramId(program_id);
        auto itr = custom_ticks_map.find(program_id);
        if (itr != custom_ticks_map.end()) {
            cb->custom_ticks = itr->second;
            cb->use_custom_ticks = true;
        } else {
            cb->custom_ticks = 0;
            cb->use_custom_ticks = false;
        }
        break;
    }
    case Settings::TicksMode::Accurate: {
        cb->custom_ticks = 0;
        cb->use_custom_ticks = false;
        break;
    }
    }
}

std::unique_ptr<Dynarmic::A32::Jit> ARM_Dynarmic::MakeJit() {
    Dynarmic::A32::UserConfig config;
    config.callbacks = cb.get();
    config.page_table = &current_page_table->pointers;
    config.coprocessors[15] = std::make_shared<DynarmicCP15>(interpreter_state);
    return std::make_unique<Dynarmic::A32::Jit>(config);
}
