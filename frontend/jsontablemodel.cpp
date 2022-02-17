// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2022 E. Devlin and T. Youngs

#include "jsontablemodel.h"
#include <QDebug>
#include <QJsonObject>

// Model to handle json data in table view
JsonTableModel::JsonTableModel(const JsonTableModel::Header &header_, QObject *parent)
    : QAbstractTableModel(parent), m_header(header_)
{
    m_groupedHeader.push_back(Heading({{"title", "Title"}, {"index", "title"}}));
    m_groupedHeader.push_back(Heading({{"title", "Total Duration"}, {"index", "duration"}}));
    m_groupedHeader.push_back(Heading({{"title", "Run Numbers"}, {"index", "run_number"}}));
}

// Sets json data to populate table
bool JsonTableModel::setJson(const QJsonArray &array)
{
    beginResetModel();
    m_json = array;
    endResetModel();
    return true;
}

QJsonArray JsonTableModel::getJson() { return m_json; }

// Sets header_ data to define table
bool JsonTableModel::setHeader(const Header &array)
{
    beginResetModel();
    m_header = array;
    endResetModel();
    return true;
}

JsonTableModel::Header JsonTableModel::getHeader() { return m_header; }

QVariant JsonTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::UserRole)
        return m_header[section]["index"]; // Index == database name

    if (role != Qt::DisplayRole)
        return QVariant();

    switch (orientation)
    {
        case Qt::Horizontal:
            return m_header[section]["title"]; // Title == desired display name
        case Qt::Vertical:
            // return section + 1;
            return QVariant();
        default:
            return QVariant();
    }
}

int JsonTableModel::rowCount(const QModelIndex &parent) const { return m_json.size(); }

int JsonTableModel::columnCount(const QModelIndex &parent) const { return m_header.size(); }

QJsonObject JsonTableModel::getJsonObject(const QModelIndex &index) const
{
    const QJsonValue &value = m_json[index.row()];
    return value.toObject();
}

// Fills table view
QVariant JsonTableModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    QJsonObject obj = getJsonObject(index);
    const QString &key = m_header[index.column()]["index"];
    if (!obj.contains(key))
        return QVariant();
    QJsonValue v = obj[key];

    if (v.isDouble())
        return QString::number(v.toDouble());
    if (!v.isString())
        return QVariant();

    // if title = duration then format
    if (m_header[index.column()]["index"] == "duration")
    {
        total = v.toString().toInt();
        minutes = total / 60;
        seconds = total % 60;
        hours = minutes / 60;
        minutes = minutes % 60;
        return QString(QString::number(hours).rightJustified(2, '0') + ":" + QString::number(minutes).rightJustified(2, '0') +
                       ":" + QString::number(seconds).rightJustified(2, '0'));
    }
    // if value = date then format
    auto date = QDateTime::fromString(v.toString(), "yyyy-MM-dd'T'HH:mm:ss");
    if (date.isValid())
    {
        if (date.date() == QDateTime::currentDate())
            return QString("Today at: ").append(date.toString("HH:mm:ss"));
        if (date.addDays(1).date() == QDateTime::currentDate())
            return QString("Yesterday at: ").append(date.toString("HH:mm:ss"));
        return date.toString("dd/MM/yyyy HH:mm:ss");
    }
    // if title = Run Numbers then format (for grouped data)
    if (m_header[index.column()]["title"] == "Run Numbers")
    {
        // Format grouped runs display
        auto runArr = v.toString().split(";");
        if (runArr.size() == 1)
            return runArr[0];
        QString displayString = runArr[0];
        for (auto i = 1; i < runArr.size(); i++)
            if (runArr[i].toInt() == runArr[i - 1].toInt() + 1)
                displayString += "-" + runArr[i];
            else
                displayString += "," + runArr[i];
        QStringList splitDisplay;
        foreach (const auto &string, displayString.split(","))
        {
            if (string.contains("-"))
                splitDisplay.append(string.left(string.indexOf("-") + 1) +
                                    string.right(string.size() - string.lastIndexOf("-") - 1));
            else
                splitDisplay.append(string);
        }
        return splitDisplay.join(",");
    }
    return v.toString();
}

// Returns grouped table data
void JsonTableModel::groupData()
{
    QJsonArray groupedJson;

    // holds data in tuple as QJson referencing is incomplete
    std::vector<std::tuple<QString, QString, QString>> groupedData;
    for (QJsonValue value : m_json)
    {
        const QJsonObject &valueObj = value.toObject();
        bool unique = true;

        // add duplicate title data to stack
        for (std::tuple<QString, QString, QString> &data : groupedData)
        {
            if (std::get<0>(data) == valueObj["title"].toString())
            {
                auto totalRunTime = std::get<1>(data).toInt() + valueObj["duration"].toString().toInt();
                std::get<1>(data) = QString::number(totalRunTime);
                std::get<2>(data) += ";" + valueObj["run_number"].toString();
                unique = false;
                break;
            }
        }
        if (unique)
            groupedData.push_back(std::make_tuple(valueObj["title"].toString(), valueObj["duration"].toString(),
                                                  valueObj["run_number"].toString()));
    }
    for (std::tuple<QString, QString, QString> data : groupedData)
    {
        auto groupData = QJsonObject({qMakePair(QString("title"), QJsonValue(std::get<0>(data))),
                                      qMakePair(QString("duration"), QJsonValue(std::get<1>(data))),
                                      qMakePair(QString("run_number"), QJsonValue(std::get<2>(data)))});
        groupedJson.push_back(QJsonValue(groupData));
    }
    // Hold ungrouped values
    m_holdJson = m_json;
    m_holdHeader = m_header;

    // Get and assign array headers
    setHeader(m_groupedHeader);
    setJson(groupedJson);
}

// Apply held (ungrouped) values to table
void JsonTableModel::unGroupData()
{
    setHeader(m_holdHeader);
    setJson(m_holdJson);
}

void JsonTableModel::setColumnTitle(int section, QString title) { m_header[section]["index"] = title; }