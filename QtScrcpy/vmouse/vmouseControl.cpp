#include <QTcpSocket>
#include <QHostAddress>
#include <QAudioOutput>
#include <QTime>
#include <QElapsedTimer>

#include "vmouseControl.h"

VMouseControl::VMouseControl(QObject *parent)
    : QObject(parent)
{
    connect(&m_sndcpy, &QProcess::readyReadStandardOutput, this, [this]() {
        qInfo() << QString("AudioOutput::") << QString(m_sndcpy.readAllStandardOutput());
    });
    connect(&m_sndcpy, &QProcess::readyReadStandardError, this, [this]() {
        qInfo() << QString("AudioOutput::") << QString(m_sndcpy.readAllStandardError());
    });
}

VMouseControl::~VMouseControl()
{
    if (QProcess::NotRunning != m_sndcpy.state()) {
        m_sndcpy.kill();
    }
    stop();
}

bool VMouseControl::start(const QString& serial, int port)
{
    if (m_running) {
        stop();
    }

    QElapsedTimer timeConsumeCount;
    timeConsumeCount.start();
    bool ret = runVMouseProcess(serial, port);
    qInfo() << "AudioOutput::run sndcpy cost:" << timeConsumeCount.elapsed() << "milliseconds";
    if (!ret) {
        return ret;
    }

    startVMouse(port);

    m_running = true;
    return true;
}

void VMouseControl::stop()
{
    if (!m_running) {
        return;
    }
    m_running = false;

    stopVMouse();
}

void VMouseControl::installonly(const QString &serial, int port)
{
    runVMouseProcess(serial, port, false);
}

bool VMouseControl::runVMouseProcess(const QString &serial, int port, bool wait)
{
    if (QProcess::NotRunning != m_sndcpy.state()) {
        m_sndcpy.kill();
    }

#ifdef Q_OS_WIN32
    QStringList params;
    params << serial;
    params << QString("%1").arg(port);
    m_sndcpy.start("sndcpy.bat", params);
#else
    QStringList params;
    params << "sndcpy.sh";
    params << serial;
    params << QString("%1").arg(port);
    m_sndcpy.start("bash", params);
#endif

    if (!wait) {
        return true;
    }

    if (!m_sndcpy.waitForStarted()) {
        qWarning() << "AudioOutput::start sndcpy.bat failed";
        return false;
    }
    if (!m_sndcpy.waitForFinished()) {
        qWarning() << "AudioOutput::sndcpy.bat crashed";
        return false;
    }

    return true;
}

void VMouseControl::startVMouse(int port)
{
//    if (m_workerThread.isRunning()) {
//        stopRecvData();
//    }

//    auto audioSocket = new QTcpSocket();
//    audioSocket->moveToThread(&m_workerThread);
//    connect(&m_workerThread, &QThread::finished, audioSocket, &QObject::deleteLater);

//    connect(this, &AudioOutput::connectTo, audioSocket, [audioSocket](int port) {
//        audioSocket->connectToHost(QHostAddress::LocalHost, port);
//        if (!audioSocket->waitForConnected(500)) {
//            qWarning("AudioOutput::audio socket connect failed");
//            return;
//        }
//        qInfo("AudioOutput::audio socket connect success");
//    });
//    connect(audioSocket, &QIODevice::readyRead, audioSocket, [this, audioSocket]() {
//        qint64 recv = audioSocket->bytesAvailable();
//        //qDebug() << "AudioOutput::recv data:" << recv;

//        if (!m_outputDevice) {
//            return;
//        }
//        if (m_buffer.capacity() < recv) {
//            m_buffer.reserve(recv);
//        }

//        qint64 count = audioSocket->read(m_buffer.data(), audioSocket->bytesAvailable());
//        m_outputDevice->write(m_buffer.data(), count);
//    });
//    connect(audioSocket, &QTcpSocket::stateChanged, audioSocket, [](QAbstractSocket::SocketState state) {
//        qInfo() << "AudioOutput::audio socket state changed:" << state;

//    });
//#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
//    connect(audioSocket, &QTcpSocket::errorOccurred, audioSocket, [](QAbstractSocket::SocketError error) {
//        qInfo() << "AudioOutput::audio socket error occurred:" << error;
//    });
//#else
//    connect(audioSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), audioSocket, [](QAbstractSocket::SocketError error) {
//        qInfo() << "AudioOutput::audio socket error occurred:" << error;
//    });
//#endif

//    m_workerThread.start();
//    emit connectTo(port);
}

void VMouseControl::stopVMouse()
{
//    if (!m_workerThread.isRunning()) {
//        return;
//    }

//    m_workerThread.quit();
//    m_workerThread.wait();
}
