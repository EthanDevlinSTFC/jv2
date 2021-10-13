// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 E. Devlin and T. Youngs

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "jsontablemodel.h"
#include <QCheckBox>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkReply>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QWidgetAction>
#include <QtGui>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  initialiseElements();
}

MainWindow::~MainWindow() { delete ui; }

// Hide column on view menu change
void MainWindow::columnHider(int state) {
  QCheckBox *action = qobject_cast<QCheckBox *>(sender());

  for (int i = 0; i < ui->runDataTable->horizontalHeader()->count(); i++) {
    if (action->text() ==
        ui->runDataTable->horizontalHeader()->model()->headerData(
            i, Qt::Horizontal)) {
      switch (action->checkState()) {
      case Qt::Unchecked:
        ui->runDataTable->setColumnHidden(i, true);
        break;
      case Qt::Checked:
        ui->runDataTable->setColumnHidden(i, false);
        break;
      default:
        ui->runDataTable->setColumnHidden(i, false);
      }
      break;
    }
  }
}

void MainWindow::initialiseElements() {
  fillInstruments();
  init = true;
  viewMenu = ui->menubar->addMenu("View");
  ui->runDataTable->horizontalHeader()->setSectionsMovable(true);
  ui->runDataTable->horizontalHeader()->setDragEnabled(true);
  ui->runDataTable->setAlternatingRowColors(true);
  ui->runDataTable->setStyleSheet("alternate-background-color: #e7e7e6;");

  QSettings settings;
  QString recentInstrument = settings.value("recentInstrument").toString();
  qDebug() << "Recent Inst: " << recentInstrument;
  int instrumentIndex = ui->instrumentsBox->findText(recentInstrument);
  if (instrumentIndex != -1) {
    ui->instrumentsBox->setCurrentIndex(instrumentIndex);
  } else {
    ui->instrumentsBox->setCurrentIndex(0);
  }

  recentCycle();
}

void MainWindow::recentCycle() {
  QSettings settings;
  QString recentCycle = settings.value("recentCycle").toString();
  qDebug() << "Recent Cycle: " << recentCycle;
  int cycleIndex = ui->cyclesBox->findText(recentCycle);
  if (ui->instrumentsBox->currentText() != "default" &&
      ui->instrumentsBox->currentText() != "") {
    if (cycleIndex != -1) {
      ui->cyclesBox->setCurrentIndex(cycleIndex);
    } else if (ui->cyclesBox->currentText() != "default" &&
               ui->cyclesBox->currentText() != "") {
      ui->cyclesBox->setCurrentIndex(ui->cyclesBox->count());
    }
  } else {
    ui->cyclesBox->setCurrentIndex(0);
  }
}

// Fill instrument list
void MainWindow::fillInstruments() {
  QList<QString> instruments = {"default", "merlin", "nimrod", "sandals",
                                "iris"};
  ui->instrumentsBox->clear();
  foreach (const QString instrument, instruments) {
    ui->instrumentsBox->addItem(instrument);
  }
}

