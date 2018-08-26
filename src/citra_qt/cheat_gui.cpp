#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QTableWidgetItem>
#include "citra_qt/cheat_gui.h"
#include "common/string_util.h"
#include "core/hle/kernel/process.h"
#include "ui_cheat_gui.h"

CheatDialog::CheatDialog(QWidget* parent)
    : QDialog{parent}, ui{std::make_unique<Ui::CheatDialog>()} {
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);
    ui->setupUi(this);
    ui->tableCheats->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableCheats->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableCheats->setColumnWidth(2, 85);
    ui->tableCheats->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->tableCheats->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableCheats->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->textLines->setEnabled(false);
    ui->labelTitle->setText(tr("Title ID: %1")
                                .arg(QString::fromStdString(Common::StringFromFormat(
                                    "%016llX", Kernel::g_current_process->codeset->program_id))));
    connect(ui->buttonClose, &QPushButton::clicked, this, &CheatDialog::OnCancel);
    connect(ui->buttonNewCheat, &QPushButton::clicked, this, &CheatDialog::OnAddCheat);
    connect(ui->buttonSave, &QPushButton::clicked, this, &CheatDialog::OnSave);
    connect(ui->buttonDelete, &QPushButton::clicked, this, &CheatDialog::OnDelete);
    connect(ui->tableCheats, &QTableWidget::cellClicked, this, &CheatDialog::OnRowSelected);
    connect(ui->textLines, &QPlainTextEdit::textChanged, this, &CheatDialog::OnLinesChanged);
    LoadCheats();
}

CheatDialog::~CheatDialog() {}

void CheatDialog::LoadCheats() {
    cheats = CheatEngine::CheatEngine::ReadFileContents();

    ui->tableCheats->setRowCount(static_cast<int>(cheats.size()));

    for (int i = 0; i < static_cast<int>(cheats.size()); i++) {
        QCheckBox* enabled{new QCheckBox()};
        enabled->setChecked(cheats[i]->GetEnabled());
        enabled->setStyleSheet("margin-left:7px;");
        ui->tableCheats->setItem(i, 0, new QTableWidgetItem());
        ui->tableCheats->setCellWidget(i, 0, enabled);
        ui->tableCheats->setItem(
            i, 1, new QTableWidgetItem(QString::fromStdString(cheats[i]->GetName())));
        ui->tableCheats->setItem(
            i, 2, new QTableWidgetItem(QString::fromStdString(cheats[i]->GetType())));
        enabled->setProperty("row", static_cast<int>(i));

        ui->tableCheats->setRowHeight(i, 23);
        connect(enabled, &QCheckBox::stateChanged, this, &CheatDialog::OnCheckChanged);
    }
}

void CheatDialog::OnSave() {
    bool error{false};
    QString error_message{tr("The following cheats are empty:\n\n%1")};
    QStringList empty_cheat_names{};
    for (auto& cheat : cheats) {
        if (cheat->GetCheatLines().empty()) {
            empty_cheat_names.append(QString::fromStdString(cheat->GetName()));
            error = true;
        }
    }
    if (error) {
        QMessageBox::critical(this, tr("Error"), error_message.arg(empty_cheat_names.join('\n')));
        return;
    }

    CheatEngine::CheatEngine::Save(cheats);
    CheatCore::RefreshCheats();
    close();
}

void CheatDialog::OnCancel() {
    close();
}

void CheatDialog::OnRowSelected(int row, int column) {
    selection_changing = true;
    if (row == -1) {
        ui->textLines->setPlainText("");
        current_row = -1;
        selection_changing = false;
        ui->textLines->setEnabled(false);
        return;
    }

    ui->textLines->setEnabled(true);
    const auto& current_cheat{cheats[row]};

    std::vector<std::string> lines{};
    for (const auto& line : current_cheat->GetCheatLines())
        lines.push_back(line.cheat_line);
    ui->textLines->setPlainText(QString::fromStdString(Common::Join(lines, "\n")));

    current_row = row;
    selection_changing = false;
}

