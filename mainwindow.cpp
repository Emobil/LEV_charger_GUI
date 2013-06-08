#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cstdlib>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDesktopWidget>
#include <QMessageBox>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupRealtimeLEVplotting(ui->customPlot_3);
    setupRealtimeMPPplotting(ui->customPlot_2);
    statusBar()->clearMessage();
    ui->customPlot_3->replot();
    ui->customPlot_2->replot();

    this->setWindowFlags(Qt::MSWindowsFixedSizeDialogHint); //Set window to fixed size
    this->setWindowFlags(Qt::CustomizeWindowHint); //Set window with no title bar
    this->setWindowFlags(Qt::FramelessWindowHint); //Set a frameless window

}

unsigned char r_buffer[I2C_BUFFER_SIZE],w_buffer[I2C_BUFFER_SIZE];
unsigned char u_sollwert;
double value0 = 0.0, value1 = 0.0, value2 = 0.0, value3 = 0.0, Umax, Imax;

int i2c_read (int slaveaddr, int length, unsigned char *buffer){
        int deviceHandle = open(I2C_DEV,O_RDWR);
        ioctl(deviceHandle, I2C_SLAVE, slaveaddr);
 //       write(deviceHandle, 0x00, 0);
        read(deviceHandle,buffer,length);
        close(deviceHandle);
        return 0;
}

int i2c_write (int slaveaddr, int length, unsigned char *buffer){
        int deviceHandle = open(I2C_DEV,O_WRONLY);
        ioctl(deviceHandle, I2C_SLAVE, slaveaddr);
        write(deviceHandle, buffer, length);
        close(deviceHandle);
        return 0;
}

void MainWindow::setupRealtimeMPPplotting(QCustomPlot *customPlot)
{
  customPlot->setNotAntialiasedElements(QCP::aeAll);
  QFont font,titleFont;
  font.setPointSize(20);
  titleFont.setPointSize(22);
  font.setStyleStrategy(QFont::NoAntialias);
  titleFont.setStyleStrategy(QFont::NoAntialias);
  titleFont.setBold(1);
  customPlot->xAxis->setTickLabelFont(font);
  customPlot->yAxis->setTickLabelFont(font);
  customPlot->yAxis2->setTickLabelFont(font);
  customPlot->legend->setFont(font);

  customPlot->addGraph(customPlot->xAxis, customPlot->yAxis); // blue line
  customPlot->graph(0)->setPen(QPen(Qt::blue));
  customPlot->graph(0)->setName("Eingangsspannung");

  customPlot->addGraph(customPlot->xAxis, customPlot->yAxis); // red line
  customPlot->graph(1)->setPen(QPen(Qt::red));
  customPlot->graph(1)->setName("Ausgangsspannung");

  customPlot->addGraph(customPlot->xAxis, customPlot->yAxis2); // green line
  customPlot->graph(2)->setPen(QPen(Qt::green));
  customPlot->graph(2)->setName("Eingangsstrom");

  customPlot->addGraph(customPlot->xAxis, customPlot->yAxis2); // black line
  customPlot->graph(3)->setPen(QPen(Qt::black));
  customPlot->graph(3)->setName("Ausgangsstrom");

  customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
  customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
  customPlot->xAxis->setAutoTickStep(true);
  customPlot->xAxis->setTickStep(4);
  customPlot->setupFullAxesBox();

  // set labels:
  customPlot->setTitle("Messwerte MPP-Tracking");
  customPlot->setTitleFont(titleFont);
  customPlot->xAxis->setLabel("Zeit");
  customPlot->xAxis->setLabelFont(font);
  customPlot->yAxis->setLabel("Spannung [V]");
  customPlot->yAxis->setLabelFont(font);
  customPlot->yAxis2->setLabelFont(font);
  customPlot->yAxis2->setLabel("Strom [A]");
  //customPlot->yAxis2->setTickLength(3, 3);
  //customPlot->yAxis2->setSubTickLength(1, 1);

  customPlot->yAxis2->setTickLabels(true);
  customPlot->yAxis2->setVisible(true);
  customPlot->yAxis2->setTicks(true);

  customPlot->legend->setVisible(true);
  customPlot->legend->setPositionStyle(QCPLegend::psTopLeft);
  customPlot->legend->setBrush(QBrush(QColor(255,255,255,230)));

 // customPlot->yAxis2->setRange(-1, 10);

  // make left and bottom axes transfer their ranges to right and top axes:
  connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
 // connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

  // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
//  connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
  connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot_2()));

  dataTimer.start(0); // Interval 0 means to refresh as fast as possible
}