void MainWindow::instrumentsBoxChange(const QString &arg1) {
  qDebug() << "inst change: " << arg1;
  QSettings settings;
  settings.setValue("recentInstrument", arg1);
  // Handle possible undesired calls
  if (arg1 == "default" || arg1 == "") {
    ui->cyclesBox->clear();
    ui->cyclesBox->addItem("default");
    ui->filterBox->setEnabled(false);
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

void MainWindow::on_cyclesBox_currentTextChanged(const QString &arg1) {
  qDebug() << "cycle change: " << arg1;
  QSettings settings;
  settings.setValue("recentCycle", arg1);
  // Handle possible undesired calls
  if (arg1 == "default" || arg1 == "") {
    ui->filterBox->setEnabled(false);
    return;
  }
  ui->filterBox->setEnabled(true);
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
void MainWindow::on_filterBox_textChanged(const QString &arg1) {
  proxyModel->setFilterFixedString(arg1.trimmed());
  proxyModel->setFilterKeyColumn(-1);
  proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
}

// Filter table data
void MainWindow::on_searchBox_textChanged(const QString &arg1) {
  std::get<0>(matchesTuple).clear();
  std::get<1>(matchesTuple) = 0;
  if (arg1 == "") {
    ui->runDataTable->selectionModel()->clearSelection();
    return;
  }
  for (int i = 0; i < proxyModel->rowCount(); i++) {
    if (ui->runDataTable->isColumnHidden(i) == false) {
      std::get<0>(matchesTuple)
          .append(proxyModel->match(proxyModel->index(0, i), Qt::DisplayRole,
                                    arg1, -1));
    }
  }
  for (QModelIndex match : std::get<0>(matchesTuple)) {
    qDebug() << match.data().toString();
  }
  qDebug() << "";
  if (std::get<0>(matchesTuple).size() > 0) {
    qDebug() << "selecting" << std::get<0>(matchesTuple)[0].data().toString();
    ui->runDataTable->selectionModel()->clearSelection();
    ui->runDataTable->selectionModel()->setCurrentIndex(
        std::get<0>(matchesTuple)[0],
        QItemSelectionModel::Select | QItemSelectionModel::Rows);
  }
}

void MainWindow::on_findUp_clicked() {
  if (std::get<0>(matchesTuple).size() > 0) {
    if (std::get<1>(matchesTuple) >= 1) {
      std::get<1>(matchesTuple) -= 1;
    } else {
      std::get<1>(matchesTuple) = 0;
    }
    qDebug() << "selecting"
             << std::get<0>(matchesTuple)[std::get<1>(matchesTuple)]
                    .data()
                    .toString();
    ui->runDataTable->selectionModel()->clearSelection();
    ui->runDataTable->selectionModel()->setCurrentIndex(
        std::get<0>(matchesTuple)[std::get<1>(matchesTuple)],
        QItemSelectionModel::Select | QItemSelectionModel::Rows);
  }
}

void MainWindow::on_findDown_clicked() {
  if (std::get<0>(matchesTuple).size() > 0) {
    if (std::get<1>(matchesTuple) < std::get<0>(matchesTuple).size() - 1) {
      std::get<1>(matchesTuple) += 1;
    }
    qDebug() << "selecting"
             << std::get<0>(matchesTuple)[std::get<1>(matchesTuple)]
                    .data()
                    .toString();
    ui->runDataTable->selectionModel()->clearSelection();
    ui->runDataTable->selectionModel()->setCurrentIndex(
        std::get<0>(matchesTuple)[std::get<1>(matchesTuple)],
        QItemSelectionModel::Select | QItemSelectionModel::Rows);
  }
}

void MainWindow::on_searchAll_clicked() {
  if (std::get<0>(matchesTuple).size() > 0) {
    ui->runDataTable->selectionModel()->clearSelection();
    std::get<1>(matchesTuple) = -1;
    for (int i = 0; i < std::get<0>(matchesTuple).size(); i++) {
      ui->runDataTable->selectionModel()->setCurrentIndex(
          std::get<0>(matchesTuple)[i],
          QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
  }
}
// Fills cycles box
void MainWindow::handle_result_instruments(HttpRequestWorker *worker) {
  QString msg;
  qDebug() << "fill";
  if (worker->error_type == QNetworkReply::NoError) {
    auto response = worker->response;
    ui->cyclesBox->blockSignals(true);
    ui->cyclesBox->clear();
    ui->cyclesBox->addItem("default");
    foreach (const QJsonValue &value, worker->json_array) {
      // removes header file
      if (value.toString() != "journal.xml")
        ui->cyclesBox->addItem(value.toString());
    }
    ui->cyclesBox->blockSignals(false);
  } else {
    // an error occurred
    msg = "Error1: " + worker->error_str;
    QMessageBox::information(this, "", msg);
  }
  if (init) {
    recentCycle();
    init = false;
  }
}

// Fills table view with run
void MainWindow::handle_result_cycles(HttpRequestWorker *worker) {
  QString msg;

  if (worker->error_type == QNetworkReply::NoError) {
    // Get keys from json data
    auto jsonArray = worker->json_array;
    auto jsonObject = jsonArray.at(0).toObject();
    header.clear();
    viewMenu->clear();
    foreach (const QString &key, jsonObject.keys()) {
      header.push_back(
          JsonTableModel::Heading({{"title", key}, {"index", key}}));

      QCheckBox *checkBox = new QCheckBox(viewMenu);
      QWidgetAction *checkableAction = new QWidgetAction(viewMenu);
      checkableAction->setDefaultWidget(checkBox);
      checkBox->setText(key);
      checkBox->setCheckState(Qt::Checked);
      connect(checkBox, SIGNAL(stateChanged(int)), this,
              SLOT(columnHider(int)));
      viewMenu->addAction(checkableAction);
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

void MainWindow::on_groupButton_clicked(bool checked) {
  if (checked) {
    model->groupData();
  } else {
    model->unGroupData();
  }
}

void MainWindow::on_clearSearchButton_clicked() { ui->filterBox->clear(); }
