#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QLabel>

#include <QLineSeries>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void readData(); // Slot for reading Serial data

private:
    Ui::MainWindow *ui;
    QSerialPort *serial; // init Serial as a class member

    // Chart
    QLineSeries *temperatureSeries;
    QChart *chart;

    // private prototypes
    void SerialPortParser(const QString &dataParse, float *temperature, int *bmp, int *spo2);

    void ShowTextData(float &temperature, int &bmp, int &spo2, QLabel *label_temperature, QLabel *label_bmp, QLabel *label_spo2);

    void createChart();

    void updateChart(float newTemperature);

    // Scan available Ports -> handshake -> if succeed -> connect
    void serialPortScanner();
};
#endif // MAINWINDOW_H
