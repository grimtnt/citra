// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <QImage>
#ifdef ENABLE_OPENCV_CAMERA
#include <opencv2/core.hpp>
#endif
#include "common/math_util.h"
#include "core/frontend/camera/factory.h"
#include "core/frontend/camera/interface.h"

namespace CameraUtil {

std::vector<u16> Rgb2Yuv(QImage source, int width, int height);
std::vector<u16> ProcessImage(QImage source, int width, int height, bool output_rgb,
                              bool flip_horizontal, bool flip_vertical);
#ifdef ENABLE_OPENCV_CAMERA
QImage cvMat2QImage(const cv::Mat& mat);
#endif

} // namespace CameraUtil
