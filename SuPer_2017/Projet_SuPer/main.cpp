#include <QApplication>
#include <QLabel>
#include <QTextCodec>
#include <QDebug>
#include "ihm.h"
#include "server.h"
#include <QSettings>

int main(int argc, char *argv[])
{
    qDebug();
    /****RÉCUPERE IP ET PORT A PARTIR DUN FICHIER*************/
    QSettings MySetting("SuPer.ini", QSettings::IniFormat);
    QString ip = MySetting.value("Adresse/adresse", "192.168.1.47").toString();
    QString port = MySetting.value("Adresse/port", "2223").toString();
    /*********************************************************/
//    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
//    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    qDebug() << QThread::currentThreadId() << Q_FUNC_INFO << "QApplication a(argc, argv);";
    QApplication a(argc, argv);

    qDebug() << QThread::currentThreadId() << Q_FUNC_INFO << "Server server(\"192.168.1.47\", \"2223\");";
//    Server server("192.168.70.70", "2222");
    Server server(ip, port);

    Ihm ihm(&server);

    ihm.show();

    return a.exec();
}