void MainWindow::setupRealtimeLEVplotting(QCustomPlot *customPlot)
{
  // include this section to fully disable antialiasing for higher performance:
  customPlot->setNotAntialiasedElements(QCP::aeAll);
  QFont font,titleFont;
  font.setPointSize(20);
  titleFont.setPointSize(22);
  font.setStyleStrategy(QFont::NoAntialias);
  titleFont.setStyleStrategy(QFont::NoAntialias);
  titleFont.setBold(1);
  customPlot->xAxis->setTickLabelFont(font);
  customPlot->yAxis->setTickLabelFont(font);
  customPlot->yAxis2->setTickLabelFont(font);
  customPlot->legend->setFont(font);

  customPlot->addGraph(customPlot->xAxis, customPlot->yAxis); // blue line
  customPlot->graph(0)->setPen(QPen(Qt::blue));
  customPlot->graph(0)->setName("Ladespannung");

  customPlot->addGraph(customPlot->xAxis, customPlot->yAxis2); // red line
  customPlot->graph(1)->setPen(QPen(Qt::red));
  customPlot->graph(1)->setName("Ladestrom");

  customPlot->addGraph(customPlot->xAxis, customPlot->yAxis2); // green line
  customPlot->graph(2)->setPen(QPen(Qt::green));
  customPlot->graph(2)->setName("Batteriestrom");

  customPlot->addGraph(customPlot->xAxis, customPlot->yAxis); // black line
  customPlot->graph(3)->setPen(QPen(Qt::black));
  customPlot->graph(3)->setName("Batteriespannung");

  customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
  customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
  customPlot->xAxis->setAutoTickStep(true);
  customPlot->xAxis->setTickStep(4);
  customPlot->setupFullAxesBox();

  // set labels:
  customPlot->setTitle("Messwerte LEV-Charging");
  customPlot->setTitleFont(titleFont);
  customPlot->xAxis->setLabel("Zeit");
  customPlot->xAxis->setLabelFont(font);
  customPlot->yAxis->setLabel("Spannung [V]");
  customPlot->yAxis->setLabelFont(font);
  customPlot->yAxis2->setLabelFont(font);
  customPlot->yAxis2->setLabel("Strom [A]");
  //customPlot->yAxis2->setTickLength(3, 3);
  //customPlot->yAxis2->setSubTickLength(1, 1);

  customPlot->yAxis2->setTickLabels(true);
  customPlot->yAxis2->setVisible(true);
  customPlot->yAxis2->setTicks(true);

  customPlot->legend->setVisible(true);
  customPlot->legend->setPositionStyle(QCPLegend::psTopLeft);
  customPlot->legend->setBrush(QBrush(QColor(255,255,255,230)));

 // customPlot->yAxis2->setRange(-1, 10);

  // make left and bottom axes transfer their ranges to right and top axes:
  connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
 // connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

  // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
  connect(&dataTimer_2, SIGNAL(timeout()), this, SLOT(realtimeDataSlot_3()));

  dataTimer_2.start(0); // Interval 0 means to refresh as fast as possible
}

