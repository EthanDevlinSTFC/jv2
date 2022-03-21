// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2022 E. Devlin and T. Youngs

#include "graphwidget.h"
#include "./ui_graphwidget.h"
#include "chartview.h"
#include "mainwindow.h"
#include <QChart>
#include <QChartView>
#include <QDateTime>
#include <QDebug>
#include <QInputDialog>
#include <QJsonArray>
#include <QValueAxis>
#include <QXYSeries>

GraphWidget::GraphWidget(QWidget *parent, QChart *chart, QString type) : QWidget(parent), ui_(new Ui::GraphWidget)
{
    type_ = type;
    ui_->setupUi(this);
    ui_->chartView->assignChart(chart);
    connect(ui_->divideByRunRadio, &QRadioButton::toggled, [=]() { runDivideSpinHandling(); });
    connect(ui_->divideByRunSpin, &QSpinBox::editingFinished, [=]() { runDivideSpinHandling(); });
    connect(ui_->divideByMonitorSpin, &QSpinBox::editingFinished, [=]() { monDivideSpinHandling(); });
    connect(ui_->divideByMonitorRadio, &QRadioButton::toggled, [=]() { monDivideSpinHandling(); });

    modified_ = "-1";
    ui_->divideByRunSpin->setSpecialValueText(tr(" "));
    ui_->divideByMonitorSpin->setSpecialValueText(tr(" "));
    ui_->divideByRunSpin->setValue(-1);
    ui_->divideByMonitorSpin->setValue(-1);
}

GraphWidget::~GraphWidget() {}

QString GraphWidget::getChartRuns() { return chartRuns_; }
QString GraphWidget::getChartDetector() { return chartDetector_; }
QJsonArray GraphWidget::getChartData() { return chartData_; }

void GraphWidget::setChartRuns(QString chartRuns) { chartRuns_ = chartRuns; }
void GraphWidget::setChartDetector(QString chartDetector) { chartDetector_ = chartDetector; }
void GraphWidget::setChartData(QJsonArray chartData)
{
    chartData_ = chartData;
    getBinWidths();
}
void GraphWidget::setLabel(QString label)
{
    return; // ui_->statusLabel->setText(label);
}

void GraphWidget::getBinWidths()
{
    binWidths_.clear();
    for (auto run : chartData_)
    {
        QVector<double> binWidths;
        auto runArray = run.toArray();
        for (auto i = 0; i < runArray.count() - 1; i++)
        {
            double binWidth = runArray.at(i + 1)[0].toDouble() - runArray.at(i)[0].toDouble();
            binWidths.append(binWidth);
        }
        binWidths_.append(binWidths);
    }
}

ChartView *GraphWidget::getChartView() { return ui_->chartView; }

void GraphWidget::runDivideSpinHandling()
{
    // IF AMP CHECKED? toggle off -> on?
    if (ui_->divideByRunSpin->isEnabled())
    {
        ui_->countsPerMicrosecondCheck->setChecked(false);
        ui_->countsPerMicrosecondCheck->setEnabled(false);
    }
    else
        ui_->countsPerMicrosecondCheck->setEnabled(true);
    QString value = QString::number(ui_->divideByRunSpin->value());
    if (!ui_->divideByRunSpin->isEnabled())
        value = "-1";
    if (modified_ == value && ui_->divideByRunSpin->isEnabled())
        return;

    if (modified_ != "-1")
    {
        ui_->countsPerMicroAmpCheck->setChecked(false);
        if (type_ == "Detector")
            emit runDivide(chartDetector_, modified_, false);
        else
            emit monDivide(modified_, chartDetector_, false);
        modified_ = "-1";
        if (ui_->divideByRunSpin->isEnabled())
            ui_->countsPerMicroAmpCheck->setChecked(true);
    }

    if (value != "-1" && value != modified_)
    {
        if (type_ == "Detector")
            emit runDivide(chartDetector_, value, true);
        else
            emit monDivide(value, chartDetector_, true);
        modified_ = value;
    }
    if (ui_->countsPerMicroAmpCheck->isChecked())
    {
        ui_->countsPerMicroAmpCheck->setChecked(false);
        ui_->countsPerMicroAmpCheck->setChecked(true);
    }
}

void GraphWidget::monDivideSpinHandling()
{
    if (ui_->divideByRunSpin->isEnabled())
    {
        ui_->countsPerMicrosecondCheck->setChecked(false);
        ui_->countsPerMicrosecondCheck->setEnabled(false);
        ui_->countsPerMicroAmpCheck->setChecked(false);
        ui_->countsPerMicroAmpCheck->setEnabled(false);
    }
    QString value = QString::number(ui_->divideByMonitorSpin->value());
    if (!ui_->divideByMonitorSpin->isEnabled())
        value = "-1";
    if (modified_ == value && ui_->divideByMonitorSpin->isEnabled())
        return;

    if (modified_ != "-1")
    {
        emit monDivide(modified_, chartDetector_, false);
        modified_ = "-1";
    }

    if (value != "-1" && value != modified_)
    {
        emit monDivide(value, chartDetector_, true);
        modified_ = value;
    }
}

