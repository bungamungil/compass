/* ============================================================
* VerticalTabs plugin for Falkon
* Copyright (C) 2026  Bunga Mungil <bungamungil@icloud.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */
#pragma once

#include "qzcommon.h"

#include <QAbstractListModel>
#include <QIcon>
#include <QUrl>
#include <QVector>

class BrowserWindow;
class WebTab;

// Returns true when either the title or the url contains the (case-insensitive,
// whitespace-as-wildcard) filter text. An empty/blank filter matches everything.
FALKON_EXPORT bool tabSearchMatches(const QString &title, const QString &url, const QString &filter);

class FALKON_EXPORT TabSearchModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Section {
        OpenTabs,
        RecentlyClosed
    };

    enum Roles {
        TitleRole = Qt::UserRole + 1,
        UrlRole,
        IconRole,
        SectionRole,
        WebTabRole,
        ClosedIndexRole
    };

    explicit TabSearchModel(BrowserWindow *window, QObject *parent = nullptr);

    void setFilter(const QString &text);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    void rebuild();

    struct Row {
        Section section;
        QString title;
        QUrl url;
        QIcon icon;
        WebTab *webTab = nullptr;      // valid only for OpenTabs rows
        int closedIndex = -1;          // valid only for RecentlyClosed rows
    };

    BrowserWindow *m_window;
    QString m_filter;
    QVector<Row> m_rows;
};
