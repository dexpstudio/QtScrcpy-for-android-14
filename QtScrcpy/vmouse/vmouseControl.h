#ifndef VMOUSECONTROL_H
#define VMOUSECONTROL_H

#include <QThread>
#include <QProcess>
#include <QPointer>
#include <QVector>
#include <QTcpServer>

class QIODevice;
class VMouseControl : public QObject
{
    Q_OBJECT
public:
    explicit VMouseControl(QObject *parent = nullptr);
    ~VMouseControl();

    bool start(const QString& serial, int port);
    void stop();
    void installonly(const QString& serial, int port);
    void sendMsg(char content);
    QTcpSocket *getVMouseSocket();

private:
    bool runVMouseProcess(const QString& serial, int port, bool wait = true);
    void startVMouse(int port);
    void stopVMouse();

signals:
    void connectTo(int port);
    void sendCtrMsg(char msg);

private:
    QThread m_workerThread;
    QProcess m_vmouse;
    QVector<char> m_buffer;
    bool m_running = false;
};

#endif // VMOUSECONTROL_H