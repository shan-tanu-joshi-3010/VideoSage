#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>
#include <QSslSocket>
#include <QDebug>

#include "NetworkManager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    /*
     * 🔥 IMPORTANT
     * Set working directory to application directory.
     * This ensures relative paths like:
     *   whisper/whisper-cli.exe
     *   whisper/ggml-small.bin
     * work correctly both in Qt Creator and after deployment.
     */
    QDir::setCurrent(QCoreApplication::applicationDirPath());

    // Debug info (safe to keep)
    qDebug() << "App dir:" << QCoreApplication::applicationDirPath();
    qDebug() << "Working dir:" << QDir::currentPath();

    qDebug() << "SSL supported:" << QSslSocket::supportsSsl();
    qDebug() << "SSL build version:" << QSslSocket::sslLibraryBuildVersionString();

    // Backend
    NetworkManager networkManager;

    // QML engine
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("NetworkManager", &networkManager);

    const QUrl mainQml(QStringLiteral("qrc:/main.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated,
        &app, [mainQml](QObject *obj, const QUrl &objUrl) {
            if (!obj && objUrl == mainQml)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection
        );

    engine.load(mainQml);

    return app.exec();
}
