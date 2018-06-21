// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "citra_qt/swkbd.h"
#include "ui_swkbd.h"

using namespace HLE::Applets;

SoftwareKeyboardDialog::SoftwareKeyboardDialog(QWidget* parent,
                                               HLE::Applets::SoftwareKeyboardConfig& config,
                                               std::u16string& text)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
      ui(new Ui::SoftwareKeyboardDialog) {
    ui->setupUi(this);
    setWindowTitle(tr("Software Keyboard"));
    std::u16string button0_text(reinterpret_cast<char16_t*>(config.button_text[0]));
    std::u16string button1_text(reinterpret_cast<char16_t*>(config.button_text[1]));
    std::u16string button2_text(reinterpret_cast<char16_t*>(config.button_text[2]));
    std::u16string hint_text(reinterpret_cast<char16_t*>(config.hint_text));
    ((config.num_buttons_m1 == SwkbdButtonConfig::SingleButton) ? ui->button0 : ui->button1)
        ->setEnabled(ValidateInput(config, std::string()) == ValidationError::None);
    ui->text->setPlaceholderText(QString::fromStdU16String(hint_text));
    ui->button1->setVisible(config.num_buttons_m1 >= SwkbdButtonConfig::DualButton);
    ui->button2->setVisible(config.num_buttons_m1 == SwkbdButtonConfig::TripleButton);
    ui->button0->setText(button0_text.empty()
                             ? QString::fromStdString(
                                   default_button_text[static_cast<u32>(config.num_buttons_m1)][0])
                             : QString::fromStdU16String(button0_text));
    ui->button1->setText(button1_text.empty()
                             ? QString::fromStdString(
                                   default_button_text[static_cast<u32>(config.num_buttons_m1)][1])
                             : QString::fromStdU16String(button1_text));
    ui->button2->setText(button2_text.empty()
                             ? QString::fromStdString(
                                   default_button_text[static_cast<u32>(config.num_buttons_m1)][2])
                             : QString::fromStdU16String(button2_text));
    connect(ui->text, &QTextEdit::textChanged, [&] {
        ((config.num_buttons_m1 == SwkbdButtonConfig::SingleButton) ? ui->button0 : ui->button1)
            ->setEnabled(ValidateInput(config, ui->text->toPlainText().toStdString()) ==
                         ValidationError::None);
    });
    connect(ui->button0, &QPushButton::clicked, [&](bool) {
        std::u16string utf16_input = ui->text->toPlainText().toStdU16String();
        text = utf16_input;
        config.return_code = results[static_cast<u32>(config.num_buttons_m1)][0];
        config.text_length = static_cast<u16>(utf16_input.size());
        config.text_offset = 0;
        close();
    });
    connect(ui->button1, &QPushButton::clicked, [&](bool) {
        std::u16string utf16_input = ui->text->toPlainText().toStdU16String();
        text = utf16_input;
        config.return_code = results[static_cast<u32>(config.num_buttons_m1)][1];
        config.text_length = static_cast<u16>(utf16_input.size());
        config.text_offset = 0;
        close();
    });
    connect(ui->button2, &QPushButton::clicked, [&](bool) {
        std::u16string utf16_input = ui->text->toPlainText().toStdU16String();
        text = utf16_input;
        config.return_code = results[static_cast<u32>(config.num_buttons_m1)][2];
        config.text_length = static_cast<u16>(utf16_input.size());
        config.text_offset = 0;
        close();
    });
}

SoftwareKeyboardDialog::~SoftwareKeyboardDialog() {
    delete ui;
}
