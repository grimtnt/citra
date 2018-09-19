// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include "core/hle/service/service.h"

namespace Service::MIC {

class MIC_U final : public ServiceFramework<MIC_U> {
public:
    MIC_U();
    ~MIC_U();
    void ReloadDevice();

private:
    /**
     * MIC::MapSharedMem service function
     *  Inputs:
     *      1: Shared-mem size
     *      2: CopyHandleDesc
     *      3: Shared-mem handle
     *  Outputs:
     *      1: Result of function, 0 on success, otherwise error code
     */
    void MapSharedMem(Kernel::HLERequestContext& ctx);

    /**
     * MIC::UnmapSharedMem service function
     *  Outputs:
     *      1: Result of function, 0 on success, otherwise error code
     */
    void UnmapSharedMem(Kernel::HLERequestContext& ctx);

    /**
     * MIC::StartSampling service function
     *  Inputs:
     *      1: Encoding
     *      2: SampleRate
     *      3: Base offset for audio data in sharedmem
     *      4: Size of the audio data in sharedmem
     *      5: Loop at end of buffer
     *  Outputs:
     *      1: Result of function, 0 on success, otherwise error code
     */
    void StartSampling(Kernel::HLERequestContext& ctx);

    /**
     * MIC::AdjustSampling service function
     *  Inputs:
     *      1: SampleRate
     *  Outputs:
     *      1: Result of function, 0 on success, otherwise error code
     */
    void AdjustSampling(Kernel::HLERequestContext& ctx);

    /**
     * MIC::StopSampling service function
     *  Outputs:
     *      1: Result of function, 0 on success, otherwise error code
     */
    void StopSampling(Kernel::HLERequestContext& ctx);

    /**
     * MIC::IsSampling service function
     *  Outputs:
     *      1: Result of function, 0 on success, otherwise error code
     *      2: 0 = sampling, non-zero = sampling
     */
    void IsSampling(Kernel::HLERequestContext& ctx);

    /**
     * MIC::GetBufferFullEvent service function
     *  Outputs:
     *      1: Result of function, 0 on success, otherwise error code
     *      3: Event handle
     */
    void GetBufferFullEvent(Kernel::HLERequestContext& ctx);

    /**
     * MIC::SetGain service function
     *  Inputs:
     *      1: Gain
     *  Outputs:
     *      1: Result of function, 0 on success, otherwise error code
     */
    void SetGain(Kernel::HLERequestContext& ctx);

    /**
     * MIC::GetGain service function
     *  Outputs:
     *      1: Result of function, 0 on success, otherwise error code
     *      2: Gain
     */
    void GetGain(Kernel::HLERequestContext& ctx);

    /**
     * MIC::SetPower service function
     *  Inputs:
     *      1: Power (0 = off, 1 = on)
     *  Outputs:
     *      1: Result of function, 0 on success, otherwise error code
     */
    void SetPower(Kernel::HLERequestContext& ctx);

    /**
     * MIC::GetPower service function
     *  Outputs:
     *      1: Result of function, 0 on success, otherwise error code
     *      2: Power
     */
    void GetPower(Kernel::HLERequestContext& ctx);

    /**
     * MIC::SetIirFilterMic service function
     *  Inputs:
     *      1: Size
     *      2: (Size << 4) | 0xA
     *      3: Pointer to IIR Filter Data
     *  Outputs:
     *      1: Result of function, 0 on success, otherwise error code
     */
    void SetIirFilterMic(Kernel::HLERequestContext& ctx);

    /**
     * MIC::SetClamp service function
     *  Inputs:
     *      1: Clamp (0 = don't clamp, non-zero = clamp)
     *  Outputs:
     *      1: Result of function, 0 on success, otherwise error code
     */
    void SetClamp(Kernel::HLERequestContext& ctx);

    /**
     * MIC::GetClamp service function
     *  Outputs:
     *      1: Result of function, 0 on success, otherwise error code
     *      2: Clamp (0 = don't clamp, non-zero = clamp)
     */
    void GetClamp(Kernel::HLERequestContext& ctx);

    /**
     * MIC::SetAllowShellClosed service function
     *  Inputs:
     *      1: Sampling allowed while shell closed (0 = disallow, non-zero = allow)
     *  Outputs:
     *      1: Result of function, 0 on success, otherwise error code
     */
    void SetAllowShellClosed(Kernel::HLERequestContext& ctx);

    /**
     * MIC_U::SetClientVersion service function
     *  Inputs:
     *      1: Used SDK Version
     *  Outputs:
     *      1: Result of function, 0 on success, otherwise error code
     */
    void SetClientVersion(Kernel::HLERequestContext& ctx);

    struct Impl;
    std::unique_ptr<Impl> impl;
};

void InstallInterfaces(SM::ServiceManager& service_manager);
void ReloadDevice();

} // namespace Service::MIC