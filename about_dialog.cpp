#include "about_dialog.h"
#include "ui_about_dialog.h"
#include <QDialogButtonBox>
#include <QPushButton>

extern const char *XRAY_LAUNCHER_VERSION;
extern const char *XRAY_LAUNCHER_BUILD_TIME;

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::AboutDialog) {
  ui->setupUi(this);
  ui->labelTitle->setText(tr("xray启动器 %1").arg(XRAY_LAUNCHER_VERSION));
  ui->labelBuild->setText(
      tr("本软件使用Qt %1 构建, 基于<a href=\"%2\">LGPLv3协议</a>.")
          .arg(QT_VERSION_STR, "https://www.gnu.org/licenses/lgpl.html"));
  ui->labelBuild->setOpenExternalLinks(true);
  ui->labelTime->setText(tr("构建于%1").arg(XRAY_LAUNCHER_BUILD_TIME));
  ui->buttonBox->button(QDialogButtonBox::StandardButton::Close)
      ->setText(tr("关闭"));
}

AboutDialog::~AboutDialog() { delete ui; }
