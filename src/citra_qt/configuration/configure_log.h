// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include <QWidget>
#include "common/logging/backend.h"
#include "common/logging/filter.h"
#include "common/logging/log.h"

namespace Ui {
class ConfigureLog;
}

class LogFilterModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit LogFilterModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;

private:
};

class LogLevelDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    LogLevelDelegate(QObject* parent = 0);

    QString displayText(const QVariant& value, const QLocale& locale) const override;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override;

    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const override;

private:
    static const QStringList level_names;
};

class ConfigureLog : public QWidget {
    Q_OBJECT

public:
    explicit ConfigureLog(QWidget* parent = nullptr);
    ~ConfigureLog();

    void applyConfiguration();

private:
    void setConfiguration();

private:
    std::unique_ptr<Ui::ConfigureLog> ui;
    LogFilterModel log_filter_model;
    LogLevelDelegate log_level_delegate;
};