void GraphWidget::on_countsPerMicrosecondCheck_stateChanged(int state)
{
    qreal max = 0;
    for (auto i = 0; i < ui_->chartView->chart()->series().count(); i++)
    {
        auto xySeries = qobject_cast<QXYSeries *>(ui_->chartView->chart()->series()[i]);
        auto points = xySeries->points();
        if (state == Qt::Checked)
            for (auto j = 0; j < points.count(); j++)
            {
                auto hold = points[j].y() / binWidths_[i][j];
                if (hold > max)
                    max = hold;
                points[j].setY(hold);
            }
        else
            for (auto j = 0; j < points.count(); j++)
            {
                auto hold = points[j].y() * binWidths_[i][j];
                if (hold > max)
                    max = hold;
                points[j].setY(hold);
            }
        xySeries->replace(points);
        ui_->chartView->chart()->axes()[1]->setMax(max);
    }
}

void GraphWidget::on_countsPerMicroAmpCheck_stateChanged(int state)
{
    if (state == Qt::Checked)
        emit muAmps(chartRuns_, true, modified_);
    else
        emit muAmps(chartRuns_, false, modified_);
}

void GraphWidget::modify(QString values, bool checked)
{
    qreal max = 0;
    for (auto i = 0; i < ui_->chartView->chart()->series().count(); i++)
    {
        double val;
        if (values.split(";").count() > 1)
            val = values.split(";")[i].toDouble();
        else
            val = values.toDouble();
        if (values.split(";").count() > ui_->chartView->chart()->series().count())
            val = values.split(";").last().toDouble() / val;
        auto xySeries = qobject_cast<QXYSeries *>(ui_->chartView->chart()->series()[i]);
        auto points = xySeries->points();
        if (checked)
        {

            for (auto j = 0; j < points.count(); j++)
            {
                auto hold = points[j].y() / val;
                if (hold > max)
                    max = hold;
                points[j].setY(hold);
            }
        }
        else
        {
            for (auto j = 0; j < points.count(); j++)
            {
                auto hold = points[j].y() * val;
                if (hold > max)
                    max = hold;
                points[j].setY(hold);
            }
        }
        xySeries->replace(points);
        ui_->chartView->chart()->axes()[1]->setMax(max);
    }
}

void GraphWidget::modifyAgainstRun(HttpRequestWorker *worker, bool checked)
{
    QJsonArray inputArray;
    QJsonArray valueArray;
    valueArray = worker->json_array[1].toArray();
    valueArray = valueArray;
    qreal max = 0;
    auto dataType = worker->json_array[0].toArray()[2].toString(0);
    for (auto i = 0; i < ui_->chartView->chart()->series().count(); i++)
    {
        if (dataType == "monitor")
            valueArray = valueArray[i].toArray();
        auto xySeries = qobject_cast<QXYSeries *>(ui_->chartView->chart()->series()[i]);
        auto points = xySeries->points();
        if (checked)
        {
            for (auto i = 0; i < points.count(); i++)
            {
                auto val = valueArray.at(i)[1].toDouble();
                if (val != 0)
                {
                    auto hold = points[i].y() / val;
                    if (hold > max)
                        max = hold;
                    points[i].setY(hold);
                }
            }
        }
        else
        {
            for (auto i = 0; i < points.count(); i++)
            {
                auto val = valueArray.at(i)[1].toDouble();
                if (val != 0)
                {
                    auto hold = points[i].y() * val;
                    if (hold > max)
                        max = hold;
                    points[i].setY(hold);
                }
            }
        }
        xySeries->replace(points);
        ui_->chartView->chart()->axes()[1]->setMax(max);
    }
}

void GraphWidget::modifyAgainstMon(HttpRequestWorker *worker, bool checked)
{
    QJsonArray monArray;
    monArray = worker->json_array;
    monArray.removeFirst();
    qreal max = 0;
    for (auto i = 0; i < ui_->chartView->chart()->series().count(); i++)
    {
        auto runArray = monArray[i].toArray();
        auto xySeries = qobject_cast<QXYSeries *>(ui_->chartView->chart()->series()[i]);
        auto points = xySeries->points();
        if (checked)
        {
            for (auto i = 0; i < points.count(); i++)
            {
                auto val = runArray.at(i)[1].toDouble();
                if (val != 0)
                {
                    auto hold = points[i].y() / val;
                    if (hold > max)
                        max = hold;
                    points[i].setY(hold);
                }
            }
        }
        else
        {
            for (auto i = 0; i < points.count(); i++)
            {
                auto val = runArray.at(i)[1].toDouble();
                if (val != 0)
                {
                    auto hold = points[i].y() * val;
                    if (hold > max)
                        max = hold;
                    points[i].setY(hold);
                }
            }
        }
        xySeries->replace(points);
        ui_->chartView->chart()->axes()[1]->setMax(max);
    }
}