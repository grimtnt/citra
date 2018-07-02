#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <QCheckBox>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include "citra_qt/cheatsearch.h"
#include "common/common_types.h"
#include "core/memory.h"
#include "ui_cheatsearch.h"

CheatSearch::CheatSearch(QWidget* parent)
    : QDialog(parent), ui(std::make_unique<Ui::CheatSearch>()) {
    ui->setupUi(this);
    ui->btnNextScan->setEnabled(false);
    ui->lblTo->setVisible(false);
    ui->txtSearchTo->setVisible(false);
    ui->tableFound->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableFound->setSelectionBehavior(QAbstractItemView::SelectRows);

    previous_found.clear();

    connect(ui->btnNextScan, &QPushButton::clicked, this, [this] { OnScan(true); });
    connect(ui->btnFirstScan, &QPushButton::clicked, this, [this] { OnScan(false); });
    connect(ui->cbScanType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int index) { OnScanTypeChanged(index); });
    connect(ui->cbValueType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int index) { OnValueTypeChanged(index); });
    connect(ui->chkHex, &QCheckBox::clicked, this, [this](bool i) { OnHexCheckedChanged(i); });
    connect(ui->tableFound, &QTableWidget::doubleClicked, this, [&](QModelIndex i) {
        ModifyAddressDialog* dialog =
            new ModifyAddressDialog(this, ui->tableFound->item(i.row(), 0)->text().toStdString(),
                                    ui->cbValueType->currentIndex(),
                                    ui->tableFound->item(i.row(), 1)->text().toStdString());
        dialog->exec();
        QString rv = dialog->return_value;
        ui->tableFound->item(i.row(), 1)->setText(rv);
    });

    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);
    setSizeGripEnabled(false);
    setFixedSize(size());
}

CheatSearch::~CheatSearch() {}

template <typename T>
T Read(const VAddr addr) {
    if (std::is_same<T, u8>::value) {
        return Memory::Read8(addr);
    } else if (std::is_same<T, u16>::value) {
        return Memory::Read16(addr);
    } else if (std::is_same<T, u32>::value) {
        return Memory::Read32(addr);
    }
}

std::string int_to_hex(int i) {
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(sizeof(int) * 2) << std::hex << i;
    return ss.str();
}

int hex_to_int(std::string hex_value) {
    int decimal_value;
    std::stringstream ss;
    ss << hex_value;
    ss >> std::hex >> decimal_value;
    return decimal_value;
}

double hexstr2double(const std::string& hexstr) {
    union {
        long long i;
        double d;
    } value;

    value.i = std::stoll(hexstr, nullptr, 16);

    return value.d;
}

std::string double2hexstr(double x) {
    union {
        long long i;
        double d;
    } value;

    value.d = x;

    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(16) << value.i;

    return oss.str();
}

std::string ieee_float_to_hex(float f) {
    union {
        float fval;
        std::uint32_t ival;
    };
    fval = f;

    std::ostringstream oss;
    oss << std::hex << std::uppercase << ival;

    return oss.str();
}

