// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <LUrlParser.h>
#include <httplib.h>
#include "core/hle/service/service.h"

namespace Service::HTTP {

enum class RequestMethod : u8 {
    Get = 0x1,
    Post = 0x2,
    Head = 0x3,
    Put = 0x4,
    Delete = 0x5,
    PostEmpty = 0x6,
    PutEmpty = 0x7,
};

struct Context {
    void SetUrl(std::string url) {
        this->url = std::move(url);
    }

    void SetMethod(RequestMethod method) {
        this->method = method;
    }

    void SetKeepAlive(bool keep_alive) {
        auto itr{request_headers.find("Connection")};
        bool header_keep_alive{(itr != request_headers.end()) && (itr->second == "Keep-Alive")};
        if (keep_alive && !header_keep_alive) {
            request_headers.emplace("Connection", "Keep-Alive");
        } else if (!keep_alive && header_keep_alive) {
            request_headers.erase("Connection");
        }
    }

    void SetSSLOptions(u32 ssl_options) {
        if ((ssl_options & 0x200) == 0x200) {
            disable_verify = true;
            LOG_WARNING(Service_HTTP, "Disable verify requested");
        }
        if ((ssl_options & 0x800) == 0x800) {
            LOG_WARNING(Service_HTTP, "TLS v1.0 requested, unimplemented.");
        }
    }

    u32 GetResponseStatusCode() const {
        return response ? response->status : 0;
    }

    u32 GetResponseContentLength() const {
        try {
            const std::string length = response->get_header_value("Content-Length");
            return std::stoi(length);
        } catch (...) {
            return 0;
        }
    }

    void Send() {
        using lup = LUrlParser::clParseURL;
        namespace hl = httplib;
        lup parsedUrl = lup::ParseURL(url);
        std::unique_ptr<hl::Client> cli{};
        int port;
        if (parsedUrl.m_Scheme == "http") {
            if (!parsedUrl.GetPort(&port)) {
                port = 80;
            }
            cli = std::make_unique<hl::Client>(parsedUrl.m_Host.c_str(), port,
                                               (timeout == 0) ? 300 : (timeout * std::pow(10, -9)));
        } else if (parsedUrl.m_Scheme == "https") {
            if (!parsedUrl.GetPort(&port)) {
                port = 443;
            }
            cli = std::make_unique<hl::SSLClient>(parsedUrl.m_Host.c_str(), port,
                                                  (timeout == 0) ? 300
                                                                 : (timeout * std::pow(10, -9)));
        } else {
            UNREACHABLE_MSG("Invalid scheme!");
        }
        if (disable_verify) {
            cli->set_verify(hl::SSLVerifyMode::None);
        }
        hl::Request request;
        static const std::unordered_map<RequestMethod, std::string> methods_map_strings = {
            {RequestMethod::Get, "GET"},       {RequestMethod::Post, "POST"},
            {RequestMethod::Head, "HEAD"},     {RequestMethod::Put, "PUT"},
            {RequestMethod::Delete, "DELETE"}, {RequestMethod::PostEmpty, "POST"},
            {RequestMethod::PutEmpty, "PUT"},
        };
        static const std::unordered_map<RequestMethod, bool> methods_map_body = {
            {RequestMethod::Get, false},      {RequestMethod::Post, true},
            {RequestMethod::Head, false},     {RequestMethod::Put, true},
            {RequestMethod::Delete, true},    {RequestMethod::PostEmpty, false},
            {RequestMethod::PutEmpty, false},
        };
        request.method = methods_map_strings.find(method)->second;
        request.path = '/' + parsedUrl.m_Path;
        request.headers = request_headers;
        if (methods_map_body.find(method)->second) {
            request.body = body;
        }
        hl::detail::parse_query_text(parsedUrl.m_Query, request.params);
        response = std::make_shared<hl::Response>();
        cli->send(request, *response);
    }

    void Initialize() {
        current_offset = 0;
        response = nullptr;
        disable_verify = false;
        proxy_default = false;
        timeout = 0;
        state = State::NotStarted;
    }

    enum class State : u32 {
        NotStarted = 0x1,             // Request has not started yet.
        InProgress = 0x5,             // Request in progress, sending request over the network.
        ReadyToDownloadContent = 0x7, // Ready to download the content. (needs verification)
        ReadyToDownload = 0x8,        // Ready to download?
        TimedOut = 0xA,               // Request timed out?
    };

    u32 current_offset;
    httplib::Headers request_headers;
    std::string url;
    std::shared_ptr<httplib::Response> response;
    RequestMethod method;
    bool disable_verify;
    bool proxy_default;
    u64 timeout;
    State state;
    std::string body;
};

class HTTP_C final : public ServiceFramework<HTTP_C> {
public:
    HTTP_C();

private:
    void Initialize(Kernel::HLERequestContext& ctx);
    void CreateContext(Kernel::HLERequestContext& ctx);
    void CloseContext(Kernel::HLERequestContext& ctx);
    void GetRequestState(Kernel::HLERequestContext& ctx);
    void GetDownloadSizeState(Kernel::HLERequestContext& ctx);
    void InitializeConnectionSession(Kernel::HLERequestContext& ctx);
    void BeginRequest(Kernel::HLERequestContext& ctx);
    void BeginRequestAsync(Kernel::HLERequestContext& ctx);
    void ReceiveData(Kernel::HLERequestContext& ctx);
    void ReceiveDataTimeout(Kernel::HLERequestContext& ctx);
    void SetProxyDefault(Kernel::HLERequestContext& ctx);
    void SetSocketBufferSize(Kernel::HLERequestContext& ctx);
    void AddRequestHeader(Kernel::HLERequestContext& ctx);
    void AddPostDataRaw(Kernel::HLERequestContext& ctx);
    void GetResponseHeader(Kernel::HLERequestContext& ctx);
    void GetResponseStatusCode(Kernel::HLERequestContext& ctx);
    void GetResponseStatusCodeTimeout(Kernel::HLERequestContext& ctx);
    void SetSSLOpt(Kernel::HLERequestContext& ctx);
    void SetKeepAlive(Kernel::HLERequestContext& ctx);
    void Finalize(Kernel::HLERequestContext& ctx);

    Kernel::SharedPtr<Kernel::SharedMemory> shared_memory{};
    std::unordered_map<u32, Context> contexts;
    u32 context_counter{};
};

void InstallInterfaces(SM::ServiceManager& service_manager);

} // namespace Service::HTTP
