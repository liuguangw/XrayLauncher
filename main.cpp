#include "mainwindow.h"
#include "version.h"
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setApplicationVersion(XRAY_LAUNCHER_VERSION);
  // 安静模式,用于开机自启动的参数-s/--silent
  bool silentMode = false;
  if (argc > 1) {
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QApplication::tr("XrayLauncher helper(build at %1)")
            .arg(XRAY_LAUNCHER_BUILD_TIME));
    parser.addHelpOption();
    parser.addVersionOption();
    QStringList optionNames = QStringList() << "s"
                                            << "silent";
    QCommandLineOption silentOption(optionNames,
                                    QApplication::tr("以安静模式启动"));
    parser.addOption(silentOption);
    parser.process(app);
    silentMode = parser.isSet(silentOption);
  }
  MainWindow w;
  w.runMain(silentMode);
  return app.exec();
}
