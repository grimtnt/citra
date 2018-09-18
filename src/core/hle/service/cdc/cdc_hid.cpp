// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/cdc/cdc_hid.h"

namespace Service::CDC {

CDC_HID::CDC_HID() : ServiceFramework("cdc:HID") {}

CDC_HID::~CDC_HID() = default;

} // namespace Service::CDC
