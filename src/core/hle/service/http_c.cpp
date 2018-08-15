// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <future>
#include <map>
#include <string>
#include <LUrlParser.h>
#include <httplib.h>
#include "core/hle/ipc_helpers.h"
#include "core/hle/kernel/ipc.h"
#include "core/hle/kernel/shared_memory.h"
#include "core/hle/service/http_c.h"

namespace Service {
namespace HTTP {

const ResultCode ERROR_CONTEXT_ERROR = // 0xD8A0A066
    ResultCode(static_cast<ErrorDescription>(102), ErrorModule::HTTP, ErrorSummary::InvalidState,
               ErrorLevel::Permanent);
const ResultCode RESULT_DOWNLOADPENDING = // 0xD840A02B
    ResultCode(static_cast<ErrorDescription>(43), ErrorModule::HTTP, ErrorSummary::WouldBlock,
               ErrorLevel::Permanent);

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

class HTTP_C::Impl {
public:
    ~Impl();
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

private:
    Kernel::SharedPtr<Kernel::SharedMemory> shared_memory = nullptr;
    std::unordered_map<u32, Context> contexts;
    u32 context_counter{0};
};

HTTP_C::Impl::~Impl() {
    shared_memory = nullptr;
}

void HTTP_C::Impl::Initialize(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x1, 1, 4);
    const u32 shmem_size = rp.Pop<u32>();
    rp.PopPID();
    shared_memory = rp.PopObject<Kernel::SharedMemory>();
    if (shared_memory) {
        shared_memory->name = "HTTP_C:shared_memory";
    }

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
    rb.Push(RESULT_SUCCESS);

    LOG_WARNING(Service_HTTP, "called, shmem_size={}", shmem_size);
}

void HTTP_C::Impl::CreateContext(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x2, 2, 2);
    const u32 url_size = rp.Pop<u32>();
    std::string url(url_size, '\0');
    RequestMethod method = rp.PopEnum<RequestMethod>();

    Kernel::MappedBuffer& buffer = rp.PopMappedBuffer();
    buffer.Read(&url[0], 0, url_size - 1);

    Context context;
    context.SetUrl(url);
    context.SetMethod(method);
    contexts[++context_counter] = std::move(context);

    IPC::RequestBuilder rb = rp.MakeBuilder(2, 2);
    rb.Push(RESULT_SUCCESS);
    rb.Push<u32>(context_counter);
    rb.PushMappedBuffer(buffer);

    LOG_WARNING(Service_HTTP, "called, url_size={}, url={}", url_size, url);
}

void HTTP_C::Impl::CloseContext(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x3, 1, 0);
    const u32 context_id = rp.Pop<u32>();

    if (contexts.find(context_id) == contexts.end()) {
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ERROR_CONTEXT_ERROR);
        LOG_ERROR(Service_HTTP, "called, context {} not found", context_id);
        return;
    }
    contexts.erase(context_id);

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
    rb.Push(RESULT_SUCCESS);

    LOG_WARNING(Service_HTTP, "called, context_id={}", context_id);
}

void HTTP_C::Impl::GetRequestState(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x5, 1, 0);
    const u32 context_id = rp.Pop<u32>();

    const auto context = contexts.find(context_id);
    if (context == contexts.end()) {
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ERROR_CONTEXT_ERROR);
        LOG_ERROR(Service_HTTP, "called, context {} not found", context_id);
        return;
    }

    IPC::RequestBuilder rb = rp.MakeBuilder(2, 0);
    rb.Push(RESULT_SUCCESS);
    rb.PushEnum<Context::State>(context->second.state);

    LOG_WARNING(Service_HTTP, "called, context_id={}", context_id);
}

void HTTP_C::Impl::GetDownloadSizeState(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x6, 1, 0);
    const u32 context_id = rp.Pop<u32>();

    const auto context = contexts.find(context_id);
    if (context == contexts.end()) {
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ERROR_CONTEXT_ERROR);
        LOG_ERROR(Service_HTTP, "called, context {} not found", context_id);
        return;
    }

    IPC::RequestBuilder rb = rp.MakeBuilder(3, 0);
    rb.Push(RESULT_SUCCESS);
    rb.Push<u32>(context->second.current_offset);
    rb.Push<u32>(context->second.GetResponseContentLength());

    LOG_WARNING(Service_HTTP, "called, context_id={}", context_id);
}

