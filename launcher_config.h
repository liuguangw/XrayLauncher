#ifndef LAUNCHERCONFIG_H
#define LAUNCHERCONFIG_H
#include <QString>

class LauncherConfig {
public:
  QString xrayPath = "";
  QString dataPath = "";
  QString configPath = "";

  LauncherConfig();
  bool loadFromDisk(const QString &path, QString& error);
  bool saveToDisk(const QString &path, QString& error);
};

#endif // LAUNCHERCONFIG_H
