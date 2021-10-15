#include "mainwindow.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkReply>
#include <QSettings>
#include <QWidgetAction>

// Fills cycles box on request completion
void MainWindow::handle_result_instruments(HttpRequestWorker *worker)
{
    QString msg;
    if (worker->error_type == QNetworkReply::NoError)
    {
        auto response = worker->response;
        ui->cyclesBox->blockSignals(true);
        ui->cyclesBox->clear();
        ui->cyclesBox->addItem("default");
        foreach (const QJsonValue &value, worker->json_array)
        {
            // removes header file
            if (value.toString() != "journal.xml")
                ui->cyclesBox->addItem(value.toString());
        }
        ui->cyclesBox->blockSignals(false);
    }
    else
    {
        // an error occurred
        msg = "Error1: " + worker->error_str;
        QMessageBox::information(this, "", msg);
    }
    if (init)
    {
        recentCycle();
        init = false;
    }
}

// Fills table view with run
void MainWindow::handle_result_cycles(HttpRequestWorker *worker)
{
    QString msg;

    if (worker->error_type == QNetworkReply::NoError)
    {
        // Get keys from json data
        auto jsonArray = worker->json_array;
        auto jsonObject = jsonArray.at(0).toObject();
        header.clear();
        viewMenu->clear();
        foreach (const QString &key, jsonObject.keys())
        {
            header.push_back(JsonTableModel::Heading({{"title", key}, {"index", key}}));

            // Fills viewMenu with all columns
            QCheckBox *checkBox = new QCheckBox(viewMenu);
            QWidgetAction *checkableAction = new QWidgetAction(viewMenu);
            checkableAction->setDefaultWidget(checkBox);
            checkBox->setText(key);
            checkBox->setCheckState(Qt::Checked);
            connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(columnHider(int)));
            viewMenu->addAction(checkableAction);
        }
        // Sets and fills table data
        model = new JsonTableModel(header, this);
        proxyModel = new QSortFilterProxyModel;
        proxyModel->setSourceModel(model);
        ui->runDataTable->setModel(proxyModel);
        model->setJson(jsonArray);
        ui->runDataTable->show();
    }
    else
    {
        // an error occurred
        msg = "Error2: " + worker->error_str;
        QMessageBox::information(this, "", msg);
    }
}

// Update cycles list when Instrument changed
void MainWindow::instrumentsBoxChange(const QString &arg1)
{
    QSettings settings;
    settings.setValue("recentInstrument", arg1);
    // Handle possible undesired calls
    if (arg1 == "default" || arg1 == "")
    {
        ui->cyclesBox->clear();
        ui->cyclesBox->addItem("default");
        ui->filterBox->setEnabled(false);
        ui->searchBox->setEnabled(false);
        return;
    }
    // Configure api call
    QString url_str = "http://127.0.0.1:5000/getCycles/" + arg1;
    HttpRequestInput input(url_str);
    HttpRequestWorker *worker = new HttpRequestWorker(this);
    // Call result handler when request completed
    connect(worker, SIGNAL(on_execution_finished(HttpRequestWorker *)), this,
            SLOT(handle_result_instruments(HttpRequestWorker *)));
    worker->execute(&input);
}

// Populate table with cycle data
void MainWindow::on_cyclesBox_currentTextChanged(const QString &arg1)
{
    QSettings settings;
    settings.setValue("recentCycle", arg1);
    // Handle possible undesired calls
    if (arg1 == "default" || arg1 == "")
    {
        ui->filterBox->setEnabled(false);
        ui->searchBox->setEnabled(false);
        return;
    }
    ui->filterBox->setEnabled(true);
    ui->searchBox->setEnabled(true);
    QString url_str = "http://127.0.0.1:5000/getJournal/" + ui->instrumentsBox->currentText() + "/" + arg1;
    HttpRequestInput input(url_str);
    HttpRequestWorker *worker = new HttpRequestWorker(this);
    // Call result handler when request completed
    connect(worker, SIGNAL(on_execution_finished(HttpRequestWorker *)), this, SLOT(handle_result_cycles(HttpRequestWorker *)));
    worker->execute(&input);
}