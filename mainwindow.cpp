#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  connect(ui->selectXrayPathBtn, &QPushButton::clicked, this,
          &MainWindow::onSelectXrayPathBtnClicked);
  connect(ui->selectDataPathBtn, &QPushButton::clicked, this,
          &MainWindow::onSelectDataPathBtnClicked);
  connect(ui->selectConfigPathBtn, &QPushButton::clicked, this,
          &MainWindow::onSelectConfigPathBtnClicked);
  connect(ui->xrayPathEdit, &QLineEdit::textChanged, this,
          &MainWindow::onXrayPathChanged);
  connect(ui->dataPathEdit, &QLineEdit::textChanged, this,
          &MainWindow::onDataPathChanged);
  connect(ui->configPathEdit, &QLineEdit::textChanged, this,
          &MainWindow::onConfigPathChanged);
  connect(ui->runBtn, &QPushButton::clicked, this,
          &MainWindow::onRunBtnClicked);
  connect(ui->stopBtn, &QPushButton::clicked, this,
          &MainWindow::onStopBtnClicked);
  this->initSystemTray();
  this->loadLauncherConfig();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::onSelectXrayPathBtnClicked() {
  QString xrayPath = QFileDialog::getOpenFileName(
      this, tr("选择xray可执行文件"), "", tr("xray程序 (xray.exe)"));
  if (!xrayPath.isNull()) {
    ui->xrayPathEdit->setText(QDir::toNativeSeparators(xrayPath));
  }
}

