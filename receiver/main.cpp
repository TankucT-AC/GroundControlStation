#include "MainWindow.h"
#include "Theme.h"
#include "UdpWorker.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QPalette>
#include <QThread>
#include <chrono>
#include <cstdio>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("UAV Telemetry Receiver");
    QApplication::setApplicationVersion("1.0.0");

    // Тёмная тема на базе Fusion: единая палитра из Theme.h
    QApplication::setStyle("Fusion");
    QPalette dark;
    dark.setColor(QPalette::Window, Theme::window);
    dark.setColor(QPalette::WindowText, Theme::textPrimary);
    dark.setColor(QPalette::Base, Theme::gaugeFace);
    dark.setColor(QPalette::Text, Theme::textPrimary);
    dark.setColor(QPalette::Button, Theme::surface);
    dark.setColor(QPalette::ButtonText, Theme::textPrimary);
    dark.setColor(QPalette::Highlight, Theme::ok);
    dark.setColor(QPalette::HighlightedText, Theme::textPrimary);
    dark.setColor(QPalette::ToolTipBase, Theme::surface);
    dark.setColor(QPalette::ToolTipText, Theme::textPrimary);
    dark.setColor(QPalette::Disabled, QPalette::Text, Theme::textDim);
    dark.setColor(QPalette::Disabled, QPalette::ButtonText, Theme::textDim);
    app.setPalette(dark);

    QCommandLineParser parser;
    parser.setApplicationDescription("Receives binary UAV telemetry over UDP and shows the dashboard");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption portOpt({"p", "port"}, "UDP port to listen on", "port", "12345");
    parser.addOption(portOpt);
    parser.process(app);

    bool portOk = false;
    const quint16 port = parser.value(portOpt).toUShort(&portOk);
    if (!portOk || port == 0) {
        fprintf(stderr, "Invalid port '%s'\n", qPrintable(parser.value(portOpt)));
        return 1;
    }

    MainWindow window;
    window.show();

    auto *workerThread = new QThread;
    auto *worker = new UdpWorker(port);
    worker->moveToThread(workerThread);

    QObject::connect(workerThread, &QThread::started, worker, &UdpWorker::start);
    QObject::connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);
    QObject::connect(worker, &UdpWorker::dataReceived, &window, &MainWindow::onTelemetryReceived);
    QObject::connect(worker, &UdpWorker::errorOccurred, &window, &MainWindow::onLinkError);
    QObject::connect(worker, &UdpWorker::errorCountChanged, &window, &MainWindow::onErrorCountChanged);

    workerThread->start();

    int ret = app.exec();

    workerThread->quit();
    if (!workerThread->wait(std::chrono::seconds(2)))
        qWarning("Worker thread did not finish in time");
    delete workerThread;

    return ret;
}
