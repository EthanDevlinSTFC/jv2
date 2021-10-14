#include "mainwindow.h"
#include <tuple>

// Search table data
void MainWindow::on_searchBox_textChanged(const QString &arg1)
{
    std::get<0>(matchesTuple).clear();
    std::get<1>(matchesTuple) = 0;
    if (arg1 == "")
    {
        ui->runDataTable->selectionModel()->clearSelection();
        return;
    }
    // Find all occurences of search string in table elements
    for (int i = 0; i < proxyModel->rowCount(); i++)
    {
        if (ui->runDataTable->isColumnHidden(i) == false)
        {
            std::get<0>(matchesTuple)
                .append(proxyModel->match(proxyModel->index(0, i), Qt::DisplayRole, arg1, -1, Qt::MatchContains));
        }
    }
    // Select first match
    if (std::get<0>(matchesTuple).size() > 0)
    {
        ui->runDataTable->selectionModel()->clearSelection();
        ui->runDataTable->selectionModel()->setCurrentIndex(std::get<0>(matchesTuple)[0],
                                                            QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}

// Select previous match
void MainWindow::on_findUp_clicked()
{
    // Boundary/ error handling
    if (std::get<0>(matchesTuple).size() > 0)
    {
        if (std::get<1>(matchesTuple) >= 1)
        {
            std::get<1>(matchesTuple) -= 1;
        }
        else
        {
            std::get<1>(matchesTuple) = 0;
        }
        ui->runDataTable->selectionModel()->clearSelection();
        ui->runDataTable->selectionModel()->setCurrentIndex(std::get<0>(matchesTuple)[std::get<1>(matchesTuple)],
                                                            QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}

// Select next match
void MainWindow::on_findDown_clicked()
{
    // Boundary/ error handling
    if (std::get<0>(matchesTuple).size() > 0)
    {
        if (std::get<1>(matchesTuple) < std::get<0>(matchesTuple).size() - 1)
        {
            std::get<1>(matchesTuple) += 1;
        }
        ui->runDataTable->selectionModel()->clearSelection();
        ui->runDataTable->selectionModel()->setCurrentIndex(std::get<0>(matchesTuple)[std::get<1>(matchesTuple)],
                                                            QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}

// Select all matches
void MainWindow::on_searchAll_clicked()
{
    // Error handling
    if (std::get<0>(matchesTuple).size() > 0)
    {
        ui->runDataTable->selectionModel()->clearSelection();
        std::get<1>(matchesTuple) = -1;
        for (int i = 0; i < std::get<0>(matchesTuple).size(); i++)
        {
            ui->runDataTable->selectionModel()->setCurrentIndex(std::get<0>(matchesTuple)[i],
                                                                QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    }
}