void MainWindow::onSelectDataPathBtnClicked() {
  QString dataPath = QFileDialog::getExistingDirectory(
      this, tr("选择资源文件夹"), "",
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (!dataPath.isNull()) {
    ui->dataPathEdit->setText(QDir::toNativeSeparators(dataPath));
  }
}

void MainWindow::onSelectConfigPathBtnClicked() {
  QString configPath = QFileDialog::getExistingDirectory(
      this, tr("选择配置文件夹"), "",
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (!configPath.isNull()) {
    ui->configPathEdit->setText(QDir::toNativeSeparators(configPath));
  }
}

void MainWindow::onXrayPathChanged(const QString &text) {
  this->launcherConfig.xrayPath = text;
}

void MainWindow::onDataPathChanged(const QString &text) {
  this->launcherConfig.dataPath = text;
}

void MainWindow::onConfigPathChanged(const QString &text) {
  this->launcherConfig.configPath = text;
}

void MainWindow::onRunBtnClicked() {
  QString xrayPath = this->launcherConfig.xrayPath;
  if (xrayPath.isEmpty()) {
    showErrorMessage(tr("xray程序路径不能为空"));
    return;
  }
  QString dataPath = this->launcherConfig.dataPath;
  if (dataPath.isEmpty()) {
    showErrorMessage(tr("资源文件夹路径不能为空"));
    return;
  }
  // 检查资源文件夹
  QString geoFilePath;
  const char *checkDataFiles[] = {"geosite.dat", "geoip.dat"};
  for (int i = 0; i < 2; i++) {
    geoFilePath = dataPath + "/" + checkDataFiles[i];
    if (!QFile::exists(geoFilePath)) {
      showErrorMessage(tr("资源文件夹下未找到文件%1").arg(checkDataFiles[i]));
      return;
    }
  }
  QString configPath = this->launcherConfig.configPath;
  if (configPath.isEmpty()) {
    showErrorMessage(tr("配置文件夹路径不能为空"));
    return;
  }
  // 工作目录
  QDir workingDir(dataPath);
  workingDir.cdUp();
  // env
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("XRAY_LOCATION_ASSET", dataPath);
  //
  this->xrayProcess = new QProcess(this);
  this->xrayProcess->setWorkingDirectory(workingDir.path());
  this->xrayProcess->setProcessEnvironment(env);
  // this->xrayProcess->setStandardErrorFile("./error.log");
  // this->xrayProcess->setStandardOutputFile("./output.log");
  QStringList arguments;
  arguments << "-confdir" << configPath;
  this->xrayProcess->start(xrayPath, arguments);
  this->changeRunningState(true);
  connect(this->xrayProcess, &QProcess::finished, this,
          &MainWindow::onProcessFinished);
}

void MainWindow::onStopBtnClicked() { this->killXrayProcess(); }

void MainWindow::onProcessFinished(int exitCode,
                                   QProcess::ExitStatus exitStatus) {
  disconnect(this->xrayProcess, &QProcess::finished, this,
             &MainWindow::onProcessFinished);
  delete this->xrayProcess;
  this->xrayProcess = nullptr;
  this->changeRunningState(false);
  if (exitStatus != QProcess::ExitStatus::NormalExit) {
    qDebug() << "xray CrashExit";
  }
  qDebug() << "xray finished with code" << exitCode;
}

void MainWindow::onShowMainWindow() {
  if (this->isHidden()) {
    this->show();
  } else if (this->isMinimized()) {
    this->showNormal();
  } else if (!this->isActiveWindow()) {
    this->activateWindow();
  }
}

void MainWindow::onExitApp() {
  this->isSystemTrayClose = true;
  // 隐藏状态下不能关闭
  if (this->isHidden()) {
    this->show();
  }
  this->close();
}

void MainWindow::onSystemTrayActivated(
    QSystemTrayIcon::ActivationReason activationReason) {
  // 除了右键点击托盘图标
  if (activationReason != QSystemTrayIcon::ActivationReason::Context) {
    this->onShowMainWindow();
  }
}

void MainWindow::showErrorMessage(const QString &text) {
  QMessageBox::critical(this, tr("出错了"), text);
}

void MainWindow::loadLauncherConfig() {
  this->launcherConfigPath =
      QCoreApplication::applicationDirPath() + "/launcher_config.json";
  QString launcherConfigError;
  LauncherConfig config;
  if (!config.loadFromDisk(this->launcherConfigPath, launcherConfigError)) {
    qDebug() << "读取配置文件失败" << launcherConfigError;
    return;
  }
  ui->xrayPathEdit->setText(config.xrayPath);
  ui->dataPathEdit->setText(config.dataPath);
  ui->configPathEdit->setText(config.configPath);
}

void MainWindow::saveLauncherConfig() {
  QString launcherConfigError;
  if (!this->launcherConfig.saveToDisk(this->launcherConfigPath,
                                       launcherConfigError)) {
    qDebug() << "保存配置文件失败" << launcherConfigError;
  }
}

void MainWindow::initSystemTray() {
  this->systemTrayIcon = new QSystemTrayIcon(this);
  this->systemTrayIcon->setIcon(QIcon(":/icons/app_icon.png"));
  this->systemTrayIcon->setToolTip(tr("xray启动器"));
  //
  QMenu *menu = new QMenu(this);
  QAction *showMainAction = new QAction("打开主窗口", this);
  menu->addAction(showMainAction);
  QAction *exitAction = new QAction("退出", this);
  menu->addSeparator();
  menu->addAction(exitAction);
  //
  this->systemTrayIcon->setContextMenu(menu);
  connect(this->systemTrayIcon, &QSystemTrayIcon::activated, this,
          &MainWindow::onSystemTrayActivated);
  connect(showMainAction, &QAction::triggered, this,
          &MainWindow::onShowMainWindow);
  connect(exitAction, &QAction::triggered, this, &MainWindow::onExitApp);
  this->systemTrayIcon->show();
}

void MainWindow::killXrayProcess() {
  if (this->xrayProcess != nullptr) {
    QProcess::ProcessState state = this->xrayProcess->state();
    if (state == QProcess::ProcessState::Running) {
#ifdef Q_OS_WIN
      // windows命令行程序不会处理 WM_CLOSE 消息
      this->xrayProcess->kill();
#else
      this->xrayProcess->terminate();
#endif
      this->xrayProcess->waitForFinished();
    }
  }
}

void MainWindow::changeRunningState(bool isRunning) {
  ui->xrayPathEdit->setEnabled(!isRunning);
  ui->selectXrayPathBtn->setEnabled(!isRunning);
  ui->dataPathEdit->setEnabled(!isRunning);
  ui->selectDataPathBtn->setEnabled(!isRunning);
  ui->configPathEdit->setEnabled(!isRunning);
  ui->selectConfigPathBtn->setEnabled(!isRunning);
  ui->runBtn->setEnabled(!isRunning);
  ui->stopBtn->setEnabled(isRunning);
}

void MainWindow::closeEvent(QCloseEvent *event) {
  // 关闭和隐藏都保存配置
  this->saveLauncherConfig();
  event->accept();
  if (this->isSystemTrayClose) {
    // 从托盘区关闭时,先kill xray线程
    this->killXrayProcess();
    event->accept();
    qDebug() << "close MainWindow";
  } else {
    if (!this->isHidden()) {
      this->hide();
    }
    event->ignore();
  }
}
