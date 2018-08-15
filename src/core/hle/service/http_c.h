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
        auto itr = request_headers.find("Connection");
        bool header_keep_alive = (itr != request_headers.end()) && (itr->second == "Keep-Alive");
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
        std::unique_ptr<hl::Client> cli = nullptr;
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
        hl::Params params;
        hl::detail::parse_query_text(parsedUrl.m_Query, params);
        request.params = params;
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
    /**
     * HTTP_C::Initialize service function
     *  Inputs:
     *      1 : POST buffer size
     *      2 : 0x20
     *      3 : 0x0 (Filled with process ID by ARM11 Kernel)
     *      4 : 0x0
     *      5 : POST buffer memory block handle
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     */
    void Initialize(Kernel::HLERequestContext& ctx);

    /**
     * HTTP_C::CreateContext service function
     *  Inputs:
     *      1 : URL buffer size, including null-terminator
     *      2 : RequestMethod
     *      3 : (URLSize << 4) | 10
     *      4 : URL data pointer
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     *      2 : HTTP context handle
     */
    void CreateContext(Kernel::HLERequestContext& ctx);

    /**
     * HTTP_C::CloseContext service function
     *  Inputs:
     *      1 : HTTP context handle
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     */
    void CloseContext(Kernel::HLERequestContext& ctx);

    /**
     * HTTP_C::CloseContext service function
     *  Inputs:
     *      1 : HTTP context handle
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     *      2 : Output state
     */
    void GetRequestState(Kernel::HLERequestContext& ctx);

    /**
     * HTTP_C::GetDownloadSizeState service function
     *  Inputs:
     *      1 : HTTP context handle
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     *      2 : Total content data downloaded so far
     *      3 : Total content size from the "Content-Length" response header
     */
    void GetDownloadSizeState(Kernel::HLERequestContext& ctx);

    /**
     * HTTP_C::InitializeConnectionSession service function
     *  Inputs:
     *      1 : HTTP context handle
     *      2 : 0x20, processID translate-header for the ARM11-kernel
     *      3 : processID set by the ARM11-kerne
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     */
    void InitializeConnectionSession(Kernel::HLERequestContext& ctx);

    /**
     * HTTP_C::BeginRequest service function
     *  Inputs:
     *      1 : HTTP context handle
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     */
    void BeginRequest(Kernel::HLERequestContext& ctx);

    /**
     * HTTP_C::BeginRequestAsync service function
     *  Inputs:
     *      1 : HTTP context handle
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     */
    void BeginRequestAsync(Kernel::HLERequestContext& ctx);

    /**
     * HTTP_C::ReceiveData service function
     *  Inputs:
     *      1 : HTTP context handle
     *      2 : Buffer size
     *      3 : (OutSize << 4) | 12
     *      4 : Output data pointer
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     */
    void ReceiveData(Kernel::HLERequestContext& ctx);

    /**
     * HTTP_C::ReceiveDataTimeout service function
     *  Inputs:
     *      1 : HTTP context handle
     *      2 : Buffer size
     *    3-4 : u64 nanoseconds timeout
     *      5 : (OutSize << 4) | 12
     *      6 : Output data pointer
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     */
    void ReceiveDataTimeout(Kernel::HLERequestContext& ctx);

    /**
     * HTTP_C::SetProxyDefault service function
     *  Inputs:
     *      1 : HTTP context handle
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     */
    void SetProxyDefault(Kernel::HLERequestContext& ctx);

    /**
     * HTTP_C::SetSocketBufferSize service function
     *  Inputs:
     *      1 : HTTP context handle
     *      2 : u32 val
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     */
    void SetSocketBufferSize(Kernel::HLERequestContext& ctx);

    /**
     * HTTP_C::AddRequestHeader service function
     *  Inputs:
     *      1 : HTTP context handle
     *      2 : Header name buffer size, including null-terminator
     *      3 : Header value buffer size, including null-terminator
     *      4 : (HeaderNameSize << 14) | 0xC02
     *      5 : Header name data pointer
     *      6 : (HeaderValueSize << 4) | 10
     *      7 : Header value data pointer
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     */
    void AddRequestHeader(Kernel::HLERequestContext& ctx);

    /**
     * HTTP_C::AddPostDataRaw service function
     *  Inputs:
     *      1 : HTTP context handle
     *      2 : Length of raw data passed in buffer
     *      3 : (BufferLength << 14) | 0xC02
     *      4 : Raw data pointer
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     */
    void AddPostDataRaw(Kernel::HLERequestContext& ctx);

    /**
     * HTTP_C::GetResponseHeader service function
     *  Inputs:
     *      1 : HTTP context handle
     *      2 : Header name buffer size, including null-terminator.
     *      3 : Header value buffer max size, including null-terminator.
     *      4 : (HeaderNameSize << 14) | 0xC02
     *      5 : Header name input data pointer
     *      6 : (HeaderValueSize << 4) | 12
     *      7 : Header value output data pointer
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     *      2 : Total header value buffer size
     */
    void GetResponseHeader(Kernel::HLERequestContext& ctx);

    /**
     * HTTP_C::GetResponseStatusCode service function
     *  Inputs:
     *      1 : HTTP context handle
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     *      2 : HTTP response status code
     */
    void GetResponseStatusCode(Kernel::HLERequestContext& ctx);

    /**
     * HTTP_C::GetResponseStatusCodeTimeout service function
     *  Inputs:
     *      1 : HTTP context handle
     *    2-3 : u64 nanoseconds timeout
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     *      2 : HTTP response status code
     */
    void GetResponseStatusCodeTimeout(Kernel::HLERequestContext& ctx);

    /**
     * HTTP_C::SetSSLOpt service function
     *  Inputs:
     *      1 : HTTP context handle
     *      2 : u32 input_opt
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     */
    void SetSSLOpt(Kernel::HLERequestContext& ctx);

    /**
     * HTTP_C::SetKeepAlive service function
     *  Inputs:
     *      1 : HTTP context handle
     *      2 : bool keep_alive
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     */
    void SetKeepAlive(Kernel::HLERequestContext& ctx);

    Kernel::SharedPtr<Kernel::SharedMemory> shared_memory = nullptr;
    std::unordered_map<u32, Context> contexts;
    u32 context_counter{0};
};

void InstallInterfaces(SM::ServiceManager& service_manager);

} // namespace Service::HTTP
