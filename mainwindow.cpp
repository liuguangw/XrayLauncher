#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "error_log_dialog.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#ifdef Q_OS_WIN
#include <QSettings>
#endif

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
  // 获取自启动的设置
  if (this->isAutoRunSetting()) {
    ui->autoRunCheckBox->setCheckState(Qt::CheckState::Checked);
  }
  connect(ui->autoRunCheckBox, &QCheckBox::stateChanged, this,
          &MainWindow::onAutoRunChanged);
  this->initSystemTray();
  this->loadLauncherConfig();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::runMain(bool silentMode) {
  if (silentMode) {
    // 安静模式不显示主界面,并且自动运行xray
    this->onRunBtnClicked();
  } else {
    this->show();
  }
}

void MainWindow::onSelectXrayPathBtnClicked() {
  QString filter;
#ifdef Q_OS_WIN
  filter = tr("xray程序 (xray.exe)");
#else
  filter = tr("xray程序 (xray)");
#endif
  QString xrayPath =
      QFileDialog::getOpenFileName(this, tr("选择xray可执行文件"), "", filter);
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
    this->showErrorMessage(tr("xray程序路径不能为空"));
    return;
  }
  QString dataPath = this->launcherConfig.dataPath;
  if (dataPath.isEmpty()) {
    this->showErrorMessage(tr("资源文件夹路径不能为空"));
    return;
  }
  // 检查资源文件夹
  QString geoFilePath;
  const char *checkDataFiles[] = {"geosite.dat", "geoip.dat"};
  for (int i = 0; i < 2; i++) {
    geoFilePath = dataPath + "/" + checkDataFiles[i];
    if (!QFile::exists(geoFilePath)) {
      this->showErrorMessage(
          tr("资源文件夹下未找到文件%1").arg(checkDataFiles[i]));
      return;
    }
  }
  QString configPath = this->launcherConfig.configPath;
  if (configPath.isEmpty()) {
    this->showErrorMessage(tr("配置文件夹路径不能为空"));
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
  this->xrayProcess->setProcessChannelMode(
      QProcess::ProcessChannelMode::MergedChannels);
  // this->xrayProcess->setStandardErrorFile("./error.log");
  // this->xrayProcess->setStandardOutputFile("./output.log");
  QStringList arguments;
  arguments << "-confdir" << configPath;
  connect(this->xrayProcess, &QProcess::finished, this,
          &MainWindow::onProcessFinished);
  this->xrayStartedTime = QDateTime::currentDateTimeUtc();
  this->xrayProcess->start(xrayPath, arguments);
  this->changeRunningState(true);
}

void MainWindow::onStopBtnClicked() { this->killXrayProcess(); }

void MainWindow::onProcessFinished(int exitCode,
                                   QProcess::ExitStatus exitStatus) {
  disconnect(this->xrayProcess, &QProcess::finished, this,
             &MainWindow::onProcessFinished);
  QString errorMessage;
  QDateTime finishedTime = QDateTime::currentDateTimeUtc();
  // 5s内进程结束了，显示错误信息
  if (this->xrayStartedTime.secsTo(finishedTime) <= 5) {
    QByteArray errorData = this->xrayProcess->readAllStandardOutput();
    errorMessage = QString::fromUtf8(errorData);
    // qDebug()<<errorMessage;
  }
  delete this->xrayProcess;
  this->xrayProcess = nullptr;
  this->changeRunningState(false);
  if (exitStatus != QProcess::ExitStatus::NormalExit) {
    qDebug() << "xray CrashExit";
  }
  qDebug() << "xray finished with code" << exitCode;
  if (!errorMessage.isNull()) {
    this->showErrorLogDialog(errorMessage);
  }
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

void MainWindow::onAutoRunChanged(int state) {
  // qDebug() << "auto run changed" << state;
  if ((int)Qt::CheckState::Checked == state) {
    this->setAutoRun(true);
  } else {
    this->setAutoRun(false);
  }
}

void MainWindow::showErrorMessage(const QString &text) {
  QMessageBox::critical(this, tr("出错了"), text);
}

void MainWindow::showErrorLogDialog(const QString &text) {
  ErrorLogDialog dialog(this);
  dialog.setLogMessage(text);
  dialog.exec();
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

bool MainWindow::isAutoRunSetting() {
#ifdef Q_OS_WIN
  QSettings settings(REG_RUN_PATH, QSettings::NativeFormat);
  return settings.contains(REG_RUN_KEY);
#endif
  return false;
}

void MainWindow::setAutoRun(bool isAutoRun) {
#ifdef Q_OS_WIN
  QSettings settings(REG_RUN_PATH, QSettings::NativeFormat);
  if (isAutoRun) {
    // 设置开机自启动
    QString exePath =
        QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    QString regValue = QString("\"%1\" --silent").arg(exePath);
    settings.setValue(REG_RUN_KEY, regValue);
  } else if (settings.contains(REG_RUN_KEY)) {
    // 取消开机自启动
    settings.remove(REG_RUN_KEY);
  }
#else
  this->showErrorMessage(tr("当前系统还不支持此操作"));
#endif
}

void MainWindow::closeEvent(QCloseEvent *event) {
  // 关闭和隐藏都保存配置
  this->saveLauncherConfig();
  if (this->isSystemTrayClose) {
    // 从托盘区关闭时,先kill xray线程
    this->killXrayProcess();
    event->accept();
    // qDebug() << "close MainWindow";
  } else {
    if (!this->isHidden()) {
      this->hide();
    }
    event->ignore();
  }
}
