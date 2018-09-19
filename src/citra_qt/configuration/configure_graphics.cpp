// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "citra_qt/configuration/configure_graphics.h"
#include "core/core.h"
#include "core/settings.h"
#include "ui_configure_graphics.h"

ConfigureGraphics::ConfigureGraphics(QWidget* parent)
    : QWidget{parent}, ui{std::make_unique<Ui::ConfigureGraphics>()} {

    ui->setupUi(this);

    setConfiguration();

    ui->toggle_accurate_shaders->setEnabled(ui->toggle_hw_shaders->isChecked());
    connect(ui->toggle_hw_shaders, &QCheckBox::stateChanged, ui->toggle_accurate_shaders,
            &QWidget::setEnabled);
}

ConfigureGraphics::~ConfigureGraphics() {}

void ConfigureGraphics::setConfiguration() {
    ui->toggle_hw_shaders->setChecked(Settings::values.use_hw_shaders);
    ui->toggle_accurate_shaders->setChecked(Settings::values.accurate_shaders);
}

void ConfigureGraphics::applyConfiguration() {
    Settings::values.use_hw_shaders = ui->toggle_hw_shaders->isChecked();
    Settings::values.accurate_shaders = ui->toggle_accurate_shaders->isChecked();
}
