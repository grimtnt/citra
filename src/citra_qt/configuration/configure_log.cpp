// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QComboBox>
#include "iostream"
#include "citra_qt/configuration/configure_log.h"
#include "core/settings.h"
#include "ui_configure_log.h" 

ConfigureLog::ConfigureLog(QWidget* parent) : QWidget(parent), ui(new Ui::ConfigureLog) {
    ui->setupUi(this);
    this->setConfiguration();
    ui->filters_combo_box->clear();
    ui->filters_combo_box->addItem("Trace");
    ui->filters_combo_box->addItem("Debug");
    ui->filters_combo_box->addItem("Info");
    ui->filters_combo_box->addItem("Warning");
    ui->filters_combo_box->addItem("Error");
    ui->filters_combo_box->addItem("Critical");
}

ConfigureLog::~ConfigureLog() {}

void ConfigureLog::setConfiguration() {
     if (Settings::values.log_filter == "Trace") {
         ui->filters_combo_box->setCurrentIndex(0);
     }
     else if (Settings::values.log_filter == "Debug") {
         ui->filters_combo_box->setCurrentIndex(1);
     }
     else if (Settings::values.log_filter == "Info") {
         ui->filters_combo_box->setCurrentIndex(2);
     }
     else if (Settings::values.log_filter == "Warning") {
         ui->filters_combo_box->setCurrentIndex(3);
     }
     else if (Settings::values.log_filter == "Error") {
         ui->filters_combo_box->setCurrentIndex(4);
     }
     else if (Settings::values.log_filter == "Critical") {
         ui->filters_combo_box->setCurrentIndex(5);
     }
}

void ConfigureLog::applyConfiguration() {
    Settings::values.log_filter =
        ui->filters_combo_box->itemText(ui->filters_combo_box->currentIndex())
                  .toStdString();
    Settings::Apply();
}
