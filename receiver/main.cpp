#include "MainWindow.h"
#include "UdpWorker.h"
#include <QApplication>
#include <QThread>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  QApplication::setApplicationName("UAV Telemetry Receiver");
  QApplication::setApplicationVersion("1.0.0");

  MainWindow window;
  window.show();

  quint16 port = 12345;

  auto *workerThread = new QThread;
  auto *worker = new UdpWorker(port);
  worker->moveToThread(workerThread);

  QObject::connect(workerThread, &QThread::started, worker, &UdpWorker::start);
  QObject::connect(workerThread, &QThread::finished, worker,
                   &QObject::deleteLater);
  QObject::connect(worker, &UdpWorker::dataReceived, &window,
                   &MainWindow::onTelemetryReceived);

  workerThread->start();

  int ret = app.exec();

  workerThread->quit();
  workerThread->wait();
  delete workerThread;

  return ret;
}
