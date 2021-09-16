// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 E. Devlin and T. Youngs

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "jsontablemodel.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkReply>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QtGui>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  fillInstruments();
}

MainWindow::~MainWindow() { delete ui; }

// Fill instrument list
void MainWindow::fillInstruments() {
  QList<QString> instruments = {"default", "merlin", "nimrod", "sandals",
                                "iris"};
  ui->instrumentsBox->clear();
  foreach (const QString instrument, instruments) {
    ui->instrumentsBox->addItem(instrument);
  }
}
  
void MainWindow::on_instrumentsBox_currentIndexChanged(const QString &arg1) {
  // Handle possible undesired calls
  if (arg1 == "default" || arg1 == "") {
    ui->cyclesBox->clear();
    ui->cyclesBox->addItem("default");
    return;
  }
  QString url_str = "http://127.0.0.1:5000/getCycles/" + arg1;
  HttpRequestInput input(url_str);
  HttpRequestWorker *worker = new HttpRequestWorker(this);
  // Call result handler when request completed
  connect(worker, SIGNAL(on_execution_finished(HttpRequestWorker *)), this,
          SLOT(handle_result_instruments(HttpRequestWorker *)));
  worker->execute(&input);
}

void MainWindow::on_cyclesBox_currentIndexChanged(const QString &arg1) {
  // Handle possible undesired calls
  if (arg1 == "default" || arg1 == "") {
    return;
  }
  QString url_str = "http://127.0.0.1:5000/getJournal/" +
                    ui->instrumentsBox->currentText() + "/" + arg1;
  HttpRequestInput input(url_str);
  HttpRequestWorker *worker = new HttpRequestWorker(this);
  // Call result handler when request completed
  connect(worker, SIGNAL(on_execution_finished(HttpRequestWorker *)), this,
          SLOT(handle_result_cycles(HttpRequestWorker *)));
  worker->execute(&input);
}

// Filter table data
void MainWindow::on_filterBox_textChanged(const QString &arg1)
{
    proxyModel->setFilterFixedString(arg1.trimmed());
    proxyModel->setFilterKeyColumn(-1);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
}

// Fills cycles box
void MainWindow::handle_result_instruments(HttpRequestWorker *worker) {
  QString msg;

  if (worker->error_type == QNetworkReply::NoError) {
    auto response = worker->response;
    ui->cyclesBox->clear();
    ui->cyclesBox->addItem("default");
    foreach (const QJsonValue &value, worker->json_array) {
      //removes header file
      if (value.toString() != "journal.xml")
        ui->cyclesBox->addItem(value.toString());
    }
  } else {
    // an error occurred
    msg = "Error1: " + worker->error_str;
    QMessageBox::information(this, "", msg);
  }
}

// Fills table view with run
void MainWindow::handle_result_cycles(HttpRequestWorker *worker) {
  QString msg;

  if (worker->error_type == QNetworkReply::NoError) {
    // Get keys from json data
    auto jsonArray = worker->json_array;
    auto jsonObject = jsonArray.at(0).toObject();
    JsonTableModel::Header header;
    foreach (const QString &key, jsonObject.keys()) {
      header.push_back(
          JsonTableModel::Heading({{"title", key}, {"index", key}}));
    }

    model = new JsonTableModel(header, this);
    proxyModel = new QSortFilterProxyModel;
    proxyModel->setSourceModel(model);
    ui->runDataTable->setModel(proxyModel);
    model->setJson(jsonArray);
    ui->runDataTable->show();

  } else {
    // an error occurred
    msg = "Error2: " + worker->error_str;
    QMessageBox::information(this, "", msg);
  }
}