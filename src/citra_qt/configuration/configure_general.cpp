// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QDesktopServices>
#include <QDirIterator>
#include <QFileDialog>
#include <QUrl>
#include "citra_qt/configuration/configure_general.h"
#include "citra_qt/ui_settings.h"
#include "citra_qt/util/console.h"
#include "common/file_util.h"
#include "common/logging/backend.h"
#include "common/logging/filter.h"
#include "common/logging/log.h"
#include "core/core.h"
#include "core/settings.h"
#include "ui_configure_general.h"

ConfigureGeneral::ConfigureGeneral(QWidget* parent)
    : QWidget(parent), ui(new Ui::ConfigureGeneral) {

    ui->setupUi(this);
    ui->language_combobox->addItem(tr("<System>"), QString(""));
    ui->language_combobox->addItem(tr("English"), QString("en"));
    QDirIterator it(":/languages", QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        QString locale = it.next();
        locale.truncate(locale.lastIndexOf('.'));
        locale.remove(0, locale.lastIndexOf('/') + 1);
        QString lang = QLocale::languageToString(QLocale(locale).language());
        ui->language_combobox->addItem(lang, locale);
    }

    // Unlike other configuration changes, interface language changes need to be reflected on the
    // interface immediately. This is done by passing a signal to the main window, and then
    // retranslating when passing back.
    connect(ui->language_combobox,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &ConfigureGeneral::onLanguageChanged);

    connect(ui->button_sd_card_root_empty, &QPushButton::clicked, this, [&](bool checked) {
        Q_UNUSED(checked);
        ui->sd_card_root->setText(QString());
    });

    connect(ui->button_sd_card_root, &QToolButton::clicked, this, [&](bool checked) {
        Q_UNUSED(checked);
        ui->sd_card_root->setText(
            QFileDialog::getExistingDirectory(this, tr("Select SD card root")));
    });

    connect(ui->open_log_button, &QPushButton::pressed, []() {
        QString path = QString::fromStdString(FileUtil::GetUserPath(D_LOGS_IDX));
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    });

    for (auto theme : UISettings::themes) {
        ui->theme_combobox->addItem(theme.first, theme.second);
    }

    setConfiguration();

#ifndef _WIN32
    ui->toggle_console->hide();
#endif

    ui->toggle_cpu_jit->setEnabled(!Core::System::GetInstance().IsPoweredOn());
    ui->toggle_new_mode->setEnabled(!Core::System::GetInstance().IsPoweredOn());
    ui->button_sd_card_root->setEnabled(!Core::System::GetInstance().IsPoweredOn());
    ui->button_sd_card_root_empty->setEnabled(!Core::System::GetInstance().IsPoweredOn());
}

ConfigureGeneral::~ConfigureGeneral() {}

void ConfigureGeneral::setConfiguration() {
    ui->toggle_check_exit->setChecked(UISettings::values.confirm_before_closing);
    ui->toggle_new_mode->setChecked(Settings::values.enable_new_mode);
    ui->toggle_cpu_jit->setChecked(Settings::values.use_cpu_jit);
    ui->sd_card_root->setText(QString::fromStdString(Settings::values.sd_card_root));

    // The first item is "auto-select" with actual value -1, so plus one here will do the trick
    ui->region_combobox->setCurrentIndex(Settings::values.region_value + 1);

    ui->theme_combobox->setCurrentIndex(ui->theme_combobox->findData(UISettings::values.theme));
    ui->language_combobox->setCurrentIndex(
        ui->language_combobox->findData(UISettings::values.language));
    ui->combobox_swkbd_implementation->setCurrentIndex(
        static_cast<int>(Settings::values.swkbd_implementation));
    ui->toggle_console->setEnabled(!Core::System::GetInstance().IsPoweredOn());
    ui->toggle_console->setChecked(UISettings::values.show_console);
    ui->log_filter_edit->setText(QString::fromStdString(Settings::values.log_filter));
}

void ConfigureGeneral::applyConfiguration() {
    UISettings::values.confirm_before_closing = ui->toggle_check_exit->isChecked();
    Settings::values.enable_new_mode = ui->toggle_new_mode->isChecked();
    Settings::values.swkbd_implementation = static_cast<Settings::SwkbdImplementation>(
        ui->combobox_swkbd_implementation->currentIndex());
    UISettings::values.theme =
        ui->theme_combobox->itemData(ui->theme_combobox->currentIndex()).toString();
    sd_card_root_changed = Settings::values.sd_card_root != ui->sd_card_root->text().toStdString();
    Settings::values.sd_card_root = ui->sd_card_root->text().toStdString();
    Settings::values.region_value = ui->region_combobox->currentIndex() - 1;
    Settings::values.use_cpu_jit = ui->toggle_cpu_jit->isChecked();
    UISettings::values.show_console = ui->toggle_console->isChecked();
    Settings::values.log_filter = ui->log_filter_edit->text().toStdString();
    Util::ToggleConsole();
    Log::Filter filter;
    filter.ParseFilterString(Settings::values.log_filter);
    Log::SetGlobalFilter(filter);
}

void ConfigureGeneral::onLanguageChanged(int index) {
    if (index == -1)
        return;

    emit languageChanged(ui->language_combobox->itemData(index).toString());
}

void ConfigureGeneral::retranslateUi() {
    ui->retranslateUi(this);
}
