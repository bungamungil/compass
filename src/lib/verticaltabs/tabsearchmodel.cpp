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
#include "tabsearchmodel.h"

#include "browserwindow.h"
#include "tabwidget.h"
#include "webtab.h"
#include "closedtabsmanager.h"

#include <QRegularExpression>

bool tabSearchMatches(const QString &title, const QString &url, const QString &filter)
{
    const QString trimmed = filter.trimmed();
    if (trimmed.isEmpty()) {
        return true;
    }

    const QString pattern = QRegularExpression::escape(trimmed).replace(QSL("\\ "), QSL(".*"));
    const QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);

    // Match against title/url individually (fast path for single-word filters)
    // as well as the two concatenated (so a multi-word filter like "kde invent"
    // matches when one word is only in the title and the other only in the url,
    // regardless of which field comes "first").
    if (title.contains(re) || url.contains(re)) {
        return true;
    }
    const QString combined = title + QLatin1Char(' ') + url;
    return combined.contains(re);
}

TabSearchModel::TabSearchModel(BrowserWindow *window, QObject *parent)
    : QAbstractListModel(parent)
    , m_window(window)
{
    rebuild();
}

void TabSearchModel::setFilter(const QString &text)
{
    if (m_filter == text) {
        return;
    }
    m_filter = text;
    rebuild();
}

int TabSearchModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_rows.count();
}

QVariant TabSearchModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.count()) {
        return QVariant();
    }

    const Row &row = m_rows.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
    case TitleRole:
        return row.title;
    case UrlRole:
        return row.url;
    case IconRole:
        return row.icon;
    case SectionRole:
        return static_cast<int>(row.section);
    case WebTabRole:
        return QVariant::fromValue(row.webTab);
    case ClosedIndexRole:
        return row.closedIndex;
    default:
        return QVariant();
    }
}

void TabSearchModel::rebuild()
{
    beginResetModel();
    m_rows.clear();

    if (m_window && m_window->tabWidget()) {
        const auto openTabs = m_window->tabWidget()->allTabs(true);
        for (WebTab *tab : openTabs) {
            if (!tab) {
                continue;
            }
            if (!tabSearchMatches(tab->title(), tab->url().toString(), m_filter)) {
                continue;
            }
            Row row;
            row.section = OpenTabs;
            row.title = tab->title();
            row.url = tab->url();
            row.icon = tab->icon();
            row.webTab = tab;
            m_rows.append(row);
        }

        const auto closedTabs = m_window->tabWidget()->closedTabsManager()->closedTabs();
        for (int i = 0; i < closedTabs.count(); ++i) {
            const ClosedTabsManager::Tab &entry = closedTabs.at(i);
            if (!tabSearchMatches(entry.tabState.title, entry.tabState.url.toString(), m_filter)) {
                continue;
            }
            Row row;
            row.section = RecentlyClosed;
            row.title = entry.tabState.title;
            row.url = entry.tabState.url;
            row.icon = entry.tabState.icon;
            row.closedIndex = i;
            m_rows.append(row);
        }
    }

    endResetModel();
}
