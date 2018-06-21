// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QMessageBox>
#include "citra_qt/applets/erreula.h"

QtErrEula::QtErrEula(QWidget* parent) : parent(parent) {}

void QtErrEula::Setup(const Frontend::ErrEulaConfig* config) {
    ErrEula::Setup(config);
    QMetaObject::invokeMethod(this, "ShowMessage", Qt::BlockingQueuedConnection);
}

void QtErrEula::ShowMessage() {
    using namespace Frontend;
    switch (config.error_type) {
    case ErrEulaErrorType::ErrorCode: {
        QMessageBox::critical(parent, tr("ErrEula"),
                              tr("Error Code: %1")
                                  .arg(QString::fromStdString(
                                      Common::StringFromFormat("0x%08X", config.error_code))));
        break;
    }
    case ErrEulaErrorType::LocalizedErrorText:
    case ErrEulaErrorType::ErrorText: {
        std::string error = Common::UTF16ToUTF8(config.error_text);
        QMessageBox::critical(
            parent, tr("ErrEula"),
            tr("Error Code: %1\n\n%2")
                .arg(QString::fromStdString(Common::StringFromFormat("0x%08X", config.error_code)),
                     QString::fromStdString(error)));
        break;
    }
    case ErrEulaErrorType::Agree:
    case ErrEulaErrorType::Eula:
    case ErrEulaErrorType::EulaDrawOnly:
    case ErrEulaErrorType::EulaFirstBoot: {
        QMessageBox::information(parent, tr("ErrEula"), tr("EULA accepted"));
        break;
    }
    }
    Finalize(ErrEulaResult::Success);
}
