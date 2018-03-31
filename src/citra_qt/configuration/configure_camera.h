// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>

#include "core/frontend/camera/factory.h"
#include "core/frontend/camera/interface.h"

namespace Ui {
class ConfigureCamera;
}

class ConfigureCamera : public QWidget {
    Q_OBJECT

public:
    explicit ConfigureCamera(QWidget* parent = nullptr);
    ~ConfigureCamera();

    void applyConfiguration();
    void retranslateUi();

    void timerEvent(QTimerEvent*) override;

public slots:
    void onCameraChanged(int index);
    void onCameraConfigChanged(const QString& text);
    void onImageSourceChanged(int index);
    void onImplementationChanged(const QString& text);
    void onToolButtonClicked();
    void onPromptBeforeLoadChanged(int state);

private:
    void setConfiguration();
    void setUiDisplay();
    void startPreviewing();
    void stopPreviewing();
    void connectEvents();

private:
    std::unique_ptr<Ui::ConfigureCamera> ui;
    std::array<std::string, 3> camera_name;
    std::array<std::string, 3> camera_config;
    int camera_selection = 0;
    int timer_id = 0;
    int preview_width = 0;
    int preview_height = 0;
    bool is_previewing = false;
    std::unique_ptr<Camera::CameraInterface> previewing_camera;
};
