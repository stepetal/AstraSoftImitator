#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      specterTransmittionTimer(new QTimer())
{
    createWidgets();
    initSockets();
    createLayout();
    createConnections();

}

MainWindow::~MainWindow()
{
}

void MainWindow::initSockets()
{
    commandUdpSocket = new QUdpSocket(this);
    dataUdpSocket = new QUdpSocket(this);
    if ((ipAddressLineEdit->text() != ""))
    {
        QHostAddress hostAddress(ipAddressLineEdit->text());
        command_port = commandPortLineEdit->text().toInt();
        data_port = dataPortLineEdit->text().toInt();
        commandUdpSocket->bind(hostAddress,command_port);
        dataUdpSocket->bind(hostAddress,data_port);
    }
}

void MainWindow::parseCommand(const QNetworkDatagram &datagram)
{
    if(datagram.data() == QString("ACQU:OSAT:CONT:STAR").toUtf8())
    {
        readSpectrFiles();
        qDebug() << "Now send data to server with certain frequency";
        {
            sendDataToServer(std::move(interpolatedCh1Data));
            QEventLoop loop;
            QTimer::singleShot(specterFreqSpinBox->value(), &loop, &QEventLoop::quit);
            loop.exec();
        }
        {
            sendDataToServer(std::move(interpolatedCh2Data));
            QEventLoop loop;
            QTimer::singleShot(specterFreqSpinBox->value(), &loop, &QEventLoop::quit);
            loop.exec();
        }
        {
            sendDataToServer(std::move(interpolatedCh3Data));
            QEventLoop loop;
            QTimer::singleShot(specterFreqSpinBox->value(), &loop, &QEventLoop::quit);
            loop.exec();
        }
        {
            sendDataToServer(std::move(interpolatedCh4Data));
            QEventLoop loop;
            QTimer::singleShot(specterFreqSpinBox->value(), &loop, &QEventLoop::quit);
            loop.exec();
        }
        {
            sendDataToServer(std::move(interpolatedCh5Data));
            QEventLoop loop;
            QTimer::singleShot(specterFreqSpinBox->value(), &loop, &QEventLoop::quit);
            loop.exec();
        }
        {
            sendDataToServer(std::move(interpolatedCh6Data));
            QEventLoop loop;
            QTimer::singleShot(specterFreqSpinBox->value(), &loop, &QEventLoop::quit);
            loop.exec();
        }
        {
            sendDataToServer(std::move(interpolatedCh7Data));
            QEventLoop loop;
            QTimer::singleShot(specterFreqSpinBox->value(), &loop, &QEventLoop::quit);
            loop.exec();
        }
        {
            sendDataToServer(std::move(interpolatedCh8Data));
            QEventLoop loop;
            QTimer::singleShot(specterFreqSpinBox->value(), &loop, &QEventLoop::quit);
            loop.exec();
        }
    }
}

void MainWindow::readCommand()
{
    while(commandUdpSocket->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = commandUdpSocket->receiveDatagram();
        parseCommand(datagram);
    }
}

void MainWindow::sendDataToServer(QByteArray &&payload)
{
    const int max_len = 8192;
    QHostAddress serverAddress(ipAddressLineEdit->text());
    int portNumb = data_port;
    //если размер посылки больше, чем максимальное число байт,
    //делим датаграмму на части и отправляем по-очереди
    if(payload.size() > max_len)
    {
        qDebug() << "Number of packets is: " << payload.size() / max_len;
        for(auto i = 0; i <= payload.size() / max_len; ++i)
        {
            QNetworkDatagram datagram(payload.mid(i*max_len,max_len),serverAddress,portNumb);
            if(datagram.isValid())
            {
                qint64 bytesWritten = dataUdpSocket->writeDatagram(datagram);
                if ( bytesWritten != -1)
                {
                    QString bytesWrittenStr = QString::number(bytesWritten);
                    statusBar()->showMessage("Successfully written: " + bytesWrittenStr + ";" + "Server: " + datagram.destinationAddress().toString() +
                                                   ":" + QString::number(datagram.destinationPort()),2000);
                }
                else
                {
                    qDebug() << "No bytes written";
                }
            }
            else
            {
                qDebug() << "Datagram is invalid";
            }
        }
    }
}

