// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2022 E. Devlin and T. Youngs

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "httprequestworker.h"
#include "jsontablemodel.h"
#include <QChart>
#include <QCheckBox>
#include <QDomDocument>
#include <QMainWindow>
#include <QSortFilterProxyModel>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    // Init
    void fillInstruments(QList<QPair<QString, QString>> instruments);
    void initialiseElements();
    // Misc
    void goToCurrentFoundIndex(QModelIndex index);   // Selects given index
    QList<QPair<QString, QString>> getInstruments(); // Get Instruments from config file
    std::vector<std::pair<QString, QString>> getFields(QString instrument, QString instType); // Get Fields from config file
    void setLoadScreen(bool state);
    private slots:
    // Search Controls
    void updateSearch(const QString &arg1);
    void on_actionSearch_triggered();
    void on_actionSelectNext_triggered();
    void on_actionSelectPrevious_triggered();
    void on_actionSelectAll_triggered();
    void findUp();
    void findDown();
    void selectAllSearches();
    void selectIndex(QString runNumber);
    void selectSimilar();
    // Filter Controls
    void on_filterBox_textChanged(const QString &arg1);
    void on_clearSearchButton_clicked();
    void massSearch(QString name, QString value);
    void on_actionMassSearchRB_No_triggered();
    void on_actionMassSearchTitle_triggered();
    void on_actionMassSearchUser_triggered();
    void on_actionClear_cached_searches_triggered();
    void goTo(HttpRequestWorker *worker, QString runNumber);
    void on_actionRun_Number_triggered();
    // Data Selection
    void handle_result_instruments(HttpRequestWorker *worker);
    void handle_result_cycles(HttpRequestWorker *worker);
    void currentInstrumentChanged(const QString &arg1);
    void on_cyclesBox_currentIndexChanged(int index);
    void recentCycle();
    void changeInst(QPair<QString, QString> instrument);
    // Grouping
    void on_groupButton_clicked(bool checked);
    // Visualisation
    void customMenuRequested(QPoint pos);
    void handle_result_contextGraph(HttpRequestWorker *worker);
    void contextGraph();
    void handle_result_contextMenu(HttpRequestWorker *worker);
    void toggleAxis(int state);
    void showStatus(qreal x, qreal y);
    // Misc Interface Functions
    void removeTab(int index);
    void savePref();
    void columnHider(int state);

    protected:
    // Window close event
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);

    signals:
    void tableFilled();

    private:
    Ui::MainWindow *ui_;
    // Table Stuff
    JsonTableModel *model_;
    QSortFilterProxyModel *proxyModel_;
    JsonTableModel::Header header_;
    std::vector<std::pair<QString, QString>> desiredHeader_;
    // Menus
    QMenu *viewMenu_;
    QMenu *findMenu_;
    QMenu *contextMenu_;
    QMenu *instrumentsMenu_;

    QModelIndexList foundIndices_;
    int currentFoundIndex_;
    // Menu button data
    QString searchString_;
    QString instType_;
    QString instName_;
    // Misc
    bool init_;
    QPoint pos_;
    QList<std::tuple<HttpRequestWorker *, QString>> cachedMassSearch_;
};
#endif // MAINWINDOW_H
