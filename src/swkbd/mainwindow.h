#pragma once

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

private slots:
    void on_Text_textChanged(const QString& arg1);
    void validateText(QString text);
    void ShiftToggled(bool state);
    void on_CapsLock_toggled(bool checked);
    void on_Shift_toggled(bool checked);
    void on_Cancel_clicked();
    void on_I_Forgot_clicked();
    void on_Ok_clicked();
    void press(QString key);
    void on_Number1_clicked();
    void on_LetterSTilde_clicked();
    void on_Number2_clicked();
    void on_LetterSPoundSign_clicked();
    void on_Number3_clicked();
    void on_LetterSEuro_clicked();
    void on_Number4_clicked();
    void on_Number5_clicked();
    void on_Number6_clicked();
    void on_Number7_clicked();
    void on_Number8_clicked();
    void on_Number9_clicked();
    void on_Number0_clicked();
    void on_LetterSDash_clicked();
    void on_LetterQ_clicked();
    void on_LetterW_clicked();
    void on_LetterE_clicked();
    void on_LetterR_clicked();
    void on_LetterT_clicked();
    void on_LetterY_clicked();
    void on_LetterU_clicked();
    void on_LetterI_clicked();
    void on_LetterO_clicked();
    void on_LetterP_clicked();
    void on_NewLine_clicked();
    void on_LetterA_clicked();
    void on_LetterS_clicked();
    void on_LetterD_clicked();
    void on_LetterF_clicked();
    void on_LetterG_clicked();
    void on_LetterH_clicked();
    void on_LetterJ_clicked();
    void on_LetterK_clicked();
    void on_LetterL_clicked();
    void on_LetterSap_clicked();
    void on_LetterSequ_clicked();
    void on_LetterSvb_clicked();
    void on_LetterZ_clicked();
    void on_LetterX_clicked();
    void on_LetterC_clicked();
    void on_LetterV_clicked();
    void on_LetterB_clicked();
    void on_LetterN_clicked();
    void on_LetterM_clicked();
    void on_LetterSat_clicked();
    void on_LetterSComma_clicked();
    void on_LetterSPoint_clicked();
    void on_LetterSQuestionMark_clicked();
    void on_LetterSExclamationMark_clicked();
    void on_Delete_clicked();
    void on_Space_clicked();

private:
    Ui::MainWindow* ui;
};
