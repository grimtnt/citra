// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <cstring>
#include "common/assert.h"
#include "core/core.h"
#include "core/core_timing.h"
#include "core/cpu/cpu.h"
#include "core/cpu/cpu_cp15.h"
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

class UserCallbacks final : public Dynarmic::A32::UserCallbacks {
public:
    explicit UserCallbacks(CPU& parent) : parent(parent) {
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
    ~UserCallbacks() = default;

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
        ASSERT_MSG(false, "Interpreter fallback (pc={}, num_instructions={})", static_cast<u32>(pc),
                   num_instructions);
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

    CPU& parent;
    u64 custom_ticks{};
    bool use_custom_ticks{};
};

CPU::CPU() : cb(std::make_unique<UserCallbacks>(*this)) {
    state = std::make_shared<State>();
    PageTableChanged();
}

CPU::~CPU() = default;

void CPU::Run() {
    ASSERT(Memory::GetCurrentPageTable() == current_page_table);

    jit->Run();
}

void CPU::SetPC(u32 pc) {
    jit->Regs()[15] = pc;
}

u32 CPU::GetPC() const {
    return jit->Regs()[15];
}

u32 CPU::GetReg(int index) const {
    return jit->Regs()[index];
}

void CPU::SetReg(int index, u32 value) {
    jit->Regs()[index] = value;
}

u32 CPU::GetVFPReg(int index) const {
    return jit->ExtRegs()[index];
}

void CPU::SetVFPReg(int index, u32 value) {
    jit->ExtRegs()[index] = value;
}

u32 CPU::GetVFPSystemReg(VFPSystemRegister reg) const {
    if (reg == VFP_FPSCR) {
        return jit->Fpscr();
    }

    // Dynarmic does not implement and/or expose other VFP registers
    return state->vfp[reg];
}

void CPU::SetVFPSystemReg(VFPSystemRegister reg, u32 value) {
    if (reg == VFP_FPSCR) {
        jit->SetFpscr(value);
    }

    // Dynarmic does not implement and/or expose other VFP registers, fallback to interpreter state
    state->vfp[reg] = value;
}

u32 CPU::GetCPSR() const {
    return jit->Cpsr();
}

void CPU::SetCPSR(u32 cpsr) {
    jit->SetCpsr(cpsr);
}

u32 CPU::GetCP15Register(CP15Register reg) {
    return state->cp15[reg];
}

void CPU::SetCP15Register(CP15Register reg, u32 value) {
    state->cp15[reg] = value;
}

std::unique_ptr<ThreadContext> CPU::NewContext() const {
    return std::make_unique<ThreadContext>();
}

void CPU::SaveContext(const std::unique_ptr<ThreadContext>& arg) {
    ThreadContext* ctx = dynamic_cast<ThreadContext*>(arg.get());
    ASSERT(ctx);

    jit->SaveContext(ctx->ctx);
    ctx->fpexc = state->vfp[VFP_FPEXC];
}

void CPU::LoadContext(const std::unique_ptr<ThreadContext>& arg) {
    const ThreadContext* ctx = dynamic_cast<ThreadContext*>(arg.get());
    ASSERT(ctx);

    jit->LoadContext(ctx->ctx);
    state->vfp[VFP_FPEXC] = ctx->fpexc;
}

void CPU::PrepareReschedule() {
    if (jit->IsExecuting()) {
        jit->HaltExecution();
    }
}

void CPU::InvalidateCacheRange(u32 start_address, size_t length) {
    jit->InvalidateCacheRange(start_address, length);
}

void CPU::PageTableChanged() {
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

void CPU::SyncSettings() {
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

std::unique_ptr<Dynarmic::A32::Jit> CPU::MakeJit() {
    Dynarmic::A32::UserConfig config;
    config.callbacks = cb.get();
    config.page_table = &current_page_table->pointers;
    config.coprocessors[15] = std::make_shared<CPUCP15>(state);
    return std::make_unique<Dynarmic::A32::Jit>(config);
}