void MainWindow::realtimeDataSlot_2()
{

  double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
  w_buffer[0] = TWI_CMD_GET_MPP_DATA;

/*  i2c_write (SLAVE_ADDRESS, 1, w_buffer);
  i2c_read(SLAVE_ADDRESS , I2C_BUFFER_SIZE , r_buffer);
  value0 = (value0 + ((double)((r_buffer[5] << 8 ) | r_buffer[6]) / 2048.0)* 130000.0 / 1740.0) / 2.0; //  U_Out
  value1 = (value1 + ((double)((r_buffer[1] << 8 ) | r_buffer[2]) / 2048.0)* 130000.0 / 1740.0) / 2.0; //  U_IN
  value2 = (value2 + ((((double)((r_buffer[7] << 8 ) | r_buffer[8]) / 2048.0) ) / ( 1.0 + 18000.0/4700.0 )) / 0.02) / 2;     // I_OUT
  value3 = (value3 + ((((double)((r_buffer[3] << 8 ) | r_buffer[4]) / 2048.0) ) / ( 1.0 + 220000.0/2100.0 )) / 0.001) / 2;    // I_IN
*/


  static double lastPointKey = 0;

  if (key-lastPointKey > 0.5) // at most add point every 10 ms
  {

     if(value0 < 80.0 && value1 < 80.0 && value3 < 10.0 )
    {
         if(value0 > Umax) {Umax = value0;}
         if(value1 > Umax) {Umax = value1;}
       //  if(value2 > Imax) {Imax = value2;}
         if(value3 > Imax) {Imax = value3;}

         // add data to lines:
        ui->customPlot_2->graph(0)->addData(key, value0);
        ui->customPlot_2->graph(1)->addData(key, value1);
    //    ui->customPlot_2->graph(2)->addData(key, value2);
        ui->customPlot_2->graph(3)->addData(key, value3);

        // remove data of lines that's outside visible range:
        ui->customPlot_2->graph(0)->removeDataBefore(key-20);
        ui->customPlot_2->graph(1)->removeDataBefore(key-20);
    //    ui->customPlot_2->graph(2)->removeDataBefore(key-20);
        ui->customPlot_2->graph(3)->removeDataBefore(key-20);
        // rescale value (vertical) axis to fit the current data:
        ui->customPlot_2->graph(0)->rescaleValueAxis(true);
 //       ui->customPlot_2->graph(1)->rescaleValueAxis(true);
        ui->customPlot_2->graph(2)->rescaleValueAxis(true);
        ui->customPlot_2->graph(3)->rescaleValueAxis(true);
        lastPointKey = key;
     }
  }

  // make key axis range scroll with the data (at a constant range size of 8):
  ui->customPlot_2->xAxis->setRange(key+0.1, 20, Qt::AlignRight);
 // ui->customPlot_2->yAxis->setRange(value0+2.0, 20, Qt::AlignRight);
 // ui->customPlot_2->yAxis2->setRange(value3+1, 5, Qt::AlignLeft);

  ui->customPlot_2->yAxis2->setRangeLower(-0.5);
  ui->customPlot_2->yAxis->setRangeLower(-0.5);
  ui->customPlot_2->yAxis->setRangeUpper(Umax+2);
  ui->customPlot_2->yAxis2->setRangeUpper(Imax+1);

  ui->customPlot_2->replot();
}


