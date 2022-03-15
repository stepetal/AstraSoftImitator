#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QTimer>
#include <QPushButton>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QHash>
#include <QFileInfo>
#include <QFileInfoList>
#include <QTextStream>
#include <QVector>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QStatusBar>
#include <QTimer>
#include <QEventLoop>

enum class channels {CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8};

/* возвращает приведенное к родному типу значение класса enum'a */
template<typename E>
constexpr std::underlying_type_t<E>
toUType(E enumerator) noexcept {
    return static_cast<std::underlying_type_t<E>>(enumerator);
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QTimer *specterTransmittionTimer;
    QLineEdit *ipAddressLineEdit;
    QLineEdit *dataPortLineEdit;
    QLineEdit *commandPortLineEdit;
    QLineEdit *spectrDirectoryLineEdit;
    QSpinBox *specterFreqSpinBox;
    QTimer *specterTimer;
    QHash<double,QVector<double>> waveLengthChDataMap;
    QVector<double> waveLengthData;
    QUdpSocket *commandUdpSocket;
    QUdpSocket *dataUdpSocket;
    int data_port;
    int command_port;
    //Всего существует 8 каналов, поэтому подход по их жесткому заданию оправдан
    //данные вектора нужны для интерполяции
    QVector<double> ch1Data;
    QByteArray interpolatedCh1Data;
    QByteArray interpolatedCh2Data;
    QByteArray interpolatedCh3Data;
    QByteArray interpolatedCh4Data;
    QByteArray interpolatedCh5Data;
    QByteArray interpolatedCh6Data;
    QByteArray interpolatedCh7Data;
    QByteArray interpolatedCh8Data;
    QVector<double> ch2Data;
    QVector<double> ch3Data;
    QVector<double> ch4Data;
    QVector<double> ch5Data;
    QVector<double> ch6Data;
    QVector<double> ch7Data;
    QVector<double> ch8Data;
private:
    void readSpectrFiles();
    //кусочно-линейная интерполяция (https://habr.com/ru/post/157407/).
    //x - значение длины волны, chn - номер канала. Ф-ция возвращает значение мощности, соответствующее x.
    double partiallyLinearInterpolation(double x, channels chn);
    void createInterpolatedChannelValues(channels chn);
    void initSockets();
    void parseCommand(const QNetworkDatagram &datagram);
    void readCommand();
    void sendDataToServer(QByteArray &&payload);
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void createActions();
    void createLayout();
    void createConnections();
    void createWidgets();

public slots:
    void handleSocketError(QAbstractSocket::SocketError e);


};
#endif // MAINWINDOW_H
