// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "citra_qt/configuration/config.h"
#include "citra_qt/configuration/configure_dialog.h"
#include "core/settings.h"
#include "ui_configure.h"

ConfigureDialog::ConfigureDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ConfigureDialog) {
    ui->setupUi(this);
    connect(ui->generalTab, &ConfigureGeneral::languageChanged, this,
            &ConfigureDialog::onLanguageChanged);
}

ConfigureDialog::~ConfigureDialog() {}

void ConfigureDialog::applyConfiguration() {
    ui->generalTab->applyConfiguration();
    ui->systemTab->applyConfiguration();
    ui->inputTab->applyConfiguration();
    ui->graphicsTab->applyConfiguration();
    ui->audioTab->applyConfiguration();
    ui->cameraTab->applyConfiguration();
    ui->webTab->applyConfiguration();
    ui->hacksTab->applyConfiguration();
    sd_card_root_changed = ui->generalTab->sd_card_root_changed;
    Settings::Apply();
}

void ConfigureDialog::onLanguageChanged(const QString& locale) {
    emit languageChanged(locale);
    ui->retranslateUi(this);
    ui->generalTab->retranslateUi();
    ui->systemTab->retranslateUi();
    ui->inputTab->retranslateUi();
    ui->graphicsTab->retranslateUi();
    ui->audioTab->retranslateUi();
    ui->cameraTab->retranslateUi();
    ui->webTab->retranslateUi();
    ui->hacksTab->retranslateUi();
}
