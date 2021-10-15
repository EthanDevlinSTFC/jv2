// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 E. Devlin and T. Youngs

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "jsontablemodel.h"
#include "test.cpp"
#include "searching.cpp"
#include "datagathering.cpp"
#include "filtering.cpp"
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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initialiseElements();
    test();
}

MainWindow::~MainWindow() { delete ui; }

// Configure initial application state
void MainWindow::initialiseElements()
{
    fillInstruments();
    // First Iteration variable for set-up commands
    init = true;
    // View menu for column toggles
    viewMenu = ui->menubar->addMenu("View");
    // Allows re-arranging of table columns
    ui->runDataTable->horizontalHeader()->setSectionsMovable(true);
    ui->runDataTable->horizontalHeader()->setDragEnabled(true);
    ui->runDataTable->setAlternatingRowColors(true);
    ui->runDataTable->setStyleSheet("alternate-background-color: #e7e7e6;");

    // Sets instrument to last used
    QSettings settings;
    QString recentInstrument = settings.value("recentInstrument").toString();
    int instrumentIndex = ui->instrumentsBox->findText(recentInstrument);
    if (instrumentIndex != -1)
    {
        ui->instrumentsBox->setCurrentIndex(instrumentIndex);
    }
    else
    {
        ui->instrumentsBox->setCurrentIndex(0);
    }
    // Sets cycle to most recently viewed
    recentCycle();
}

// Sets cycle to most recently viewed
void MainWindow::recentCycle()
{
    QSettings settings;
    QString recentCycle = settings.value("recentCycle").toString();
    int cycleIndex = ui->cyclesBox->findText(recentCycle);
    // Sets cycle to last used/ most recent if unavailable
    if (ui->instrumentsBox->currentText() != "default" && ui->instrumentsBox->currentText() != "")
    {
        if (cycleIndex != -1)
        {
            ui->cyclesBox->setCurrentIndex(cycleIndex);
        }
        else if (ui->cyclesBox->currentText() != "default" && ui->cyclesBox->currentText() != "")
        {
            ui->cyclesBox->setCurrentIndex(ui->cyclesBox->count());
        }
    }
    else
    {
        ui->cyclesBox->addItem("default");
    }
}

// Fill instrument list
void MainWindow::fillInstruments()
{
    QList<QString> instruments = {"default", "merlin", "nimrod", "sandals", "iris"};
    ui->instrumentsBox->clear();
    foreach (const QString instrument, instruments)
    {
        ui->instrumentsBox->addItem(instrument);
    }
    // Only allow calls after initial population
    connect(ui->instrumentsBox, SIGNAL(currentTextChanged(const QString)), this, SLOT(instrumentsBoxChange(const QString)));
}