void CheatDialog::OnLinesChanged() {
    if (selection_changing)
        return;
    QString lines{ui->textLines->toPlainText()};
    std::vector<std::string> lines_vec{};
    Common::SplitString(lines.toStdString(), '\n', lines_vec);
    auto new_lines{std::vector<CheatEngine::CheatLine>()};
    for (const auto& line : lines_vec) {
        new_lines.emplace_back(line);
    }
    cheats[current_row]->SetCheatLines(new_lines);
}

void CheatDialog::OnCheckChanged(int state) {
    QCheckBox* checkbox{qobject_cast<QCheckBox*>(sender())};
    int row{static_cast<int>(checkbox->property("row").toInt())};
    cheats[row]->SetEnabled(state);
}

void CheatDialog::OnDelete() {
    QItemSelectionModel* selectionModel{ui->tableCheats->selectionModel()};
    QModelIndexList selected{selectionModel->selectedRows()};
    std::vector<int> rows{};
    for (int i = selected.count() - 1; i >= 0; i--) {
        QModelIndex index{selected.at(i)};
        int row{index.row()};
        cheats.erase(cheats.begin() + row);
        rows.push_back(row);
    }
    for (int row : rows)
        ui->tableCheats->removeRow(row);

    ui->tableCheats->clearSelection();
    OnRowSelected(-1, -1);
}

void CheatDialog::OnAddCheat() {
    NewCheatDialog dialog;
    dialog.exec();

    auto result{dialog.GetReturnValue()};
    if (result == nullptr)
        return;
    cheats.push_back(result);
    int new_cheat_index{static_cast<int>(cheats.size() - 1)};
    QCheckBox* enabled{new QCheckBox()};
    ui->tableCheats->setRowCount(static_cast<int>(cheats.size()));
    enabled->setCheckState(Qt::CheckState::Unchecked);
    enabled->setStyleSheet("margin-left:7px;");
    ui->tableCheats->setItem(new_cheat_index, 0, new QTableWidgetItem());
    ui->tableCheats->setCellWidget(new_cheat_index, 0, enabled);
    ui->tableCheats->setItem(
        new_cheat_index, 1,
        new QTableWidgetItem(QString::fromStdString(cheats[new_cheat_index]->GetName())));
    ui->tableCheats->setItem(
        new_cheat_index, 2,
        new QTableWidgetItem(QString::fromStdString(cheats[new_cheat_index]->GetType())));
    ui->tableCheats->setRowHeight(new_cheat_index, 23);
    enabled->setProperty("row", new_cheat_index);
    connect(enabled, &QCheckBox::stateChanged, this, &CheatDialog::OnCheckChanged);
    ui->tableCheats->selectRow(new_cheat_index);
    OnRowSelected(new_cheat_index, 0);
}

NewCheatDialog::NewCheatDialog(QWidget* parent) : QDialog(parent) {
    resize(166, 115);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    setWindowTitle(tr("New Cheat"));
    QVBoxLayout* main_layout{new QVBoxLayout(this)};
    QHBoxLayout* name_panel{new QHBoxLayout()};
    QLabel* name_label{new QLabel()};
    name_block = new QLineEdit();
    name_label->setText(tr("Name: "));
    name_panel->addWidget(name_label);
    name_panel->addWidget(name_block);

    QHBoxLayout* type_panel{new QHBoxLayout()};
    QLabel* type_label{new QLabel()};
    type_select = new QComboBox();
    type_label->setText(tr("Type: "));
    type_select->addItem(tr("Gateway"), 0);
    type_panel->addWidget(type_label);
    type_panel->addWidget(type_select);

    QDialogButtonBox* button_box{
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel)};
    connect(button_box, &QDialogButtonBox::accepted, this, [&]() {
        std::string name{name_block->text().toStdString()};
        if (type_select->currentIndex() == 0 && Common::Trim(name).length() > 0) {
            return_value =
                std::make_shared<CheatEngine::GatewayCheat>(name_block->text().toStdString());
        }
        close();
    });
    connect(button_box, &QDialogButtonBox::rejected, this, [&]() { close(); });
    QHBoxLayout* confirmation_panel{new QHBoxLayout()};
    confirmation_panel->addWidget(button_box);
    main_layout->addLayout(name_panel);
    main_layout->addLayout(type_panel);
    main_layout->addLayout(confirmation_panel);
}

NewCheatDialog::~NewCheatDialog() {}
