// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <memory>

#include <QDirIterator>
#include <QFileDialog>
#include <QImageReader>
#include <QMessageBox>
#include <QWidget>

#include "citra_qt/configuration/configure_camera.h"
#include "citra_qt/ui_settings.h"
#include "core/core.h"
#include "core/settings.h"
#include "ui_configure_camera.h"

#ifdef ENABLE_OPENCV_CAMERA
const std::map<QString, std::vector<QString>> Implementations = {
    /* blank */ {QObject::tr("Blank"), {"blank"}},
    /* still image */ {QObject::tr("Still Image"), {"image"}},
    /* video & image sequence */ {QObject::tr("Video & Image Sequence"), {"opencv"}},
#ifdef ENABLE_QT_CAMERA
    /* camera */ {QObject::tr("System Camera"), {"opencv", "qt"}},
#else
    /* camera */ {QObject::tr("System Camera"), {"opencv"}},
#endif
};
#else
const std::map<QString, std::vector<QString>> Implementations = {
    /* blank */ {QObject::tr("Blank"), {"blank"}},
    /* still image */ {QObject::tr("Still Image"), {"image"}},
    /* video & image sequence */ {QObject::tr("Video & Image Sequence"), {}},
#ifdef ENABLE_QT_CAMERA
    /* camera */ {QObject::tr("System Camera"), {"qt"}},
#else
    /* camera */ {QObject::tr("System Camera"), {}},
#endif
};
#endif

ConfigureCamera::ConfigureCamera(QWidget* parent) : QWidget(parent), ui(new Ui::ConfigureCamera) {

    ui->setupUi(this);
    ui->preview_box->setHidden(true);
    connect(ui->camera_selection,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &ConfigureCamera::onCameraChanged);
    camera_name = Settings::values.camera_name;
    camera_config = Settings::values.camera_config;
    for (auto pair : Implementations) {
        if (!pair.second.empty()) {
            ui->image_source->addItem(pair.first);
        }
    }

    setConfiguration();
    connectEvents();
}

ConfigureCamera::~ConfigureCamera() {
    stopPreviewing();
}

void ConfigureCamera::connectEvents() {
    connect(ui->image_source,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &ConfigureCamera::onImageSourceChanged);
    connect(ui->implementation,
            static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged), this,
            &ConfigureCamera::onImplementationChanged);
    connect(ui->camera_file, &QLineEdit::textChanged, this,
            &ConfigureCamera::onCameraConfigChanged);
    connect(ui->toolButton, &QToolButton::clicked, this, &ConfigureCamera::onToolButtonClicked);
    connect(ui->preview_button, &QPushButton::clicked, this, [=] { startPreviewing(); });
    connect(ui->prompt_before_load, &QCheckBox::stateChanged, this,
            &ConfigureCamera::onPromptBeforeLoadChanged);
}

void ConfigureCamera::setUiDisplay() {
    QString image_source = ui->image_source->currentText();
    if (image_source == tr("Blank") || image_source == tr("System Camera")) {
        ui->prompt_before_load->setHidden(true);
        ui->prompt_before_load->setChecked(false);
        ui->camera_file_label->setHidden(true);
        ui->camera_file->setHidden(true);
        ui->camera_file->setText("");
        ui->toolButton->setHidden(true);
    } else if (image_source == tr("Still Image")) {
        ui->prompt_before_load->setHidden(false);
        ui->camera_file_label->setHidden(false);
        ui->camera_file->setHidden(false);
        ui->toolButton->setHidden(false);
    } else if (image_source == tr("Video & Image Sequence")) {
        ui->prompt_before_load->setHidden(true);
        ui->prompt_before_load->setChecked(false);
        ui->camera_file_label->setHidden(false);
        ui->camera_file->setHidden(false);
        ui->toolButton->setHidden(false);
    } else {
        LOG_ERROR(Frontend, "Error: unknown image source");
    }

    // Get the implementations
    ui->implementation->clear();
    for (auto implementation : Implementations.at(ui->image_source->currentText())) {
        ui->implementation->addItem(implementation);
    }

    ui->implementation->setCurrentText(QString::fromStdString(camera_name[camera_selection]));

    ui->camera_file->setText(QString::fromStdString(camera_config[camera_selection]));
}

void ConfigureCamera::startPreviewing() {
    stopPreviewing();
    previewing_camera =
        Camera::CreateCameraPreview(camera_name[camera_selection], camera_config[camera_selection],
                                    preview_width, preview_height);
    if (!previewing_camera) {
        return;
    }
    ui->preview_box->setHidden(false);
    ui->preview_button->setHidden(true);
    preview_width = ui->preview_box->size().width();
    preview_height = preview_width * 0.75;
    ui->preview_box->setToolTip(tr("Resolution: ") + QString::number(preview_width) + "*" +
                                QString::number(preview_height));
    previewing_camera->SetResolution(
        {static_cast<u16>(preview_width), static_cast<u16>(preview_height)});
    previewing_camera->SetEffect(Service::CAM::Effect::None);
    previewing_camera->SetFlip(Service::CAM::Flip::None);
    previewing_camera->SetFormat(Service::CAM::OutputFormat::RGB565);
    previewing_camera->SetFrameRate(Service::CAM::FrameRate::Rate_30);
    previewing_camera->StartCapture();

    timer_id = startTimer(1000 / 30);
}

