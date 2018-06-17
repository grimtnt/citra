// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QIcon>
#include <QMessageBox>
#include "citra_qt/configuration/configure_web.h"
#include "core/settings.h"
#include "ui_configure_web.h"
#include "web_service/verify_login.h"

ConfigureWeb::ConfigureWeb(QWidget* parent)
    : QWidget(parent), ui(std::make_unique<Ui::ConfigureWeb>()) {
    ui->setupUi(this);

    connect(ui->button_verify_login, &QPushButton::clicked, this, &ConfigureWeb::VerifyLogin);
    connect(this, &ConfigureWeb::LoginVerified, this, &ConfigureWeb::OnLoginVerified);

    setConfiguration();
}

ConfigureWeb::~ConfigureWeb() {}

void ConfigureWeb::setConfiguration() {
    ui->web_credentials_disclaimer->setWordWrap(true);
    ui->web_signup_link->setOpenExternalLinks(true);
    ui->web_signup_link->setText(
        tr("<a href='https://services.citra-emu.org/'><span style=\"text-decoration: underline; "
           "color:#039be5;\">Sign up</span></a>"));
    ui->web_token_info_link->setOpenExternalLinks(true);
    ui->web_token_info_link->setText(
        tr("<a href='https://citra-emu.org/wiki/citra-web-service/'><span style=\"text-decoration: "
           "underline; color:#039be5;\">What is my token?</span></a>"));

    ui->edit_username->setText(QString::fromStdString(Settings::values.citra_username));
    ui->edit_token->setText(QString::fromStdString(Settings::values.citra_token));
    // Connect after setting the values, to avoid calling OnLoginChanged now
    connect(ui->edit_token, &QLineEdit::textChanged, this, &ConfigureWeb::OnLoginChanged);
    connect(ui->edit_username, &QLineEdit::textChanged, this, &ConfigureWeb::OnLoginChanged);
    user_verified = true;
}

void ConfigureWeb::applyConfiguration() {
    if (user_verified) {
        Settings::values.citra_username = ui->edit_username->text().toStdString();
        Settings::values.citra_token = ui->edit_token->text().toStdString();
    } else {
        QMessageBox::warning(this, tr("Username and token not verified"),
                             tr("Username and token were not verified. The changes to your "
                                "username and/or token have not been saved."));
    }
    Settings::Apply();
}

void ConfigureWeb::OnLoginChanged() {
    if (ui->edit_username->text().isEmpty() && ui->edit_token->text().isEmpty()) {
        user_verified = true;
        ui->label_username_verified->setPixmap(QIcon::fromTheme("checked").pixmap(16));
        ui->label_token_verified->setPixmap(QIcon::fromTheme("checked").pixmap(16));
    } else {
        user_verified = false;
        ui->label_username_verified->setPixmap(QIcon::fromTheme("failed").pixmap(16));
        ui->label_token_verified->setPixmap(QIcon::fromTheme("failed").pixmap(16));
    }
}

void ConfigureWeb::VerifyLogin() {
    std::string username = ui->edit_username->text().toStdString();
    std::string token = ui->edit_token->text().toStdString();
    verified = WebService::VerifyLogin(username, token, Settings::values.verify_endpoint_url,
                                       [&]() { emit LoginVerified(); });
    ui->button_verify_login->setDisabled(true);
    ui->button_verify_login->setText(tr("Verifying"));
}

void ConfigureWeb::OnLoginVerified() {
    ui->button_verify_login->setEnabled(true);
    ui->button_verify_login->setText(tr("Verify"));
    if (verified.get()) {
        user_verified = true;
        ui->label_username_verified->setPixmap(QIcon::fromTheme("checked").pixmap(16));
        ui->label_token_verified->setPixmap(QIcon::fromTheme("checked").pixmap(16));
    } else {
        ui->label_username_verified->setPixmap(QIcon::fromTheme("failed").pixmap(16));
        ui->label_token_verified->setPixmap(QIcon::fromTheme("failed").pixmap(16));
        QMessageBox::critical(
            this, tr("Verification failed"),
            tr("Verification failed. Check that you have entered your username and token "
               "correctly, and that your internet connection is working."));
    }
}

void ConfigureWeb::retranslateUi() {
    ui->retranslateUi(this);
}
