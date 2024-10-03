#ifndef ERRORLOGDIALOG_H
#define ERRORLOGDIALOG_H

#include <QDialog>

namespace Ui {
class ErrorLogDialog;
}

class ErrorLogDialog : public QDialog {
  Q_OBJECT

public:
  explicit ErrorLogDialog(QWidget *parent = nullptr);
  ~ErrorLogDialog();
  void setLogMessage(const QString &text);

private:
  Ui::ErrorLogDialog *ui;
};

#endif // ERRORLOGDIALOG_H
