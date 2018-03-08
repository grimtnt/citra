#include <cctype>
#include <fstream>
#include <iostream>
#include "mainwindow.h"
#include "ui_mainwindow.h"

#define D_TITLE "Keyboard"
#define D_PLACEHOLDER "Type text"
#define D_TEXT ""
#define D_MAX_LENGTH INT_MAX
#define D_CANCEL_TEXT "Cancel"
#define D_I_FORGOT_TEXT "I Forgot"
#define D_OK_TEXT "Ok"

bool caps_lock = false;
bool dcaps = false;
bool dshift = false;
bool shift = false;
bool fixed_length = false;
int valid_input;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->Ok->setEnabled(false);
    setWindowTitle("Keyboard");
    setFixedSize(size());
    QStringList args = QApplication::arguments();

    /**
     * Arguments.
     * 1: Title
     * 2: Placeholder
     * 3: Default
     * 4: Max
     * 5: Cancel text
     * 6: I Forgot text
     * 7: Ok text
     * 8: Fixed Length
     * 9: Valid input
     */

    // Title
    if (args.count() >= 2) {
        if (args[1] == "_def")
            this->setWindowTitle(D_TITLE);
        else
            this->setWindowTitle(args[1]);
    }

    // Placeholder
    if (args.count() >= 3) {
        if (args[2] == "_def")
            ui->Text->setPlaceholderText(D_PLACEHOLDER);
        else
            ui->Text->setPlaceholderText(args[2]);
    }

    // Default text
    if (args.count() >= 4) {
        if (args[3] == "_def")
            ui->Text->setText(D_TEXT);
        else
            ui->Text->setText(args[3]);
    }

    // Max length
    if (args.count() >= 5) {
        if (args[4] == "_def")
            ui->Text->setMaxLength(D_MAX_LENGTH);
        else
            ui->Text->setMaxLength(args[4].toInt());
    }

    // Cancel text
    if (args.count() >= 6) {
        if (args[5] == "_def")
            ui->Cancel->setText(D_CANCEL_TEXT);
        else
            ui->Cancel->setText(args[5]);
    }

    // I Forgot text
    if (args.count() >= 7) {
        if (args[6] == "_def")
            ui->I_Forgot->setText(D_I_FORGOT_TEXT);
        else
            ui->I_Forgot->setText(args[6]);
    }

    // Ok text
    if (args.count() >= 8) {
        if (args[7] == "_def")
            ui->Ok->setText(D_OK_TEXT);
        else
            ui->Ok->setText(args[7]);
    }

    // Fixed length
    if (args.count() >= 9) {
        if (args[8] == "1")
            fixed_length = true;
    }

    // Valid input
    if (args.count() >= 10)
        valid_input = args[9].toInt();

    validateText("");
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_Text_textChanged(const QString& arg1) {
    validateText(arg1);
}

void MainWindow::validateText(QString text) {
    std::string input = text.toStdString();
    switch (valid_input) {
    case 0: {
        ui->Ok->setEnabled(true);
        break;
    }
    case 1: {
        if (input.empty())
            ui->Ok->setEnabled(false);
        else
            ui->Ok->setEnabled(true);
        break;
    }
    case 2: {
        bool valid =
            std::any_of(input.begin(), input.end(), [](const char c) { return !std::isspace(c); });
        if (valid)
            ui->Ok->setEnabled(!input.empty());
        else
            ui->Ok->setEnabled(false);
        break;
    }
    case 3: {
        bool valid =
            std::any_of(input.begin(), input.end(), [](const char c) { return !std::isspace(c); });
        if (valid)
            ui->Ok->setEnabled(true);
        else
            ui->Ok->setEnabled(false);
        break;
    }
    case 4: {
        if (fixed_length) {
            if (input.length() == ui->Text->maxLength())
                ui->Ok->setEnabled(true);
            else
                ui->Ok->setEnabled(false);
        } else {
            ui->Ok->setEnabled(true);
        }
        break;
    }
    }
}

void MainWindow::ShiftToggled(bool state) {
    ui->Number1->setVisible(!state);
    ui->LetterSTilde->setVisible(state);
    ui->Number2->setVisible(!state);
    ui->LetterSPoundSign->setVisible(state);
    ui->Number3->setVisible(!state);
    ui->LetterSEuro->setVisible(state);
}

void MainWindow::on_CapsLock_toggled(bool checked) {
    caps_lock = checked;
    if (shift && dshift) {
        shift = false;
        ui->Shift->setChecked(false);
    }
    dshift = true;
    dcaps = true;
}

void MainWindow::on_Shift_toggled(bool checked) {
    shift = checked;
    if (caps_lock && dcaps) {
        caps_lock = false;
        dshift = false;
        ui->CapsLock->setChecked(false);
    }
    dcaps = false;
    ShiftToggled(checked);
}