void HTTP_C::Impl::InitializeConnectionSession(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x8, 1, 2);
    const u32 context_id = rp.Pop<u32>();
    rp.PopPID();

    const auto context = contexts.find(context_id);
    if (context == contexts.end()) {
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ERROR_CONTEXT_ERROR);
        LOG_ERROR(Service_HTTP, "called, context {} not found", context_id);
        return;
    }
    context->second.Initialize();

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
    rb.Push(RESULT_SUCCESS);

    LOG_WARNING(Service_HTTP, "called, context_id={}", context_id);
}

void HTTP_C::Impl::BeginRequest(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x9, 1, 0);
    const u32 context_id = rp.Pop<u32>();

    auto context = contexts.find(context_id);
    if (context == contexts.end()) {
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ERROR_CONTEXT_ERROR);
        LOG_ERROR(Service_HTTP, "called, context {} not found", context_id);
        return;
    }

    std::async(std::launch::async, [&] {
        context->second.state = Context::State::InProgress;
        context->second.Send();
        context->second.state = Context::State::ReadyToDownloadContent;
    });

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
    rb.Push(RESULT_SUCCESS);

    LOG_WARNING(Service_HTTP, "called, context_id={}", context_id);
}

void HTTP_C::Impl::BeginRequestAsync(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x9, 1, 0);
    const u32 context_id = rp.Pop<u32>();

    auto context = contexts.find(context_id);
    if (context == contexts.end()) {
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ERROR_CONTEXT_ERROR);
        LOG_ERROR(Service_HTTP, "called, context {} not found", context_id);
        return;
    }

    std::async(std::launch::async, [&] {
        context->second.state = Context::State::InProgress;
        context->second.Send();
        context->second.state = Context::State::ReadyToDownloadContent;
    });

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
    rb.Push(RESULT_SUCCESS);

    LOG_WARNING(Service_HTTP, "called, context_id={}", context_id);
}

void HTTP_C::Impl::ReceiveData(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0xB, 2, 2);
    const u32 context_id = rp.Pop<u32>();
    const u32 buffer_size = rp.Pop<u32>();
    Kernel::MappedBuffer& buffer = rp.PopMappedBuffer();

    auto context = contexts.find(context_id);
    if (context == contexts.end()) {
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ERROR_CONTEXT_ERROR);
        LOG_ERROR(Service_HTTP, "called, context {} not found", context_id);
        return;
    }
    const u32 size = std::min(buffer_size, context->second.GetResponseContentLength() -
                                               context->second.current_offset);
    buffer.Write(&context->second.response->body[context->second.current_offset], 0, size);
    context->second.current_offset += size;

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 2);
    rb.Push(context->second.current_offset < context->second.GetResponseContentLength()
                ? RESULT_DOWNLOADPENDING
                : RESULT_SUCCESS);
    rb.PushMappedBuffer(buffer);

    LOG_WARNING(Service_HTTP, "called, context_id={}", context_id);
}

void HTTP_C::Impl::ReceiveDataTimeout(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0xC, 4, 2);
    const u32 context_id = rp.Pop<u32>();
    const u32 buffer_size = rp.Pop<u32>();
    const u64 timeout = rp.Pop<u64>();
    Kernel::MappedBuffer& buffer = rp.PopMappedBuffer();

    auto context = contexts.find(context_id);
    if (context == contexts.end()) {
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ERROR_CONTEXT_ERROR);
        LOG_ERROR(Service_HTTP, "called, context {} not found", context_id);
        return;
    }
    context->second.timeout = timeout;
    const u32 size = std::min(buffer_size, context->second.GetResponseContentLength() -
                                               context->second.current_offset);
    buffer.Write(&context->second.response->body[context->second.current_offset], 0, size);
    context->second.current_offset += size;

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 2);
    rb.Push(context->second.current_offset < context->second.GetResponseContentLength()
                ? RESULT_DOWNLOADPENDING
                : RESULT_SUCCESS);
    rb.PushMappedBuffer(buffer);

    LOG_WARNING(Service_HTTP, "called, context_id={}", context_id);
}

void HTTP_C::Impl::SetProxyDefault(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0xE, 1, 0);
    const u32 context_id = rp.Pop<u32>();

    const auto context = contexts.find(context_id);
    if (context == contexts.end()) {
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ERROR_CONTEXT_ERROR);
        LOG_ERROR(Service_HTTP, "called, context {} not found", context_id);
        return;
    }
    context->second.proxy_default = true;

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
    rb.Push(RESULT_SUCCESS);

    LOG_WARNING(Service_HTTP, "called, context_id={}", context_id);
}

