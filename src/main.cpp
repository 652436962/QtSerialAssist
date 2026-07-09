#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QTranslator>
#include <QDir>
#include <QQuickStyle>
#include <QtQml>
#include "backend/AppCore.h"

int main(int argc, char *argv[])
{
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QQuickStyle::setStyle("Fusion");
    QGuiApplication app(argc, argv);
    app.setApplicationName("QtSerialAssist");
    app.setOrganizationName("QtSerialAssist");
    app.setApplicationVersion("2.0.1");

    QIcon icon;
    icon.addFile(":/icons/icon_16x16.png", QSize(16, 16));
    icon.addFile(":/icons/icon_32x32.png", QSize(32, 32));
    icon.addFile(":/icons/icon_48x48.png", QSize(48, 48));
    icon.addFile(":/icons/icon_64x64.png", QSize(64, 64));
    app.setWindowIcon(icon);

    QTranslator translator;
    QStringList searchPaths = {
        QCoreApplication::applicationDirPath() + "/QtSerialAssist_zh_CN.qm",
        QCoreApplication::applicationDirPath() + "/../QtSerialAssist_zh_CN.qm",
        ":/QtSerialAssist_zh_CN.qm"
    };
    for (const auto &path : searchPaths) {
        if (translator.load(path)) {
            app.installTranslator(&translator);
            break;
        }
    }

    AppCore core(&app);

    QQmlApplicationEngine engine;

    qmlRegisterSingletonType(QUrl(QStringLiteral("qrc:/qml/theme/DeepinTheme.qml")),
                             "theme", 1, 0, "DeepinTheme");

    engine.rootContext()->setContextProperty("core", &core);

    const QUrl qmlUrl(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.load(qmlUrl);

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
