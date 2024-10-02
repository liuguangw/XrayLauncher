#include "launcher_config.h"
#include <QFile>
#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>

LauncherConfig::LauncherConfig() {}

bool LauncherConfig::loadFromDisk(const QString &path, QString& error)
{
    if(!QFile::exists(path)){
        return true;
    }
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)){
        error.append(QObject::tr("打开文件%1失败").arg(path));
        return false;
    }
    QByteArray fileData = file.readAll();
    QJsonDocument document = QJsonDocument::fromJson(fileData);
    if(document.isNull()||!document.isObject()){
        error.append(QObject::tr("文件%1中的json格式无效").arg(path));
        return false;
    }
    QJsonObject json = document.object();
    //解析
    const QJsonValue xrayPathJson = json["xray_path"];
    if(xrayPathJson.isString()){
        this->xrayPath = xrayPathJson.toString();
    }
    const QJsonValue dataPathJson = json["data_path"];
    if(dataPathJson.isString()){
        this->dataPath = dataPathJson.toString();
    }
    const QJsonValue configPathJson = json["config_path"];
    if(configPathJson.isString()){
        this->configPath = configPathJson.toString();
    }
    return true;
}

bool LauncherConfig::saveToDisk(const QString &path, QString& error)
{
    QFile file(path);
    if(!file.open(QIODevice::Truncate|QIODevice::WriteOnly)){
        error.append(QObject::tr("只写模式打开文件%1失败").arg(path));
        return false;
    }
    QJsonObject json;
    json["xray_path"] = this->xrayPath;
    json["data_path"]=this->dataPath;
    json["config_path"]=this->configPath;
    if(file.write(QJsonDocument(json).toJson())<0){
        error.append(QObject::tr("写入文件%1失败").arg(path));
        return false;
    }
    return true;
}
