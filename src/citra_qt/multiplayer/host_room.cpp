// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <chrono>
#include <future>
#include <QColor>
#include <QImage>
#include <QInputDialog>
#include <QList>
#include <QLocale>
#include <QMessageBox>
#include <QMetaType>
#include <QTableWidgetItem>
#include <QTime>
#include <QtConcurrent/QtConcurrentRun>
#include "citra_qt/game_list_p.h"
#include "citra_qt/main.h"
#include "citra_qt/multiplayer/host_room.h"
#include "citra_qt/multiplayer/message.h"
#include "citra_qt/multiplayer/state.h"
#include "citra_qt/multiplayer/validation.h"
#include "citra_qt/ui_settings.h"
#include "common/logging/log.h"
#include "core/settings.h"
#include "ui_host_room.h"

HostRoomWindow::HostRoomWindow(QWidget* parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowSystemMenuHint),
      ui(std::make_unique<Ui::HostRoom>()) {
    ui->setupUi(this);

    // set up validation for all of the fields
    ui->port->setValidator(validation.GetPort());
    ui->port->setPlaceholderText(QString::number(Network::DefaultRoomPort));

    // Connect all the widgets to the appropriate events
    connect(ui->host, &QPushButton::pressed, this, &HostRoomWindow::Host);
    connect(ui->stop_in_checkbox, &QCheckBox::stateChanged, this, [&](bool state) {
        ui->minutes_label->setEnabled(state);
        ui->minutes_spinbox->setEnabled(state);
    });

    // Restore the port:
    ui->port->setText(UISettings::values.port);
}

HostRoomWindow::~HostRoomWindow() = default;

void HostRoomWindow::Host() {
    if (!ui->port->hasAcceptableInput()) {
        NetworkMessage::ShowError(NetworkMessage::PORT_NOT_VALID);
        return;
    }
    auto parent = static_cast<MultiplayerState*>(parentWidget());
    bool stop_in_checked = ui->stop_in_checkbox->isChecked();
    if (stop_in_checked) {
        parent->SetCloseMs(std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::minutes(ui->minutes_spinbox->value()))
                               .count());
    }
    if (auto member = Network::GetRoomMember().lock()) {
        if (member->GetState() == Network::RoomMember::State::Joining) {
            return;
        } else if (member->GetState() == Network::RoomMember::State::Joined) {
            if (!parent->OnCloseRoom()) {
                close();
                return;
            }
        }
        ui->host->setDisabled(true);

        auto port = ui->port->isModified() ? ui->port->text().toInt() : Network::DefaultRoomPort;
        if (auto room = Network::GetRoom().lock()) {
            bool created = room->Create(port);
            if (!created) {
                NetworkMessage::ShowError(NetworkMessage::COULD_NOT_CREATE_ROOM);
                LOG_ERROR(Network, "Could not create room!");
                ui->host->setEnabled(true);
                return;
            }
        }
        member->Join("127.0.0.1", port, Network::NoPreferredMac);

        // Store port setting
        UISettings::values.port_host = (ui->port->isModified() && !ui->port->text().isEmpty())
                                           ? ui->port->text()
                                           : QString::number(Network::DefaultRoomPort);
        Settings::Apply();
        OnConnection();
        if (stop_in_checked)
            parent->StartTimer();
    }
}

void HostRoomWindow::OnConnection() {
    ui->host->setEnabled(true);
    close();
}