void CheatSearch::OnScan(bool isNextScan) {
    int valueType;
    int searchType;
    QString searchvalue;
    bool convertHex;

    try {
        valueType = ui->cbValueType->currentIndex();
        searchType = ui->cbScanType->currentIndex();
        searchvalue = ui->txtSearch->text();
        convertHex = ui->chkHex->isChecked();
    } catch (const std::exception&) {
        ui->txtSearch->clear();
        return;
    }

    std::function<bool(int, int, int)> comparer = [&](int a, int b, int c) {
        return Equals(a, b, c);
    };

    switch (searchType) {
    case 0: { // Equals
        comparer = [&](int a, int b, int c) { return CheatSearch::Equals(a, b, c); };
        break;
    }
    case 1: { // Greater Than
        comparer = [&](int a, int b, int c) { return CheatSearch::GreaterThan(a, b, c); };
        break;
    }
    case 2: { // Less Than
        comparer = [&](int a, int b, int c) { return CheatSearch::LessThan(a, b, c); };
        break;
    }
    case 3: { // Between
        comparer = [&](int a, int b, int c) { return CheatSearch::Between(a, b, c); };
        break;
    }
    }

    int base = (ui->chkHex->isChecked()) ? 16 : 10;

    switch (valueType) {
    case 0: { // u32
        u32 value = searchvalue.toUInt(nullptr, base);
        if (!isNextScan)
            previous_found = FirstSearch<u32>(value, comparer);
        else
            previous_found = NextSearch<u32>(value, comparer, previous_found);
        break;
    }

    case 1: { // u16
        u16 value = searchvalue.toUShort(nullptr, base);
        if (!isNextScan)
            previous_found = FirstSearch<u16>(value, comparer);
        else
            previous_found = NextSearch<u16>(value, comparer, previous_found);
        break;
    }
    case 2: { // u8
        u8 value = static_cast<u8>(searchvalue.toUShort(nullptr, base));
        if (!isNextScan)
            previous_found = FirstSearch<u8>(value, comparer);
        else
            previous_found = NextSearch<u8>(value, comparer, previous_found);
        break;
    }
    }

    ui->tableFound->setRowCount(0);

    if (previous_found.size() > 50000) {
        ui->lblCount->setText(tr("Found: 50000+"));
    } else {
        LoadTable(previous_found);
        ui->lblCount->setText(tr("Found: %1").arg(previous_found.size()));
    }

    ui->btnNextScan->setEnabled(previous_found.size() > 0);
}

void CheatSearch::OnValueTypeChanged(int index) {
    ui->txtSearch->clear();
    ui->txtSearchTo->clear();
    if (index >= 0 && index <= 2) {
        ui->chkHex->setVisible(true);
    } else {
        ui->chkHex->setVisible(false);
        ui->chkHex->setChecked(false);
    }
}

void CheatSearch::OnScanTypeChanged(int index) {
    if (index == 3) { // Between
        ui->lblTo->setVisible(true);
        ui->txtSearchTo->setVisible(true);
    } else {
        ui->lblTo->setVisible(false);
        ui->txtSearchTo->setVisible(false);
        ui->txtSearchTo->clear();
    }

    if (index == 0) { // Equals
        ui->chkNot->setVisible(true);
    } else {
        ui->chkNot->setVisible(false);
        ui->chkNot->setChecked(false);
    }
}

void CheatSearch::OnHexCheckedChanged(bool checked) {
    std::string text = ui->txtSearch->text().toStdString();
    std::string text2 = ui->txtSearchTo->text().toStdString();

    try {
        if (checked) {
            if (text.length() > 0) {
                int val = std::stoi(text, nullptr, 10);
                ui->txtSearch->setText(QString::fromStdString(int_to_hex(val)));
            }

            if (text2.length() > 0) {
                int val2 = std::stoi(text2, nullptr, 10);
                ui->txtSearchTo->setText(QString::fromStdString(int_to_hex(val2)));
            }
        } else {
            if (text.length() > 0) {
                ui->txtSearch->setText(QString::fromStdString(std::to_string(hex_to_int(text))));
            }

            if (text2.length() > 0) {
                ui->txtSearchTo->setText(QString::fromStdString(std::to_string(hex_to_int(text2))));
            }
        }
    } catch (const std::exception&) {
        ui->txtSearch->clear();
        ui->txtSearchTo->clear();
    }
}

void CheatSearch::LoadTable(const std::vector<FoundItem>& items) {
    ui->tableFound->setRowCount(static_cast<int>(items.size()));

    for (int i = 0; i < items.size(); i++) {
        ui->tableFound->setItem(
            i, 0, new QTableWidgetItem(QString::fromStdString(items[i].address).toUpper()));
        ui->tableFound->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(items[i].value)));
        ui->tableFound->setRowHeight(i, 23);
    }
}

template <typename T>
std::vector<FoundItem> CheatSearch::FirstSearch(const T value,
                                                std::function<bool(int, int, int)> comparer) {
    u32 start_address = 0x00000000;
    u32 end_address = 0x08000000 + 0x08000000;
    std::vector<int> address_in_use;
    std::vector<FoundItem> results;
    int base = (ui->chkHex->isChecked()) ? 16 : 10;
    T searchToValue = static_cast<T>(ui->txtSearchTo->text().toInt(nullptr, base));

    for (u32 i = start_address; i < end_address; i += 4096) {
        if (Memory::IsValidVirtualAddress(i)) {
            address_in_use.push_back(i);
        }
    }

    for (auto& range : address_in_use) {
        for (int i = range; i < (range + 4096); i++) {
            T result = Read<T>(i);
            if (comparer(result, value, searchToValue)) {
                FoundItem item;
                item.address = int_to_hex(i);
                item.value = std::to_string(result);
                results.push_back(item);
            }
        }
    }
    return results;
}

