#include <QTcpSocket>
#include <QHostAddress>
#include <QAudioOutput>
#include <QTime>
#include <QElapsedTimer>

#include "vmouseControl.h"

VMouseControl::VMouseControl(QObject *parent)
    : QObject(parent)
{
    connect(&m_vmouse, &QProcess::readyReadStandardOutput, this, [this]() {
        qInfo() << QString("VMouseOutput::") << QString(m_vmouse.readAllStandardOutput());
    });
    connect(&m_vmouse, &QProcess::readyReadStandardError, this, [this]() {
        qInfo() << QString("VMouseOutput::") << QString(m_vmouse.readAllStandardError());
    });
}

VMouseControl::~VMouseControl()
{
    if (QProcess::NotRunning != m_vmouse.state()) {
        m_vmouse.kill();
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
    qInfo() << "VMouseOutput::run vmouse cost:" << timeConsumeCount.elapsed() << "milliseconds";
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
    if (QProcess::NotRunning != m_vmouse.state()) {
        m_vmouse.kill();
    }

#ifdef Q_OS_WIN32
    QStringList params;
    params << serial;
    params << QString("%1").arg(port);
    m_vmouse.start("vmouse.bat", params);
#else
    QStringList params;
    params << "vmouse.sh";
    params << serial;
    params << QString("%1").arg(port);
    m_vmouse.start("bash", params);
#endif

    if (!wait) {
        return true;
    }

    if (!m_vmouse.waitForStarted()) {
        qWarning() << "VMouseOutput::start vmouse.bat failed";
        return false;
    }
    if (!m_vmouse.waitForFinished()) {
        qWarning() << "VMouseOutput::vmouse.bat crashed";
        return false;
    }

    return true;
}

void VMouseControl::startVMouse(int port)
{
    if (m_workerThread.isRunning()) {
        stopVMouse();
    }

    auto vmouseCtrSocket = new QTcpSocket();
    vmouseCtrSocket->moveToThread(&m_workerThread);
    connect(&m_workerThread, &QThread::finished, vmouseCtrSocket, &QObject::deleteLater);

    connect(this, &VMouseControl::connectTo, vmouseCtrSocket, [vmouseCtrSocket](int port) {
        vmouseCtrSocket->connectToHost(QHostAddress::LocalHost, port);
        if (!vmouseCtrSocket->waitForConnected(500)) {
            qWarning("VMouseOutput::vmouseCtr socket connect failed");
            return;
        }
        qInfo("VMouseOutput::vmouseCtr socket connect success");
    });
    connect(vmouseCtrSocket, &QIODevice::readyRead, vmouseCtrSocket, [this, vmouseCtrSocket]() {
        qint64 recv = vmouseCtrSocket->bytesAvailable();
        qDebug() << "VMouseOutput::recv data:" << recv;

        if (m_buffer.capacity() < recv) {
            m_buffer.reserve(recv);
        }

        qint64 count = vmouseCtrSocket->read(m_buffer.data(), vmouseCtrSocket->bytesAvailable());
        //m_outputDevice->write(m_buffer.data(), count);
    });
    connect(this, &VMouseControl::sendCtrMsg, vmouseCtrSocket, [vmouseCtrSocket](char msg) {
        vmouseCtrSocket->write(&msg);
        qInfo("VMouseOutput::send Msg",msg);
    });
    connect(vmouseCtrSocket, &QTcpSocket::stateChanged, vmouseCtrSocket, [](QAbstractSocket::SocketState state) {
        qInfo() << "VMouseOutput::vmouseCtr socket state changed:" << state;

    });
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(vmouseCtrSocket, &QTcpSocket::errorOccurred, vmouseCtrSocket, [](QAbstractSocket::SocketError error) {
        qInfo() << "VMouseOutput::audio socket error occurred:" << error;
    });
#else
    connect(vmouseCtrSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), vmouseCtrSocket, [](QAbstractSocket::SocketError error) {
        qInfo() << "VMouseOutput::vmouseCtr socket error occurred:" << error;
    });
#endif

    m_workerThread.start();
    emit connectTo(port);
}

void VMouseControl::stopVMouse()
{
    if (!m_workerThread.isRunning()) {
        return;
    }

    m_workerThread.quit();
    m_workerThread.wait();
}

void VMouseControl::sendMsg(char content)
{
    if (!m_workerThread.isRunning()) {
        return;
    }
    emit sendCtrMsg(content);
}
