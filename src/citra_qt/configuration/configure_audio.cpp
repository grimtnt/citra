// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <memory>
#include "audio_core/sink.h"
#include "citra_qt/configuration/configure_audio.h"
#include "core/settings.h"
#include "ui_configure_audio.h"

ConfigureAudio::ConfigureAudio(QWidget* parent)
    : QWidget{parent}, ui{std::make_unique<Ui::ConfigureAudio>()} {
    ui->setupUi(this);

    setConfiguration();
}

ConfigureAudio::~ConfigureAudio() {}

void ConfigureAudio::setConfiguration() {
    ui->toggle_audio_stretching->setChecked(Settings::values.enable_audio_stretching);

    ui->audio_device_combo_box->addItem("auto");
    std::vector<std::string> device_list{AudioCore::ListDevices()};
    for (const auto& device : device_list) {
        ui->audio_device_combo_box->addItem(device.c_str());
    }

    int new_device_index{-1};
    for (int index{}; index < ui->audio_device_combo_box->count(); index++) {
        if (ui->audio_device_combo_box->itemText(index).toStdString() ==
            Settings::values.audio_device_id) {
            new_device_index = index;
            break;
        }
    }

    ui->audio_device_combo_box->setCurrentIndex(new_device_index);
}

void ConfigureAudio::applyConfiguration() {
    Settings::values.enable_audio_stretching = ui->toggle_audio_stretching->isChecked();
    Settings::values.audio_device_id =
        ui->audio_device_combo_box->itemText(ui->audio_device_combo_box->currentIndex())
            .toStdString();
}
