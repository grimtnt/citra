// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QCamera>
#include <QImageReader>
#include <QMessageBox>
#include "citra_qt/camera/qt_multimedia_camera.h"
#include "citra_qt/main.h"

namespace Camera {

QList<QVideoFrame::PixelFormat> QtCameraSurface::supportedPixelFormats(
    QAbstractVideoBuffer::HandleType handleType) const {
    Q_UNUSED(handleType);
    return QList<QVideoFrame::PixelFormat>()
           << QVideoFrame::Format_ARGB32 << QVideoFrame::Format_ARGB32_Premultiplied
           << QVideoFrame::Format_RGB32 << QVideoFrame::Format_RGB24 << QVideoFrame::Format_RGB565
           << QVideoFrame::Format_RGB555 << QVideoFrame::Format_ARGB8565_Premultiplied
           << QVideoFrame::Format_BGRA32 << QVideoFrame::Format_BGRA32_Premultiplied
           << QVideoFrame::Format_BGR32 << QVideoFrame::Format_BGR24 << QVideoFrame::Format_BGR565
           << QVideoFrame::Format_BGR555 << QVideoFrame::Format_BGRA5658_Premultiplied
           << QVideoFrame::Format_AYUV444 << QVideoFrame::Format_AYUV444_Premultiplied
           << QVideoFrame::Format_YUV444 << QVideoFrame::Format_YUV420P << QVideoFrame::Format_YV12
           << QVideoFrame::Format_UYVY << QVideoFrame::Format_YUYV << QVideoFrame::Format_NV12
           << QVideoFrame::Format_NV21 << QVideoFrame::Format_IMC1 << QVideoFrame::Format_IMC2
           << QVideoFrame::Format_IMC3 << QVideoFrame::Format_IMC4 << QVideoFrame::Format_Y8
           << QVideoFrame::Format_Y16 << QVideoFrame::Format_Jpeg << QVideoFrame::Format_CameraRaw
           << QVideoFrame::Format_AdobeDng; // Supporting all the formats
}

bool QtCameraSurface::present(const QVideoFrame& frame) {
    if (frame.isValid()) {
        QVideoFrame cloneFrame(frame);
        cloneFrame.map(QAbstractVideoBuffer::ReadOnly);
        const QImage image(cloneFrame.bits(), cloneFrame.width(), cloneFrame.height(),
                           QVideoFrame::imageFormatFromPixelFormat(cloneFrame.pixelFormat()));
        current_frame = image.copy();
        cloneFrame.unmap();
        return true;
    }
    return false;
}

QtMultimediaCamera::QtMultimediaCamera(const std::string& camera_name) : camera_surface() {
    QMetaObject::invokeMethod(QApplication::instance(), [this] {
        camera = new QCamera;
        camera->setViewfinder(&camera_surface);
    });
}

void QtMultimediaCamera::StartCapture() {
    QMetaObject::invokeMethod(QApplication::instance(), [this] { camera->start(); });
    // camera.start();
}

void QtMultimediaCamera::StopCapture() {
    camera->stop();
}

void QtMultimediaCamera::SetFormat(Service::CAM::OutputFormat output_format) {
    output_rgb = output_format == Service::CAM::OutputFormat::RGB565;
}

void QtMultimediaCamera::SetResolution(const Service::CAM::Resolution& resolution) {
    width = resolution.width;
    height = resolution.height;
};

void QtMultimediaCamera::SetFlip(Service::CAM::Flip flip) {
    using namespace Service::CAM;
    flip_horizontal = (flip == Flip::Horizontal) || (flip == Flip::Reverse);
    flip_vertical = (flip == Flip::Vertical) || (flip == Flip::Reverse);
}

void QtMultimediaCamera::SetEffect(Service::CAM::Effect effect) {
    if (effect != Service::CAM::Effect::None) {
        LOG_ERROR(Service_CAM, "Unimplemented effect %d", static_cast<int>(effect));
    }
}

std::vector<u16> QtMultimediaCamera::ReceiveFrame() {
    return CameraUtil::ProcessImage(camera_surface.current_frame, width, height, output_rgb,
                                    flip_horizontal, flip_vertical);
}

void QtMultimediaCamera::OnServicePaused() {
    camera->stop();
}

void QtMultimediaCamera::OnServiceResumed() {
    camera->start();
}

void QtMultimediaCamera::OnServiceStopped() {
    camera->stop();
}

bool QtMultimediaCamera::CanReceiveFrame() {
    return camera->isAvailable();
}

std::unique_ptr<CameraInterface> QtMultimediaCameraFactory::Create(
    const std::string& config) const {
    return std::make_unique<QtMultimediaCamera>(config);
}

} // namespace Camera
