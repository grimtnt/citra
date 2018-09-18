// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/service.h"

namespace Service::CDC {

class CDC_HID final : public ServiceFramework<CDC_HID> {
public:
    CDC_HID();
    ~CDC_HID();
};

} // namespace Service::CDC
