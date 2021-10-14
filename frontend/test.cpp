#include "mainwindow.h"
#include <QDebug>
void MainWindow::test(){
    qDebug() << "this works";
    ui->groupButton->setText("beans");
}