void MainWindow::on_Cancel_clicked() {
    std::ofstream ofs;
    ofs.open("text.txt", std::ofstream::out | std::ofstream::trunc);
    ofs << "";
    ofs.close();
    exit(0);
}

void MainWindow::on_I_Forgot_clicked() {
    std::ofstream ofs;
    ofs.open("text.txt", std::ofstream::out | std::ofstream::trunc);
    ofs << ui->Text->text().toStdString();
    ofs.close();
    exit(1);
}

void MainWindow::on_Ok_clicked() {
    std::ofstream ofs;
    ofs.open("text.txt", std::ofstream::out | std::ofstream::trunc);
    ofs << ui->Text->text().toStdString();
    ofs.close();
    exit(2);
}

void MainWindow::press(QString key) {
    if (shift) {
        ui->Text->setText(ui->Text->text().append(key.toUpper()));
        shift = false;
        ui->Shift->setChecked(false);
        return;
    }

    if (caps_lock) {
        ui->Text->setText(ui->Text->text().append(key.toUpper()));
    } else {
        ui->Text->setText(ui->Text->text().append(key.toLower()));
    }
}

void MainWindow::on_Number1_clicked() {
    press("1");
}

void MainWindow::on_LetterSTilde_clicked() {
    press("~");
}

void MainWindow::on_Number2_clicked() {
    press("2");
}

void MainWindow::on_LetterSPoundSign_clicked() {
    press("£");
}

void MainWindow::on_Number3_clicked() {
    press("3");
}

void MainWindow::on_LetterSEuro_clicked() {
    press("€");
}

void MainWindow::on_Number4_clicked() {
    press("4");
}

void MainWindow::on_Number5_clicked() {
    press("5");
}

void MainWindow::on_Number6_clicked() {
    press("6");
}

void MainWindow::on_Number7_clicked() {
    press("7");
}

void MainWindow::on_Number8_clicked() {
    press("8");
}

void MainWindow::on_Number9_clicked() {
    press("9");
}

void MainWindow::on_Number0_clicked() {
    press("0");
}

void MainWindow::on_LetterSDash_clicked() {
    press("-");
}

void MainWindow::on_LetterQ_clicked() {
    press("q");
}

void MainWindow::on_LetterW_clicked() {
    press("w");
}

void MainWindow::on_LetterE_clicked() {
    press("e");
}

void MainWindow::on_LetterR_clicked() {
    press("r");
}

void MainWindow::on_LetterT_clicked() {
    press("t");
}

void MainWindow::on_LetterY_clicked() {
    press("y");
}

void MainWindow::on_LetterU_clicked() {
    press("u");
}

void MainWindow::on_LetterI_clicked() {
    press("i");
}

void MainWindow::on_LetterO_clicked() {
    press("o");
}

void MainWindow::on_LetterP_clicked() {
    press("p");
}

void MainWindow::on_NewLine_clicked() {
    ui->Text->setText("");
}

void MainWindow::on_LetterA_clicked() {
    press("a");
}

void MainWindow::on_LetterS_clicked() {
    press("s");
}

void MainWindow::on_LetterD_clicked() {
    press("d");
}

void MainWindow::on_LetterF_clicked() {
    press("f");
}

void MainWindow::on_LetterG_clicked() {
    press("g");
}

void MainWindow::on_LetterH_clicked() {
    press("h");
}

void MainWindow::on_LetterJ_clicked() {
    press("j");
}

void MainWindow::on_LetterK_clicked() {
    press("k");
}

void MainWindow::on_LetterL_clicked() {
    press("L");
}

void MainWindow::on_LetterSap_clicked() {
    press("'");
}

void MainWindow::on_LetterSequ_clicked() {
    press("=");
}

void MainWindow::on_LetterSvb_clicked() {
    press("/");
}

void MainWindow::on_LetterZ_clicked() {
    press("z");
}

void MainWindow::on_LetterX_clicked() {
    press("x");
}

void MainWindow::on_LetterC_clicked() {
    press("c");
}

void MainWindow::on_LetterV_clicked() {
    press("v");
}

void MainWindow::on_LetterB_clicked() {
    press("b");
}

void MainWindow::on_LetterN_clicked() {
    press("n");
}

void MainWindow::on_LetterM_clicked() {
    press("m");
}

void MainWindow::on_LetterSat_clicked() {
    press("@");
}

void MainWindow::on_LetterSComma_clicked() {
    press(",");
}

void MainWindow::on_LetterSPoint_clicked() {
    press(".");
}

void MainWindow::on_LetterSQuestionMark_clicked() {
    press("?");
}

void MainWindow::on_LetterSExclamationMark_clicked() {
    press("!");
}

void MainWindow::on_Delete_clicked() {
    ui->Text->setText(ui->Text->text().chopped(1));
}

void MainWindow::on_Space_clicked() {
    press(" ");
}
