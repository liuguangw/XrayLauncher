#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "./launcher_config.h"
#include <QCloseEvent>
#include <QDateTime>
#include <QMainWindow>
#include <QProcess>
#include <QSystemTrayIcon>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
  void runMain(bool silentMode);
private slots:

  // 路径的选择
  void onSelectXrayPathBtnClicked();
  void onSelectDataPathBtnClicked();
  void onSelectConfigPathBtnClicked();
  void onXrayPathChanged(const QString &text);
  void onDataPathChanged(const QString &text);
  void onConfigPathChanged(const QString &text);
  // 启动
  void onRunBtnClicked();
  // 停止
  void onStopBtnClicked();
  // xray进程结束的处理
  void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
  // 显示主窗口
  void onShowMainWindow();
  // 托盘区
  void onExitApp();
  void
  onSystemTrayActivated(QSystemTrayIcon::ActivationReason activationReason);

private:
  Ui::MainWindow *ui;
  QString launcherConfigPath;
  LauncherConfig launcherConfig;
  QSystemTrayIcon *systemTrayIcon;
  QProcess *xrayProcess = nullptr;
  QDateTime xrayStartedTime;
  // 是否为托盘区域调用关闭
  bool isSystemTrayClose = false;
  void showErrorMessage(const QString &text);
  void showErrorLogDialog(const QString &text);
  void loadLauncherConfig();
  void saveLauncherConfig();
  // 初始化托盘
  void initSystemTray();
  void killXrayProcess();
  // 改变运行显示状态
  void changeRunningState(bool isRunning);

protected:
  void closeEvent(QCloseEvent *event) override;
};
#endif // MAINWINDOW_H
