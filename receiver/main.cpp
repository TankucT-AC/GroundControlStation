#include "MainWindow.h"
#include "Theme.h"
#include "UdpWorker.h"
#include <QApplication>
#include <QPalette>
#include <QThread>

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

    MainWindow window;
    window.show();

    quint16 port = 12345;

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
    workerThread->wait();
    delete workerThread;

    return ret;
}
