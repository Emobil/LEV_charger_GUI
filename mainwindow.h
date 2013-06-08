#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QTimer>
#include <QMainWindow>
#include "qcustomplot.h"


//#define SLAVE_ADDRESS 0x55
#define SLAVE_ADDRESS 0x13
#define I2C_DEV "/dev/i2c-1"
#define I2C_BUFFER_SIZE 10 // Number of Bytes

/* TWI commands for µC */
#define TWI_CMD_GET_MPP_DATA 	0x22
#define TWI_CMD_GET_LEV_DATA 	0x23
#define TWI_CMD_SG_EN           0x30
#define TWI_CMD_SG_DE           0x31
#define TWI_CMD_OUT_EN      	0x32
#define TWI_CMD_OUT_DE          0x33
#define TWI_CMD_CHAR_EN         0x34
#define TWI_CMD_CHAR_DE         0x35
#define TWI_CMD_MPP_EN          0x36
#define TWI_CMD_MPP_DE          0x37
#define TWI_CMD_OUT_VOLT        0x38


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setupRealtimeMPPplotting(QCustomPlot *customPlot);
    void setupRealtimeLEVplotting(QCustomPlot *customPlot);
    
private slots:
    void realtimeDataSlot_3();
    void realtimeDataSlot_2();

    void on_checkBox_toggled(bool checked);

    void on_checkBox_2_toggled(bool checked);

    void on_horizontalSlider_valueChanged(int value);

    void on_pushButton_released();

    void on_checkBox_3_toggled(bool checked);

    void on_checkBox_4_toggled(bool checked);

private:
    Ui::MainWindow *ui;
    QTimer dataTimer, dataTimer_2;
};

#endif // MAINWINDOW_H