void MainWindow::createWidgets()
{
    ipAddressLineEdit = new QLineEdit();
    dataPortLineEdit = new QLineEdit();
    commandPortLineEdit = new QLineEdit();
    ipAddressLineEdit->setText("127.0.0.1");
    dataPortLineEdit->setText("15682");
    commandPortLineEdit->setText("15680");
    specterFreqSpinBox = new QSpinBox();
    specterFreqSpinBox->setRange(0,5000);
    specterFreqSpinBox->setValue(500);
    spectrDirectoryLineEdit = new QLineEdit(QString("e:\\Projects\\Qt\\ForInversiaSensor\\"));
}

void MainWindow::handleSocketError(QAbstractSocket::SocketError e)
{
    qDebug() << "Socket error is: " << e;
}


void MainWindow::createLayout()
{
    QWidget *mainWindowWidget = new QWidget();
    QVBoxLayout *mainWindowWidgetLayout = new QVBoxLayout();
    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow(QString("Ip адрес"),ipAddressLineEdit);
    formLayout->addRow(QString("Порт данных"),commandPortLineEdit);
    formLayout->addRow(QString("Порт команд"),dataPortLineEdit);
    formLayout->addRow(QString("Период выдачи спектров, мс"),specterFreqSpinBox);
    formLayout->addRow(QString("Расположение спектров"),spectrDirectoryLineEdit);
    formWidget->setLayout(formLayout);
    mainWindowWidgetLayout->addWidget(formWidget,0,Qt::AlignCenter);
    mainWindowWidget->setLayout(mainWindowWidgetLayout);
    setCentralWidget(mainWindowWidget);
}

void MainWindow::createConnections()
{

    connect(commandUdpSocket,&QUdpSocket::readyRead,[&]()
    {
        readCommand();
    });

    connect(ipAddressLineEdit,&QLineEdit::editingFinished,[&]()
    {
        QHostAddress hostAddress(ipAddressLineEdit->text());
        command_port = commandPortLineEdit->text().toInt();
        data_port = dataPortLineEdit->text().toInt();
        commandUdpSocket->close();
        dataUdpSocket->close();
        commandUdpSocket->bind(hostAddress,command_port);
        dataUdpSocket->bind(hostAddress,data_port);
    });

    // TODO: разобраться, почему при максимальной длине посылки больше, чем 8192, не происходит переход в слот handleSocketError
    connect(dataUdpSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(handleSocketError(QAbstractSocket::SocketError)));

    connect(specterTransmittionTimer,&QTimer::timeout,[&]
    {

    });




}



void MainWindow::readSpectrFiles()
{
    QDir dir;
    dir.setFilter(QDir::Files);
    dir.cd(spectrDirectoryLineEdit->text());
    foreach(auto fileInfo,dir.entryInfoList())
    {
        if(fileInfo.suffix() == QStringLiteral("csv"))
        {
            QFile spectrFile(fileInfo.absoluteFilePath());
            if(spectrFile.open(QFile::ReadOnly | QFile::Text))
            {
                QTextStream spectrFileStream(&spectrFile);
                bool first_line{true};
                while(!spectrFileStream.atEnd())
                {
                    QString line = spectrFileStream.readLine();
                    if(first_line)
                    {
                        first_line = false;
                    }
                    else
                    {
                        QStringList lineData = line.split(";");
                        if(lineData.size() < 9)
                        {
                            auto start_size = lineData.size();
                            for(auto i = 0; i < 9 - start_size; ++i)
                            {
                                lineData.append("");
                            }
                        }
                        waveLengthData.append(lineData[0].toDouble());
                        ch1Data.append(lineData[1].toDouble());
                        ch2Data.append(lineData[2].toDouble());
                        ch3Data.append(lineData[3].toDouble());
                        ch4Data.append(lineData[4].toDouble());
                        ch5Data.append(lineData[5].toDouble());
                        ch6Data.append(lineData[6].toDouble());
                        ch7Data.append(lineData[7].toDouble());
                        ch8Data.append(lineData[8].toDouble());
                    }
                }
            }

        }
    }
    qDebug() << "Check the size of ...";
    qDebug() << ch1Data.size() << ch2Data.size() << ch3Data.size();
    createInterpolatedChannelValues(channels::CH1);
    createInterpolatedChannelValues(channels::CH2);
    createInterpolatedChannelValues(channels::CH3);
    createInterpolatedChannelValues(channels::CH4);
    createInterpolatedChannelValues(channels::CH5);
    createInterpolatedChannelValues(channels::CH6);
    createInterpolatedChannelValues(channels::CH7);
    createInterpolatedChannelValues(channels::CH8);
    qDebug() << "Interpolated data size is: " << interpolatedCh1Data.size()
                                              << interpolatedCh2Data.size()
                                              << interpolatedCh3Data.size()
                                              << interpolatedCh4Data.size()
                                              << interpolatedCh5Data.size()
                                              << interpolatedCh6Data.size()
                                              << interpolatedCh7Data.size()
                                              << interpolatedCh8Data.size();
    qDebug() << "Interpolation successfull";


}

