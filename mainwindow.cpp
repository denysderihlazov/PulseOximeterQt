#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QRegularExpression>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QVBoxLayout>
#include <QLayoutItem>
#include <QValueAxis>


// using namespace QtCharts;

#include "stdlib.h"

// Private variables
float temperature = 0;
int bmp = 0;
int spo2 = 0;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , serial(new QSerialPort(this)) // Init Serial Port
{
    ui->setupUi(this);
    createChart();

    // Setup Serial port
    serial->setPortName("COM15"); // +++ADD A FUNCTIONALITY TO AUTOMATICALLY SEARCH FOR PORT

    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    // Try to open COM port
    if(serial->open(QIODevice::ReadWrite)) {
        qDebug() << "COM port opened successfully";
    } else {
        qDebug() << "Failed to open COM port" << serial->errorString();
    }

    // Set ready read signal to ReadData slot
    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);
}


MainWindow::~MainWindow()
{
    if(serial->isOpen()) {
        serial->close(); // Close the port when finished
    }
    delete ui;
}


void MainWindow::readData() {
    // qDebug() << "Reading data from serial port...";
    static QByteArray dataBuffer;

    // Append new data to the buf
    QByteArray newData = serial->readAll();
    if (newData.isEmpty()) {
        qDebug() << "No data received from serial port!";
        return;
    }

    dataBuffer += newData;

    // Checking if the buffer has whole line (then finish on a new line)
    if (dataBuffer.contains('\n')) {
        QList<QByteArray> lines = dataBuffer.split('\n');

        // Handle each line from Serial UART
        for (int i = 0; i < lines.size() - 1; i++) {
            qDebug() << "Received:" << lines[i].trimmed();

            // Parsing
            SerialPortParser(QString::fromUtf8(lines[i].trimmed()), &temperature, &bmp, &spo2);

            // Refresh UI
            ShowTextData(temperature, bmp, spo2, ui->label_temperature, ui->label_bmp, ui->label_spo2);

            // Append a new temp value and refresh chart
            updateChart(temperature);
        }

        // Save only last one line
        dataBuffer = lines.last();
    }
}



void MainWindow::SerialPortParser(const QString &dataParse, float *temperature, int *bmp, int *spo2) {
    static QRegularExpression regex("T: ([\\d.]+), bpm: (\\d+), SPO2: (\\d+)");
    QRegularExpressionMatch match = regex.match(dataParse);

    if (match.hasMatch()) {
        *temperature = match.captured(1).toFloat();
        *bmp = match.captured(2).toInt();
        *spo2 = match.captured(3).toInt();

        // qDebug() << "Parsed values - Temperature:" << *temperature << "BPM:" << *bmp << "SpO2:" << *spo2;
    } else {
        qDebug() << "Parsing failed for data:" << dataParse;
    }
}


// Write parsed data to UI
void MainWindow::ShowTextData(float &temperature, int &bmp, int &spo2, QLabel *label_temperature, QLabel *label_bmp, QLabel *label_spo2) {
    QString labelTemperatureText = QString("Temperature: %1°C").arg(temperature);
    QString labelBMPText = QString("PRbpm: %1").arg(bmp);
    QString labelSpO2Text = QString("SpO2: %1%").arg(spo2);

    label_temperature->setText(labelTemperatureText);
    label_bmp->setText(labelBMPText);
    label_spo2->setText(labelSpO2Text);
}

void MainWindow::createChart() {
    // Creating a new data for temp
    temperatureSeries = new QLineSeries();

    // Chart init
    chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(temperatureSeries);
    chart->createDefaultAxes();
    chart->setTitle("Temperature Data");

    // Creating QChartView for chart
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Dark theme for chart
    chart->setBackgroundBrush(QBrush(QColor(53, 53, 53)));
    chart->setTitleBrush(QBrush(Qt::white));

    QValueAxis *axisX = new QValueAxis();
    axisX->setLabelFormat("%.1f");
    axisX->setTitleText("Time");
    axisX->setTitleBrush(Qt::white);
    axisX->setLabelsBrush(Qt::white);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(25, 45);
    axisY->setTitleText("Temperature (°C)");
    axisY->setTitleBrush(Qt::white);
    axisY->setLabelsBrush(Qt::white);



    if (ui->widget->layout() != nullptr) {
        QLayoutItem *item;
        while ((item = ui->widget->layout()->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
    } else {
        ui->widget->setLayout(new QVBoxLayout());
    }

    // Adding chartView into widget
    ui->widget->layout()->addWidget(chartView);
}


void MainWindow::updateChart(float newTemperature) {
    // get current amount of points in a seria
    int pointCount = temperatureSeries->count();

    // Max points count
    const int maxPoints = 25;
    if (pointCount >= maxPoints) {
        temperatureSeries->removePoints(0, pointCount - maxPoints + 1);
    }

    // Adding a new point of temp every step
    temperatureSeries->append(pointCount, newTemperature);

    // Refreshing both axis
    chart->axes(Qt::Horizontal).first()->setRange(0, maxPoints - 1);
    chart->axes(Qt::Vertical).first()->setRange(25, 45);
}
