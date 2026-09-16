/* ============================================================
* Falkon - Qt web browser
* Copyright (C) 2026 Bunga Mungil <bungamungil@icloud.com>
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
#include "verticaltabstest.h"
#include "autotests.h"
#include "tablistview.h"
#include "tablistdelegate.h"
#include "tabfiltermodel.h"
#include "verticaltabswidget.h"
#include "tabmodel.h"
#include "tabwidget.h"
#include "mainapplication.h"
#include "browserwindow.h"

#include <QStyleOptionViewItem>
#include <QUrl>

void VerticalTabsTest::listGrowsWithTabs()
{
    BrowserWindow *w = mApp->createWindow(Qz::BW_NewWindow);

    auto *view = new TabListView(w);
    auto *model = new TabFilterModel(view);
    model->setFilterPinnedTabs(true);
    model->setSourceModel(w->tabModel());
    view->setModel(model);
    view->setAutoHeight(false);

    QTRY_COMPARE(model->rowCount(), 1);
    QCOMPARE(view->sizeHint().height(), model->rowCount() * TabListDelegate::IconOnlyCell);

    w->tabWidget()->addView(QUrl());
    QTRY_COMPARE(model->rowCount(), 2);
    QCOMPARE(view->sizeHint().height(), model->rowCount() * TabListDelegate::IconOnlyCell);

    w->tabWidget()->addView(QUrl());
    QTRY_COMPARE(model->rowCount(), 3);
    QCOMPARE(view->sizeHint().height(), model->rowCount() * TabListDelegate::IconOnlyCell);

    QVERIFY(view->minimumSizeHint().height() <= TabListDelegate::IconOnlyCell);

    delete view;
    delete w;
}

void VerticalTabsTest::rowHeightMatchesIconOnly()
{
    BrowserWindow *w = mApp->createWindow(Qz::BW_NewWindow);

    auto *view = new TabListView(w);

    view->setIconOnly(true);
    QCOMPARE(view->itemDelegate()->sizeHint(QStyleOptionViewItem(), QModelIndex()).height(), TabListDelegate::IconOnlyCell);

    view->setIconOnly(false);
    QCOMPARE(view->itemDelegate()->sizeHint(QStyleOptionViewItem(), QModelIndex()).height(), TabListDelegate::IconOnlyCell);

    delete view;
    delete w;
}

void VerticalTabsTest::expandedMinimumWidth()
{
    BrowserWindow *w = mApp->createWindow(Qz::BW_NewWindow);

    VerticalTabsWidget v(w);

    v.setIconOnly(false);
    QCOMPARE(v.minimumWidth(), VerticalTabsWidget::ExpandedMinWidth);

    v.setIconOnly(true);
    QCOMPARE(v.minimumWidth(), 56);
    QCOMPARE(v.maximumWidth(), 56);

    delete w;
}

FALKONTEST_MAIN(VerticalTabsTest)