void MainWindow::realtimeDataSlot_3()
{

  double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
  w_buffer[0] = TWI_CMD_GET_LEV_DATA;

  i2c_write (SLAVE_ADDRESS, 1, w_buffer);
  i2c_read(SLAVE_ADDRESS , I2C_BUFFER_SIZE , r_buffer);
  value0 = (value0 + ((double)((r_buffer[5] << 8 ) | r_buffer[6]) / 2048.0)* 130000.0 / 1740.0) / 2.0; //  U_Out
  value1 = (value1 + ((double)((r_buffer[1] << 8 ) | r_buffer[2]) / 2048.0)* 130000.0 / 1740.0) / 2.0; //  U_IN
  value2 = (value2 + ((((double)((r_buffer[7] << 8 ) | r_buffer[8]) / 2048.0) ) / ( 1.0 + 18000.0/4700.0 )) / 0.02) / 2;     // I_OUT
  value3 = (value3 + ((((double)((r_buffer[3] << 8 ) | r_buffer[4]) / 2048.0) ) / ( 1.0 + 220000.0/2100.0 )) / 0.001) / 2;    // I_IN

  static double lastPointKey = 0;

  if (key-lastPointKey > 0.5) // at most add point every 10 ms
  {

    // if(value0 < 80.0 && value1 < 80.0 && value3 < 10.0 )
   // {
         if(value0 > Umax) {Umax = value0;}
         if(value1 > Umax) {Umax = value1;}
       //  if(value2 > Imax) {Imax = value2;}
         if(value3 > Imax) {Imax = value3;}

         // add data to lines:
        ui->customPlot_3->graph(0)->addData(key, value0);
        ui->customPlot_3->graph(1)->addData(key, value1);
    //    ui->customPlot_3->graph(2)->addData(key, value2);
        ui->customPlot_3->graph(3)->addData(key, value3);

        // remove data of lines that's outside visible range:
        ui->customPlot_3->graph(0)->removeDataBefore(key-20);
        ui->customPlot_3->graph(1)->removeDataBefore(key-20);
    //    ui->customPlot_3->graph(2)->removeDataBefore(key-20);
        ui->customPlot_3->graph(3)->removeDataBefore(key-20);
        // rescale value (vertical) axis to fit the current data:
        ui->customPlot_3->graph(0)->rescaleValueAxis(true);
 //       ui->customPlot_3->graph(1)->rescaleValueAxis(true);
        ui->customPlot_3->graph(2)->rescaleValueAxis(true);
        ui->customPlot_3->graph(3)->rescaleValueAxis(true);
        lastPointKey = key;
    // }
  }

  // make key axis range scroll with the data (at a constant range size of 8):
  ui->customPlot_3->xAxis->setRange(key+0.1, 20, Qt::AlignRight);
 // ui->customPlot_2->yAxis->setRange(value0+2.0, 20, Qt::AlignRight);
 // ui->customPlot_2->yAxis2->setRange(value3+1, 5, Qt::AlignLeft);

//  ui->customPlot_3->yAxis2->setRangeLower(-0.5);
//  ui->customPlot_3->yAxis->setRangeLower(-0.5);
//  ui->customPlot_3->yAxis->setRangeUpper(Umax+2);
//  ui->customPlot_3->yAxis2->setRangeUpper(Imax+1);

  ui->customPlot_3->replot();
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_checkBox_toggled(bool checked)
{
    if (checked == TRUE){ w_buffer[0] = TWI_CMD_MPP_EN;}
    else {  w_buffer[0] = TWI_CMD_MPP_DE;   }
    i2c_write (SLAVE_ADDRESS, 1, w_buffer);
}


void MainWindow::on_checkBox_2_toggled(bool checked)
{
    if (checked == TRUE){ w_buffer[0] = TWI_CMD_SG_EN;}
    else {  w_buffer[0] = TWI_CMD_SG_DE;   }
    i2c_write (SLAVE_ADDRESS, 1, w_buffer);
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    u_sollwert = (unsigned char) value;
}

void MainWindow::on_pushButton_released()
{
    w_buffer[0] = TWI_CMD_OUT_VOLT;
    w_buffer[1] = u_sollwert;
    i2c_write (SLAVE_ADDRESS, 2, w_buffer);
}

void MainWindow::on_checkBox_3_toggled(bool checked)
{
    if (checked == TRUE){ w_buffer[0] = TWI_CMD_OUT_EN;}
    else {  w_buffer[0] = TWI_CMD_OUT_DE;   }
    i2c_write (SLAVE_ADDRESS, 1, w_buffer);
}



void MainWindow::on_checkBox_4_toggled(bool checked)
{

    if (checked == TRUE){ w_buffer[0] = TWI_CMD_CHAR_EN;}
    else {  w_buffer[0] = TWI_CMD_CHAR_DE;   }
    i2c_write (SLAVE_ADDRESS, 1, w_buffer);
}