template <typename T>
std::vector<FoundItem> CheatSearch::NextSearch(const T value,
                                               std::function<bool(int, int, int)> comparer,
                                               std::vector<FoundItem> previousFound) {
    std::vector<FoundItem> results;
    int base = (ui->chkHex->isChecked()) ? 16 : 10;
    T searchToValue = static_cast<T>(ui->txtSearchTo->text().toUInt(nullptr, base));

    for (auto& f : previousFound) {
        VAddr addr = std::stoul(f.address, nullptr, 16);
        T result = Read<T>(addr);

        if (comparer(result, value, searchToValue)) {
            FoundItem item;
            item.address = int_to_hex(addr);
            item.value = std::to_string(result);
            results.push_back(item);
        }
    }

    return results;
}

bool CheatSearch::Equals(int a, int b, int c) {
    return ui->chkNot->isChecked() ? (a != b) : (a == b);
}

bool CheatSearch::LessThan(int a, int b, int c) {
    return a < b;
}

bool CheatSearch::GreaterThan(int a, int b, int c) {
    return a > b;
}

bool CheatSearch::Between(int value, int min, int max) {
    return min < value && value < max;
}

ModifyAddressDialog::ModifyAddressDialog(QWidget* parent, std::string address, int type,
                                         std::string value)
    : QDialog(parent) {
    resize(300, 30);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    setWindowTitle(tr("Modify Address"));
    setSizeGripEnabled(false);
    auto mainLayout = new QVBoxLayout(this);

    QHBoxLayout* editPanel = new QHBoxLayout();
    address_block = new QLineEdit();
    value_block = new QLineEdit();
    type_select = new QComboBox();

    address_block->setReadOnly(true);
    address_block->setText(QString::fromStdString(address));

    type_select->addItem("u32");
    type_select->addItem("u16");
    type_select->addItem("u8");
    type_select->addItem("float");
    type_select->addItem("double");
    type_select->setCurrentIndex(type);

    value_block->setText(QString::fromStdString(value));

    editPanel->addWidget(address_block);
    editPanel->addWidget(type_select);
    editPanel->addWidget(value_block);

    auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(button_box, &QDialogButtonBox::accepted, this, [=]() { OnOkClicked(); });
    QHBoxLayout* confirmationPanel = new QHBoxLayout();
    confirmationPanel->addWidget(button_box);
    mainLayout->addLayout(editPanel);
    mainLayout->addLayout(confirmationPanel);
}

ModifyAddressDialog::~ModifyAddressDialog() {}

void ModifyAddressDialog::OnOkClicked() {
    int valueType;
    QString newValue;
    int address;

    try {
        valueType = type_select->currentIndex();
        newValue = value_block->text();
        address = address_block->text().toInt(nullptr, 16);
    } catch (const std::exception&) {
        this->close();
    }

    int base = 10;

    switch (valueType) {
    case 0: { // u32
        u32 value = newValue.toUInt(nullptr, base);
        Memory::Write32(address, value);
        break;
    }
    case 1: { // u16
        u16 value = newValue.toUShort(nullptr, base);
        Memory::Write16(address, value);
        break;
    }
    case 2: { // u8
        u8 value = static_cast<u8>(newValue.toUShort(nullptr, base));
        Memory::Write8(address, value);
        break;
    }
    case 3: { // float
        float value = newValue.toFloat();
        u32 converted = std::stoul(ieee_float_to_hex(value), nullptr, 10);
        Memory::Write32(address, converted);
        break;
    }
    case 4: { // double
        double value = newValue.toDouble();
        u64 converted = std::stoull(double2hexstr(value), nullptr, 10);
        Memory::Write64(address, converted);
        break;
    }
    }

    return_value = newValue;
    this->close();
}
