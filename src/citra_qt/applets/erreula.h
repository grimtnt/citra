// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <QWidget>
#include "core/frontend/applets/erreula.h"

class QtErrEula final : public QObject, public Frontend::ErrorEula {
    Q_OBJECT

public:
    explicit QtErrEula(QWidget* parent);
    void Setup(const Frontend::ErrorEulaConfig* config) override;

private:
    Q_INVOKABLE void ShowMessage();

    QWidget* parent;
};
