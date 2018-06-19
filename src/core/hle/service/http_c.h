// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "core/hle/service/service.h"

namespace Service {
namespace HTTP {

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

    class Impl;
    std::unique_ptr<Impl> pimpl;
};

void InstallInterfaces(SM::ServiceManager& service_manager);

} // namespace HTTP
} // namespace Service