void HTTP_C::Impl::SetSocketBufferSize(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x10, 2, 0);
    const u32 context_id = rp.Pop<u32>();
    const u32 val = rp.Pop<u32>();

    const auto context = contexts.find(context_id);
    if (context == contexts.end()) {
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ERROR_CONTEXT_ERROR);
        LOG_ERROR(Service_HTTP, "called, context {} not found", context_id);
        return;
    }

    // TODO: Implement

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
    rb.Push(RESULT_SUCCESS);

    LOG_WARNING(Service_HTTP, "(STUBBED) called, val={}", val);
}

void HTTP_C::Impl::AddRequestHeader(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x11, 3, 4);
    const u32 context_id = rp.Pop<u32>();
    const u32 name_size = rp.Pop<u32>();
    const u32 value_size = rp.Pop<u32>();
    const std::vector<u8> name_buffer = rp.PopStaticBuffer();
    Kernel::MappedBuffer& value_buffer = rp.PopMappedBuffer();
    const std::string name(name_buffer.begin(), name_buffer.end() - 1);
    std::string value(value_size - 1, '\0');
    value_buffer.Read(&value[0], 0, value_size - 1);

    auto context = contexts.find(context_id);
    if (context == contexts.end()) {
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ERROR_CONTEXT_ERROR);
        LOG_ERROR(Service_HTTP, "called, context {} not found", context_id);
        return;
    }
    context->second.request_headers.emplace(name, value);

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 2);
    rb.Push(RESULT_SUCCESS);
    rb.PushMappedBuffer(value_buffer);

    LOG_WARNING(Service_HTTP, "called, name={}, value={}, context_id={}", name, value, context_id);
}

void HTTP_C::Impl::AddPostDataRaw(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x14, 2, 2);
    const u32 context_id = rp.Pop<u32>();
    const u32 length = rp.Pop<u32>();
    auto buffer = rp.PopMappedBuffer();
    std::string data(length, '\0');
    buffer.Read(&data[0], 0, length);
    auto context = contexts.find(context_id);
    if (context == contexts.end()) {
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ERROR_CONTEXT_ERROR);
        LOG_ERROR(Service_HTTP, "called, context {} not found", context_id);
        return;
    }
    context->second.body = data;

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 2);
    rb.Push(RESULT_SUCCESS);
    rb.PushMappedBuffer(buffer);

    LOG_WARNING(Service_HTTP, "called, context_id={}, data={}", context_id, data);
}

void HTTP_C::Impl::GetResponseHeader(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x1E, 3, 4);
    const u32 context_id = rp.Pop<u32>();
    const u32 name_size = rp.Pop<u32>();
    const u32 value_size = rp.Pop<u32>();
    const std::vector<u8> name_buffer = rp.PopStaticBuffer();
    Kernel::MappedBuffer& value_buffer = rp.PopMappedBuffer();
    const std::string name(name_buffer.begin(), name_buffer.end() - 1);

    auto context = contexts.find(context_id);
    if (context == contexts.end()) {
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ERROR_CONTEXT_ERROR);
        LOG_ERROR(Service_HTTP, "called, context {} not found", context_id);
        return;
    }
    auto itr = context->second.response->headers.find(name);
    bool has_header = itr != context->second.response->headers.end();
    const std::string value = has_header ? itr->second + '\0' : "";

    u32 size = static_cast<u32>(value.size());
    value_buffer.Write(value.c_str(), 0, size);

    IPC::RequestBuilder rb = rp.MakeBuilder(2, 2);
    rb.Push(RESULT_SUCCESS);
    rb.Push<u32>(size);
    rb.PushMappedBuffer(value_buffer);

    LOG_WARNING(Service_HTTP,
                "called, name={}, name_size={}, value={}, value_size={}, context_id={}", name,
                name_size, value, value_size, context_id);
}

void HTTP_C::Impl::GetResponseStatusCode(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x22, 1, 0);
    const u32 context_id = rp.Pop<u32>();

    const auto context = contexts.find(context_id);
    if (context == contexts.end()) {
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ERROR_CONTEXT_ERROR);
        LOG_ERROR(Service_HTTP, "called, context {} not found", context_id);
        return;
    }
    const u32 status_code = context->second.GetResponseStatusCode();

    IPC::RequestBuilder rb = rp.MakeBuilder(2, 0);
    rb.Push(RESULT_SUCCESS);
    rb.Push<u32>(status_code);

    LOG_WARNING(Service_HTTP, "called, context_id={}, status_code={}", context_id, status_code);
}

