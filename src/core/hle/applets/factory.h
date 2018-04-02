#pragma once

#include <functional>
#include <map>
#include <string>

namespace HLE {
namespace Applets {

template <typename CallbackType, typename ResultType, typename ConfigType>
class AppletFactory {
public:
    ~AppletFactory() {
        Clear();
    }

    void Clear() {
        callbacks.clear();
    }

    void Register(const std::string& name, CallbackType callback) {
        callbacks.emplace(std::move(name), std::move(callback));
    }

    bool IsRegistered(const std::string& name) const {
        auto it = callbacks.find(name);
        return it != callbacks.end();
    }

    ResultType Launch(const std::string& name, const ConfigType& config) {
        auto it = callbacks.find(name);
        if (it != callbacks.end())
            return it->second(config);
        return default_result;
    }

    ResultType default_result;

private:
    std::map<std::string, CallbackType> callbacks;
};

} // namespace Applets
} // namespace HLE
