// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/am/am_net.h"

namespace Service::AM {

AM_NET::AM_NET(std::shared_ptr<Module> am) : Module::Interface{std::move(am), "am:net", 5} {
    static const FunctionInfo functions[]{
        {0x00010040, &AM_NET::GetNumPrograms, "GetNumPrograms"},
        {0x00020082, &AM_NET::GetProgramList, "GetProgramList"},
        {0x00030084, &AM_NET::GetProgramInfos, "GetProgramInfos"},
        {0x000400C0, &AM_NET::DeleteUserProgram, "DeleteUserProgram"},
        {0x000500C0, &AM_NET::GetProductCode, "GetProductCode"},
        {0x000600C0, nullptr, "GetStorageId"},
        {0x00070080, &AM_NET::DeleteTicket, "DeleteTicket"},
        {0x00080000, &AM_NET::GetNumTickets, "GetNumTickets"},
        {0x00090082, &AM_NET::GetTicketList, "GetTicketList"},
        {0x000A0000, nullptr, "GetDeviceID"},
        {0x000B0040, nullptr, "GetNumImportTitleContexts"},
        {0x000C0082, nullptr, "GetImportTitleContextList"},
        {0x000D0084, nullptr, "GetImportTitleContexts"},
        {0x000E00C0, nullptr, "DeleteImportTitleContext"},
        {0x000F00C0, nullptr, "GetNumImportContentContexts"},
        {0x00100102, nullptr, "GetImportContentContextList"},
        {0x00110104, nullptr, "GetImportContentContexts"},
        {0x00120102, nullptr, "DeleteImportContentContexts"},
        {0x00130040, nullptr, "NeedsCleanup"},
        {0x00140040, nullptr, "DoCleanup"},
        {0x00150040, nullptr, "DeleteAllImportContexts"},
        {0x00160000, nullptr, "DeleteAllTemporaryPrograms"},
        {0x00170044, nullptr, "ImportTwlBackupLegacy"},
        {0x00180080, nullptr, "InitializeTitleDatabase"},
        {0x00190040, nullptr, "QueryAvailableTitleDatabase"},
        {0x001A00C0, nullptr, "CalcTwlBackupSize"},
        {0x001B0144, nullptr, "ExportTwlBackup"},
        {0x001C0084, nullptr, "ImportTwlBackup"},
        {0x001D0000, nullptr, "DeleteAllTwlUserPrograms"},
        {0x001E00C8, nullptr, "ReadTwlBackupInfo"},
        {0x001F0040, nullptr, "DeleteAllExpiredUserPrograms"},
        {0x00200000, nullptr, "GetTwlArchiveResourceInfo"},
        {0x00210042, nullptr, "GetPersonalizedTicketInfoList"},
        {0x00220080, nullptr, "DeleteAllImportContextsFiltered"},
        {0x00230080, nullptr, "GetNumImportTitleContextsFiltered"},
        {0x002400C2, nullptr, "GetImportTitleContextListFiltered"},
        {0x002500C0, nullptr, "CheckContentRights"},
        {0x00260044, nullptr, "GetTicketLimitInfos"},
        {0x00270044, nullptr, "GetDemoLaunchInfos"},
        {0x00280108, nullptr, "ReadTwlBackupInfoEx"},
        {0x00290082, nullptr, "DeleteUserProgramsAtomically"},
        {0x002A00C0, nullptr, "GetNumExistingContentInfosSystem"},
        {0x002B0142, nullptr, "ListExistingContentInfosSystem"},
        {0x002C0084, nullptr, "GetProgramInfosIgnorePlatform"},
        {0x002D00C0, nullptr, "CheckContentRightsIgnorePlatform"},
        {0x04010080, nullptr, "UpdateFirmwareTo"},
        {0x04020040, &AM_NET::BeginImportProgram, "BeginImportProgram"},
        {0x04030000, nullptr, "BeginImportProgramTemporarily"},
        {0x04040002, nullptr, "CancelImportProgram"},
        {0x04050002, &AM_NET::EndImportProgram, "EndImportProgram"},
        {0x04060002, nullptr, "EndImportProgramWithoutCommit"},
        {0x040700C2, nullptr, "CommitImportPrograms"},
        {0x04080042, &AM_NET::GetProgramInfoFromCia, "GetProgramInfoFromCia"},
        {0x04090004, &AM_NET::GetSystemMenuDataFromCia, "GetSystemMenuDataFromCia"},
        {0x040A0002, &AM_NET::GetDependencyListFromCia, "GetDependencyListFromCia"},
        {0x040B0002, &AM_NET::GetTransferSizeFromCia, "GetTransferSizeFromCia"},
        {0x040C0002, &AM_NET::GetCoreVersionFromCia, "GetCoreVersionFromCia"},
        {0x040D0042, &AM_NET::GetRequiredSizeFromCia, "GetRequiredSizeFromCia"},
        {0x040E00C2, nullptr, "CommitImportProgramsAndUpdateFirmwareAuto"},
        {0x040F0000, nullptr, "UpdateFirmwareAuto"},
        {0x041000C0, &AM_NET::DeleteProgram, "DeleteProgram"},
        {0x04110044, nullptr, "GetTwlProgramListForReboot"},
        {0x04120000, nullptr, "GetSystemUpdaterMutex"},
        {0x04130002, &AM_NET::GetMetaSizeFromCia, "GetMetaSizeFromCia"},
        {0x04140044, &AM_NET::GetMetaDataFromCia, "GetMetaDataFromCia"},
        {0x04150080, nullptr, "CheckDemoLaunchRights"},
        {0x041600C0, nullptr, "GetInternalTitleLocationInfo"},
        {0x041700C0, nullptr, "PerpetuateAgbSaveData"},
        {0x04180040, nullptr, "BeginImportProgramForOverWrite"},
        {0x04190000, nullptr, "BeginImportSystemProgram"},
        {0x08010000, nullptr, "BeginImportTicket"},
        {0x08020002, nullptr, "CancelImportTicket"},
        {0x08030002, nullptr, "EndImportTicket"},
        {0x08040100, nullptr, "BeginImportTitle"},
        {0x08050000, nullptr, "StopImportTitle"},
        {0x080600C0, nullptr, "ResumeImportTitle"},
        {0x08070000, nullptr, "CancelImportTitle"},
        {0x08080000, nullptr, "EndImportTitle"},
        {0x080900C2, nullptr, "CommitImportTitles"},
        {0x080A0000, nullptr, "BeginImportTmd"},
        {0x080B0002, nullptr, "CancelImportTmd"},
        {0x080C0042, nullptr, "EndImportTmd"},
        {0x080D0042, nullptr, "CreateImportContentContexts"},
        {0x080E0040, nullptr, "BeginImportContent"},
        {0x080F0002, nullptr, "StopImportContent"},
        {0x08100040, nullptr, "ResumeImportContent"},
        {0x08110002, nullptr, "CancelImportContent"},
        {0x08120002, nullptr, "EndImportContent"},
        {0x08130000, nullptr, "GetNumCurrentImportContentContexts"},
        {0x08140042, nullptr, "GetCurrentImportContentContextList"},
        {0x08150044, nullptr, "GetCurrentImportContentContexts"},
        {0x08160146, nullptr, "Sign"},
        {0x08170146, nullptr, "Verify"},
        {0x08180042, nullptr, "GetDeviceCert"},
        {0x08190108, nullptr, "ImportCertificates"},
        {0x081A0042, nullptr, "ImportCertificate"},
        {0x081B00C2, nullptr, "CommitImportTitlesAndUpdateFirmwareAuto"},
        {0x081C0100, nullptr, "DeleteTicketId"},
        {0x081D0080, nullptr, "GetNumTicketIds"},
        {0x081E0102, nullptr, "GetTicketIdList"},
        {0x081F0080, nullptr, "GetNumTicketsOfProgram"},
        {0x08200102, nullptr, "ListTicketInfos"},
        {0x08210142, nullptr, "GetRightsOnlyTicketData"},
        {0x08220000, nullptr, "GetNumCurrentContentInfos"},
        {0x08230044, nullptr, "FindCurrentContentInfos"},
        {0x08240082, nullptr, "ListCurrentContentInfos"},
        {0x08250102, nullptr, "CalculateContextRequiredSize"},
        {0x08260042, nullptr, "UpdateImportContentContexts"},
        {0x08270000, nullptr, "DeleteAllDemoLaunchInfos"},
        {0x082800C0, nullptr, "BeginImportTitleForOverWrite"},
        {0x08290184, nullptr, "ExportTicketWrapped"},
    };
    RegisterHandlers(functions);
}

} // namespace Service::AM