void HTTP_C::Impl::GetResponseStatusCodeTimeout(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x23, 3, 0);
    const u32 context_id = rp.Pop<u32>();
    const u64 timeout = rp.Pop<u64>();

    const auto context = contexts.find(context_id);
    if (context == contexts.end()) {
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ERROR_CONTEXT_ERROR);
        LOG_ERROR(Service_HTTP, "called, context {} not found", context_id);
        return;
    }
    context->second.timeout = timeout;
    IPC::RequestBuilder rb = rp.MakeBuilder(2, 0);
    rb.Push(RESULT_SUCCESS);
    rb.Push<u32>(contexts[context_id].GetResponseStatusCode());

    LOG_WARNING(Service_HTTP, "called, context_id={}, timeout={}", context_id, timeout);
}

void HTTP_C::Impl::SetSSLOpt(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x2B, 2, 0);
    const u32 context_id = rp.Pop<u32>();
    const u32 ssl_options = rp.Pop<u32>();

    const auto context = contexts.find(context_id);
    if (context == contexts.end()) {
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ERROR_CONTEXT_ERROR);
        LOG_ERROR(Service_HTTP, "called, context {} not found", context_id);
        return;
    }
    context->second.SetSSLOptions(ssl_options);

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
    rb.Push(RESULT_SUCCESS);

    LOG_WARNING(Service_HTTP, "called, context_id={}, ssl_options=0x{:X}", context_id, ssl_options);
}

void HTTP_C::Impl::SetKeepAlive(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x37, 2, 0);
    const u32 context_id = rp.Pop<u32>();
    const bool keep_alive = rp.Pop<bool>();

    const auto context = contexts.find(context_id);
    if (context == contexts.end()) {
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ERROR_CONTEXT_ERROR);
        LOG_ERROR(Service_HTTP, "called, context {} not found", context_id);
        return;
    }
    context->second.SetKeepAlive(keep_alive);

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
    rb.Push(RESULT_SUCCESS);

    LOG_WARNING(Service_HTTP, "called, context_id={}, keep_alive={}", context_id, keep_alive);
}

HTTP_C::HTTP_C() : ServiceFramework("http:C", 32), pimpl{new Impl{}} {
    static const FunctionInfo functions[] = {
        {0x00010044, &HTTP_C::Initialize, "Initialize"},
        {0x00020082, &HTTP_C::CreateContext, "CreateContext"},
        {0x00030040, &HTTP_C::CloseContext, "CloseContext"},
        {0x00040040, nullptr, "CancelConnection"},
        {0x00050040, &HTTP_C::GetRequestState, "GetRequestState"},
        {0x00060040, &HTTP_C::GetDownloadSizeState, "GetDownloadSizeState"},
        {0x00070040, nullptr, "GetRequestError"},
        {0x00080042, &HTTP_C::InitializeConnectionSession, "InitializeConnectionSession"},
        {0x00090040, &HTTP_C::BeginRequest, "BeginRequest"},
        {0x000A0040, &HTTP_C::BeginRequestAsync, "BeginRequestAsync"},
        {0x000B0082, &HTTP_C::ReceiveData, "ReceiveData"},
        {0x000C0102, &HTTP_C::ReceiveDataTimeout, "ReceiveDataTimeout"},
        {0x000D0146, nullptr, "SetProxy"},
        {0x000E0040, &HTTP_C::SetProxyDefault, "SetProxyDefault"},
        {0x000F00C4, nullptr, "SetBasicAuthorization"},
        {0x00100080, &HTTP_C::SetSocketBufferSize, "SetSocketBufferSize"},
        {0x001100C4, &HTTP_C::AddRequestHeader, "AddRequestHeader"},
        {0x001200C4, nullptr, "AddPostDataAscii"},
        {0x001300C4, nullptr, "AddPostDataBinary"},
        {0x00140082, &HTTP_C::AddPostDataRaw, "AddPostDataRaw"},
        {0x00150080, nullptr, "SetPostDataType"},
        {0x001600C4, nullptr, "SendPostDataAscii"},
        {0x00170144, nullptr, "SendPostDataAsciiTimeout"},
        {0x001800C4, nullptr, "SendPostDataBinary"},
        {0x00190144, nullptr, "SendPostDataBinaryTimeout"},
        {0x001A0082, nullptr, "SendPostDataRaw"},
        {0x001B0102, nullptr, "SendPOSTDataRawTimeout"},
        {0x001C0080, nullptr, "SetPostDataEncoding"},
        {0x001D0040, nullptr, "NotifyFinishSendPostData"},
        {0x001E00C4, &HTTP_C::GetResponseHeader, "GetResponseHeader"},
        {0x001F0144, nullptr, "GetResponseHeaderTimeout"},
        {0x00200082, nullptr, "GetResponseData"},
        {0x00210102, nullptr, "GetResponseDataTimeout"},
        {0x00220040, &HTTP_C::GetResponseStatusCode, "GetResponseStatusCode"},
        {0x002300C0, &HTTP_C::GetResponseStatusCodeTimeout, "GetResponseStatusCodeTimeout"},
        {0x00240082, nullptr, "AddTrustedRootCA"},
        {0x00250080, nullptr, "AddDefaultCert"},
        {0x00260080, nullptr, "SelectRootCertChain"},
        {0x002700C4, nullptr, "SetClientCert"},
        {0x002B0080, &HTTP_C::SetSSLOpt, "SetSSLOpt"},
        {0x002C0080, nullptr, "SetSSLClearOpt"},
        {0x002D0000, nullptr, "CreateRootCertChain"},
        {0x002E0040, nullptr, "DestroyRootCertChain"},
        {0x002F0082, nullptr, "RootCertChainAddCert"},
        {0x00300080, nullptr, "RootCertChainAddDefaultCert"},
        {0x00310080, nullptr, "RootCertChainRemoveCert"},
        {0x00320084, nullptr, "OpenClientCertContext"},
        {0x00330040, nullptr, "OpenDefaultClientCertContext"},
        {0x00340040, nullptr, "CloseClientCertContext"},
        {0x00350186, nullptr, "SetDefaultProxy"},
        {0x00360000, nullptr, "ClearDNSCache"},
        {0x00370080, &HTTP_C::SetKeepAlive, "SetKeepAlive"},
        {0x003800C0, nullptr, "SetPostDataTypeSize"},
        {0x00390000, nullptr, "Finalize"},
    };
    RegisterHandlers(functions);
}