void ConfigureCamera::stopPreviewing() {
    ui->preview_box->setHidden(true);
    ui->preview_button->setHidden(false);

    if (previewing_camera) {
        previewing_camera->StopCapture();
        previewing_camera.release();
    }

    if (timer_id != 0) {
        killTimer(timer_id);
        timer_id = 0;
    }
}

void ConfigureCamera::timerEvent(QTimerEvent* event) {
    if (event->timerId() != timer_id) {
        return;
    }
    if (!previewing_camera) {
        killTimer(timer_id);
        timer_id = 0;
        return;
    }
    std::vector<u16> frame = previewing_camera->ReceiveFrame();
    int width = ui->preview_box->size().width();
    int height = width * 0.75;
    if (width != preview_width || height != preview_height) {
        stopPreviewing();
        return;
    }
    QImage image(width, height, QImage::Format::Format_RGB16);
    std::memcpy(image.bits(), frame.data(), width * height * sizeof(u16));
    ui->preview_box->setPixmap(QPixmap::fromImage(image));
}

void ConfigureCamera::onCameraChanged(int index) {
    camera_selection = index;

    stopPreviewing();

    // Load configuration
    setConfiguration();
}

void ConfigureCamera::onCameraConfigChanged(const QString& text) {
    camera_config[camera_selection] = text.toStdString();

    stopPreviewing();
}

void ConfigureCamera::onImageSourceChanged(int index) {
    setUiDisplay();

    stopPreviewing();
}

void ConfigureCamera::onImplementationChanged(const QString& text) {
    camera_name[camera_selection] = text.toStdString();

    stopPreviewing();
}

void ConfigureCamera::onToolButtonClicked() {
    QString filter;
    if (camera_name[camera_selection] == "image") {
        QList<QByteArray> types = QImageReader::supportedImageFormats();
        QList<QString> temp_filters = {};
        for (QByteArray type : types) {
            temp_filters << QString("*." + QString(type));
        }

        filter = tr("Supported image files (") + temp_filters.join(" ") + ")";
    }
#ifdef ENABLE_OPENCV_CAMERA
    else if (camera_name[camera_selection] == "opencv") {
        filter = tr("Supported video files (") + "*.mpg *.avi)";
    }
#endif
    QString path = QFileDialog::getOpenFileName(this, tr("Open File"), ".", filter);
    if (!path.isEmpty()) {
        ui->camera_file->setText(path);
    }
}

void ConfigureCamera::onPromptBeforeLoadChanged(int state) {
    if (state == Qt::Checked) {
        ui->camera_file->setText("");
        ui->camera_file->setDisabled(true);
        ui->toolButton->setDisabled(true);
    } else {
        ui->camera_file->setDisabled(false);
        ui->toolButton->setDisabled(false);
    }
}

void ConfigureCamera::setConfiguration() {
    int index = camera_selection;
    // Convert camera name to image sources
    if (camera_name[index] == "blank") {
        ui->image_source->setCurrentText(tr("Blank"));
    } else if (camera_name[index] == "image") {
        ui->image_source->setCurrentText(tr("Still Image"));
        if (camera_config[index].empty()) {
            ui->prompt_before_load->setChecked(true);
        }
    }
#ifdef ENABLE_OPENCV_CAMERA
    else if (camera_name[index] == "opencv") {
        if (camera_config[index].empty()) {
            ui->image_source->setCurrentText(tr("System Camera"));
        } else {
            ui->image_source->setCurrentText(tr("Video & Image Sequence"));
        }
    }
#endif
#ifdef ENABLE_QT_CAMERA
    else if (camera_name[index] == "qt") {
        ui->image_source->setCurrentText(tr("System Camera"));
    }
#endif
    else {
        LOG_ERROR(Frontend, "Unknown camera type %s", camera_name[index].c_str());
        QString message =
            tr(("Sorry, but your configuration file seems to be invalid:\n\nUnknown camera type " +
                camera_name[index])
                   .c_str());
#ifndef ENABLE_OPENCV_CAMERA
        if (camera_name[index] == "opencv") {
            message.append(tr("\n\nNote: This build does not have the OpenCV camera included."));
        }
#endif
        QMessageBox::critical(this, tr("Error"), message);
        ui->image_source->setCurrentText(tr("Blank"));
    }

    setUiDisplay();
}

void ConfigureCamera::applyConfiguration() {
    stopPreviewing();
    Settings::values.camera_name = camera_name;
    Settings::values.camera_config = camera_config;
    Settings::Apply();
}

void ConfigureCamera::retranslateUi() {
    ui->retranslateUi(this);
}