double MainWindow::partiallyLinearInterpolation(double x,channels chn)
{
    QVector<double> k;
    QVector<double> b;
    switch (chn)
    {
        case channels::CH1:
        {
            if((waveLengthData.size() == ch1Data.size()) && ((!std::equal(ch1Data.begin() + 1,ch1Data.end(),ch1Data.begin())) && (ch1Data[0] == 0)))
            {
                int n = ch1Data.size();
                //расчет коэффициентов для ch1 - ch3
                for(int i = 0; i < n - 1; ++i)
                {
                    k.append((ch1Data[i + 1] - ch1Data[i]) / (waveLengthData[i + 1] - waveLengthData[i]));
                    b.append(ch1Data[i] - k[i] * waveLengthData[i]);
                }
                if(x <= waveLengthData[0])
                    return ch1Data[0];
                if(x >= waveLengthData[n - 1])
                    return ch1Data[n - 1];
                if(waveLengthData.contains(x))
                    return ch1Data[waveLengthData.indexOf(x)];
                for(auto i = 0; i < n - 1; ++i)
                {
                    if(x >= waveLengthData[i] && x <= waveLengthData[i + 1])
                    {
                        return k[i] * x + b[i];
                    }
                }
            }
            break;
        }
        case channels::CH2:
        {
            if((waveLengthData.size() == ch2Data.size()) && ((!std::equal(ch2Data.begin() + 1,ch2Data.end(),ch2Data.begin())) && (ch2Data[0] == 0)))
            {
                int n = ch2Data.size();
                //расчет коэффициентов для ch1 - ch3
                for(int i = 0; i < n - 1; ++i)
                {
                    k.append((ch2Data[i + 1] - ch2Data[i]) / (waveLengthData[i + 1] - waveLengthData[i]));
                    b.append(ch2Data[i] - k[i] * waveLengthData[i]);
                }
                if(x <= waveLengthData[0])
                    return ch2Data[0];
                if(x >= waveLengthData[n - 1])
                    return ch2Data[n - 1];
                if(waveLengthData.contains(x))
                    return ch2Data[waveLengthData.indexOf(x)];
                for(auto i = 0; i < n - 1; ++i)
                {
                    if(x >= waveLengthData[i] && x <= waveLengthData[i + 1])
                    {
                        return k[i] * x + b[i];
                    }
                }
            }
            break;
        }
        case channels::CH3:
        {
            //интерполяция для 1 канала
            if((waveLengthData.size() == ch3Data.size()) && ((!std::equal(ch3Data.begin() + 1,ch3Data.end(),ch3Data.begin())) && (ch3Data[0] == 0)))
            {
                int n = ch3Data.size();
                //расчет коэффициентов для ch1 - ch3
                for(int i = 0; i < n - 1; ++i)
                {
                    k.append((ch3Data[i + 1] - ch3Data[i]) / (waveLengthData[i + 1] - waveLengthData[i]));
                    b.append(ch3Data[i] - k[i] * waveLengthData[i]);
                }
                if(x <= waveLengthData[0])
                    return ch3Data[0];
                if(x >= waveLengthData[n - 1])
                    return ch3Data[n - 1];
                if(waveLengthData.contains(x))
                    return ch3Data[waveLengthData.indexOf(x)];
                for(auto i = 0; i < n - 1; ++i)
                {
                    if(x >= waveLengthData[i] && x <= waveLengthData[i + 1])
                    {
                        return k[i] * x + b[i];
                    }
                }
            }
            break;
        }
        case channels::CH4:
        {
            //интерполяция для 1 канала
            if((waveLengthData.size() == ch4Data.size()) && ((!std::equal(ch4Data.begin() + 1,ch4Data.end(),ch4Data.begin())) && (ch4Data[0] == 0)))
            {
                int n = ch4Data.size();
                //расчет коэффициентов для ch1 - ch3
                for(int i = 0; i < n - 1; ++i)
                {
                    k.append((ch4Data[i + 1] - ch4Data[i]) / (waveLengthData[i + 1] - waveLengthData[i]));
                    b.append(ch4Data[i] - k[i] * waveLengthData[i]);
                }
                if(x <= waveLengthData[0])
                    return ch4Data[0];
                if(x >= waveLengthData[n - 1])
                    return ch4Data[n - 1];
                if(waveLengthData.contains(x))
                    return ch4Data[waveLengthData.indexOf(x)];
                for(auto i = 0; i < n - 1; ++i)
                {
                    if(x >= waveLengthData[i] && x <= waveLengthData[i + 1])
                    {
                        return k[i] * x + b[i];
                    }
                }
            }
            break;
        }
        case channels::CH5:
        {
            //интерполяция для 1 канала
            if((waveLengthData.size() == ch5Data.size()) && ((!std::equal(ch5Data.begin() + 1,ch5Data.end(),ch5Data.begin())) && (ch5Data[0] == 0)))
            {
                int n = ch5Data.size();
                //расчет коэффициентов для ch1 - ch3
                for(int i = 0; i < n - 1; ++i)
                {
                    k.append((ch5Data[i + 1] - ch5Data[i]) / (waveLengthData[i + 1] - waveLengthData[i]));
                    b.append(ch5Data[i] - k[i] * waveLengthData[i]);
                }
                if(x <= waveLengthData[0])
                    return ch5Data[0];
                if(x >= waveLengthData[n - 1])
                    return ch5Data[n - 1];
                if(waveLengthData.contains(x))
                    return ch5Data[waveLengthData.indexOf(x)];
                for(auto i = 0; i < n - 1; ++i)
                {
                    if(x >= waveLengthData[i] && x <= waveLengthData[i + 1])
                    {
                        return k[i] * x + b[i];
                    }
                }
            }
            break;
        }
        case channels::CH6:
        {
            //интерполяция для 1 канала
            if((waveLengthData.size() == ch6Data.size()) && ((!std::equal(ch6Data.begin() + 1,ch6Data.end(),ch6Data.begin())) && (ch6Data[0] == 0)))
            {
                int n = ch6Data.size();
                //расчет коэффициентов для ch1 - ch3
                for(int i = 0; i < n - 1; ++i)
                {
                    k.append((ch6Data[i + 1] - ch6Data[i]) / (waveLengthData[i + 1] - waveLengthData[i]));
                    b.append(ch6Data[i] - k[i] * waveLengthData[i]);
                }
                if(x <= waveLengthData[0])
                    return ch6Data[0];
                if(x >= waveLengthData[n - 1])
                    return ch6Data[n - 1];
                if(waveLengthData.contains(x))
                    return ch6Data[waveLengthData.indexOf(x)];
                for(auto i = 0; i < n - 1; ++i)
                {
                    if(x >= waveLengthData[i] && x <= waveLengthData[i + 1])
                    {
                        return k[i] * x + b[i];
                    }
                }
            }
            break;
        }
        case channels::CH7:
        {
            //интерполяция для 1 канала
            if((waveLengthData.size() == ch7Data.size()) && ((!std::equal(ch7Data.begin() + 1,ch7Data.end(),ch7Data.begin())) && (ch7Data[0] == 0)))
            {
                int n = ch7Data.size();
                //расчет коэффициентов для ch1 - ch3
                for(int i = 0; i < n - 1; ++i)
                {
                    k.append((ch7Data[i + 1] - ch7Data[i]) / (waveLengthData[i + 1] - waveLengthData[i]));
                    b.append(ch7Data[i] - k[i] * waveLengthData[i]);
                }
                if(x <= waveLengthData[0])
                    return ch7Data[0];
                if(x >= waveLengthData[n - 1])
                    return ch7Data[n - 1];
                if(waveLengthData.contains(x))
                    return ch7Data[waveLengthData.indexOf(x)];
                for(auto i = 0; i < n - 1; ++i)
                {
                    if(x >= waveLengthData[i] && x <= waveLengthData[i + 1])
                    {
                        return k[i] * x + b[i];
                    }
                }
            }
            break;
        }
        case channels::CH8:
        {
            //интерполяция для 1 канала
            if((waveLengthData.size() == ch8Data.size()) && ((!std::equal(ch8Data.begin() + 1,ch8Data.end(),ch8Data.begin())) && (ch8Data[0] == 0)))
            {
                int n = ch8Data.size();
                //расчет коэффициентов для ch1 - ch3
                for(int i = 0; i < n - 1; ++i)
                {
                    k.append((ch8Data[i + 1] - ch8Data[i]) / (waveLengthData[i + 1] - waveLengthData[i]));
                    b.append(ch8Data[i] - k[i] * waveLengthData[i]);
                }
                if(x <= waveLengthData[0])
                    return ch8Data[0];
                if(x >= waveLengthData[n - 1])
                    return ch8Data[n - 1];
                if(waveLengthData.contains(x))
                    return ch8Data[waveLengthData.indexOf(x)];
                for(auto i = 0; i < n - 1; ++i)
                {
                    if(x >= waveLengthData[i] && x <= waveLengthData[i + 1])
                    {
                        return k[i] * x + b[i];
                    }
                }
            }
            break;
        }
        default:
            break;
    }
}