void HTTP_C::Initialize(Kernel::HLERequestContext& ctx) {
    pimpl->Initialize(ctx);
}

void HTTP_C::CreateContext(Kernel::HLERequestContext& ctx) {
    pimpl->CreateContext(ctx);
}

void HTTP_C::CloseContext(Kernel::HLERequestContext& ctx) {
    pimpl->CloseContext(ctx);
}

void HTTP_C::GetRequestState(Kernel::HLERequestContext& ctx) {
    pimpl->GetRequestState(ctx);
}

void HTTP_C::GetDownloadSizeState(Kernel::HLERequestContext& ctx) {
    pimpl->GetDownloadSizeState(ctx);
}

void HTTP_C::InitializeConnectionSession(Kernel::HLERequestContext& ctx) {
    pimpl->InitializeConnectionSession(ctx);
}

void HTTP_C::BeginRequest(Kernel::HLERequestContext& ctx) {
    pimpl->BeginRequest(ctx);
}

void HTTP_C::BeginRequestAsync(Kernel::HLERequestContext& ctx) {
    pimpl->BeginRequestAsync(ctx);
}

void HTTP_C::ReceiveData(Kernel::HLERequestContext& ctx) {
    pimpl->ReceiveData(ctx);
}

void HTTP_C::ReceiveDataTimeout(Kernel::HLERequestContext& ctx) {
    pimpl->ReceiveDataTimeout(ctx);
}

void HTTP_C::SetProxyDefault(Kernel::HLERequestContext& ctx) {
    pimpl->SetProxyDefault(ctx);
}

void HTTP_C::SetSocketBufferSize(Kernel::HLERequestContext& ctx) {
    pimpl->SetSocketBufferSize(ctx);
}

void HTTP_C::AddRequestHeader(Kernel::HLERequestContext& ctx) {
    pimpl->AddRequestHeader(ctx);
}

void HTTP_C::AddPostDataRaw(Kernel::HLERequestContext& ctx) {
    pimpl->AddPostDataRaw(ctx);
}

void HTTP_C::GetResponseHeader(Kernel::HLERequestContext& ctx) {
    pimpl->GetResponseHeader(ctx);
}

void HTTP_C::GetResponseStatusCode(Kernel::HLERequestContext& ctx) {
    pimpl->GetResponseStatusCode(ctx);
}

void HTTP_C::GetResponseStatusCodeTimeout(Kernel::HLERequestContext& ctx) {
    pimpl->GetResponseStatusCodeTimeout(ctx);
}

void HTTP_C::SetSSLOpt(Kernel::HLERequestContext& ctx) {
    pimpl->SetSSLOpt(ctx);
}

void HTTP_C::SetKeepAlive(Kernel::HLERequestContext& ctx) {
    pimpl->SetKeepAlive(ctx);
}

void InstallInterfaces(SM::ServiceManager& service_manager) {
    std::make_shared<HTTP_C>()->InstallAsService(service_manager);
}

} // namespace HTTP
} // namespace Service
