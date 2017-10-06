// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <algorithm>
#include <QComboBox>
#include "citra_qt/configuration/configure_log.h"
#include "core/settings.h"

LogFilterModel::LogFilterModel(QObject* parent) : QAbstractItemModel(parent) {}

QVariant LogFilterModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return {};

    Log::Class log_class = static_cast<Log::Class>(index.internalId());

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (index.column() == 0)
            return QString(Log::GetLogClassName(log_class));
        else
            return static_cast<int>(Log::GetFilter()->GetClassLevel(log_class));
    default:
        return {};
    }
}

bool LogFilterModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (role == Qt::EditRole && index.column() == 1) {
        Log::Class log_class = static_cast<Log::Class>(index.internalId());
        Log::GetFilter()->SetClassLevel(log_class, static_cast<Log::Level>(value.toInt()));
        return true;
    }
    return false;
}

QVariant LogFilterModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return tr("Class");
        case 1:
            return tr("Level");
        }
    } else {
        return {};
    }
}

Qt::ItemFlags LogFilterModel::flags(const QModelIndex& index) const {
    if (!index.isValid())
        return {};
    if (index.column() == 0)
        return QAbstractItemModel::flags(index);
    else
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QModelIndex LogFilterModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent))
        return {};

    auto& groups = Log::g_class_hierarchy.Groups();

    if (parent.isValid()) {
        return createIndex(row, column,
                           static_cast<quintptr>(groups[parent.row()].sub_classes[row]));
    }

    return createIndex(row, column, static_cast<quintptr>(groups[row].parent));
}

QModelIndex LogFilterModel::parent(const QModelIndex& index) const {
    if (!index.isValid())
        return {};

    auto& groups = Log::g_class_hierarchy.Groups();
    auto iter =
        std::find_if(groups.begin(), groups.end(),
                     [sub_class = static_cast<Log::Class>(index.internalId())](const auto& group) {
                         return std::find(group.sub_classes.begin(), group.sub_classes.end(),
                                          sub_class) != group.sub_classes.end();
                     });
    if (iter == groups.end())
        return {};

    return createIndex(std::distance(groups.begin(), iter), 0, static_cast<quintptr>(iter->parent));
}

int LogFilterModel::rowCount(const QModelIndex& parent) const {
    auto& groups = Log::g_class_hierarchy.Groups();

    if (!parent.isValid())
        return static_cast<int>(groups.size());

    if (!parent.parent().isValid())
        return static_cast<int>(groups[parent.row()].sub_classes.size());

    return 0;
}

int LogFilterModel::columnCount(const QModelIndex& parent) const {
    return 2;
}

const QStringList LogLevelDelegate::level_names{{
    tr("Trace"), tr("Debug"), tr("Info"), tr("Warning"), tr("Error"), tr("Critical"),
}};

LogLevelDelegate::LogLevelDelegate(QObject* parent) : QStyledItemDelegate(parent) {}
QString LogLevelDelegate::displayText(const QVariant& value, const QLocale& locale) const {
    return level_names[qvariant_cast<int>(value)];
}
QWidget* LogLevelDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const {
    QComboBox* editor = new QComboBox(parent);
    editor->addItems(level_names);
    editor->setEditable(false);
    return editor;
}

void LogLevelDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    int value = index.model()->data(index, Qt::EditRole).toInt();
    static_cast<QComboBox*>(editor)->setCurrentIndex(value);
}

void LogLevelDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                    const QModelIndex& index) const {
    model->setData(index, static_cast<QComboBox*>(editor)->currentIndex(), Qt::EditRole);
}

void LogLevelDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                                            const QModelIndex& index) const {
    editor->setGeometry(option.rect);
}

ConfigureLog::ConfigureLog(QWidget* parent) : QWidget(parent), ui(new Ui::ConfigureLog) {
    ui->setupUi(this);
    this->setConfiguration();
    ui->log_tree->setModel(&log_filter_model);
    ui->log_tree->setItemDelegateForColumn(1, &log_level_delegate);
}

ConfigureLog::~ConfigureLog() {}

void ConfigureLog::setConfiguration() {}

void ConfigureLog::applyConfiguration() {
    Settings::Apply();
}