void MainWindow::createInterpolatedChannelValues(channels chn)
{
    switch(chn)
    {
        case channels::CH1:
        {
            QDataStream dataStream(&interpolatedCh1Data,QIODevice::WriteOnly);
            double wavelength_value { 1500.0 };
            for(auto i = 0; i < 20000; ++i)
            {
                dataStream << partiallyLinearInterpolation(wavelength_value,channels::CH1);
                wavelength_value += 0.005;
            }
            break;
        }
        case channels::CH2:
        {
            QDataStream dataStream(&interpolatedCh2Data,QIODevice::WriteOnly);
            double wavelength_value { 1500.0 };
            for(auto i = 0; i < 20000; ++i)
            {
                dataStream << partiallyLinearInterpolation(wavelength_value,channels::CH2);
                wavelength_value += 0.005;
            }
            break;
        }
        case channels::CH3:
        {
            QDataStream dataStream(&interpolatedCh3Data,QIODevice::WriteOnly);
            double wavelength_value { 1500.0 };
            for(auto i = 0; i < 20000; ++i)
            {
                dataStream << partiallyLinearInterpolation(wavelength_value,channels::CH3);
                wavelength_value += 0.005;
            }
            break;
        }
        case channels::CH4:
        {
            QDataStream dataStream(&interpolatedCh4Data,QIODevice::WriteOnly);
            double wavelength_value { 1500.0 };
            for(auto i = 0; i < 20000; ++i)
            {
                dataStream << partiallyLinearInterpolation(wavelength_value,channels::CH4);
                wavelength_value += 0.005;
            }
            break;
        }
        case channels::CH5:
        {
            QDataStream dataStream(&interpolatedCh5Data,QIODevice::WriteOnly);
            double wavelength_value { 1500.0 };
            for(auto i = 0; i < 20000; ++i)
            {
                dataStream << partiallyLinearInterpolation(wavelength_value,channels::CH5);
                wavelength_value += 0.005;
            }
            break;
        }
        case channels::CH6:
        {
            QDataStream dataStream(&interpolatedCh6Data,QIODevice::WriteOnly);
            double wavelength_value { 1500.0 };
            for(auto i = 0; i < 20000; ++i)
            {
                dataStream << partiallyLinearInterpolation(wavelength_value,channels::CH6);
                wavelength_value += 0.005;
            }
            break;
        }
        case channels::CH7:
        {
            QDataStream dataStream(&interpolatedCh7Data,QIODevice::WriteOnly);
            double wavelength_value { 1500.0 };
            for(auto i = 0; i < 20000; ++i)
            {
                dataStream << partiallyLinearInterpolation(wavelength_value,channels::CH7);
                wavelength_value += 0.005;
            }
            break;
        }
        case channels::CH8:
        {
            QDataStream dataStream(&interpolatedCh8Data,QIODevice::WriteOnly);
            double wavelength_value { 1500.0 };
            for(auto i = 0; i < 20000; ++i)
            {
                dataStream << partiallyLinearInterpolation(wavelength_value,channels::CH8);
                wavelength_value += 0.005;
            }
            break;
        }
        default:
            break;
    }
}


