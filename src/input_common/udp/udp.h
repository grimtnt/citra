// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <memory>
#include <unordered_map>
#include "input_common/main.h"
#include "input_common/udp/client.h"

namespace InputCommon {
namespace UDP {

class UDPTouchDevice;
class UDPMotionDevice;

class State {
public:
    State();
    ~State();

private:
    std::unique_ptr<Client> client;
};

std::unique_ptr<State> Init();

namespace Polling {

/// Get all DevicePoller that use the SDL backend for a specific device type
void GetPollers(InputCommon::Polling::DeviceType type,
                std::vector<std::unique_ptr<InputCommon::Polling::DevicePoller>>& pollers);

} // namespace Polling
} // namespace UDP
} // namespace InputCommon
