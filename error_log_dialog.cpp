#include "error_log_dialog.h"
#include "ui_error_log_dialog.h"

ErrorLogDialog::ErrorLogDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ErrorLogDialog) {
  ui->setupUi(this);
}

ErrorLogDialog::~ErrorLogDialog() { delete ui; }

void ErrorLogDialog::setLogMessage(const QString &text) {
  ui->plainTextEdit->setPlainText(text);
}
