#pragma once

#include <functional>
#include <map>
#include <string>
#include <utility>

namespace HLE {
namespace Applets {

template <typename CallbackType, typename ResultType, typename ConfigType>
/**
 * @brief Factory class for applets
 */
class AppletFactory {
public:
    /**
     * @brief IsRegistered Checks if a callback is registered
     * @param name The name of the callback
     * @return true if the callback is registered, otherwise false
     */
    bool IsRegistered(const std::string& name) const {
        auto it = callbacks.find(name);
        return it != callbacks.end();
    }

    /**
     * @brief Register Registers a callback
     * @param name Name of the callback
     * @param callback The callback
     */
    void Register(std::string name, CallbackType callback) {
        callbacks.emplace(std::move(name), std::move(callback));
    }

    /**
     * @brief Launch Launches a applet
     * @param name The callback name
     * @param config The configuration of the applet
     * @return The result of the callback
     */
    ResultType Launch(const std::string& name, const ConfigType& config) {
        auto it = callbacks.find(name);
        if (it != callbacks.end())
            return it->second(config);
        return default_result;
    }

protected:
    /// Result of Launch if the callback was not found
    ResultType default_result;

private:
    /// Registered callbacks
    std::map<std::string, CallbackType> callbacks;
};
} // namespace Applets
} // namespace HLE
