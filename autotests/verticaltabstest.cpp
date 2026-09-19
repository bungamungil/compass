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
#include "qzsettings.h"

#include <QCoreApplication>
#include <QApplication>
#include <QScopeGuard>
#include <QScrollBar>
#include <QSplitter>
#include <QStyleOptionViewItem>
#include <QUrl>
#include <QVBoxLayout>
#include <QWheelEvent>

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

void VerticalTabsTest::initialIconOnlyWidth()
{
    const bool oldEnabled = qzSettings->verticalTabsEnabled;
    const bool oldIconOnly = qzSettings->verticalTabsIconOnly;
    qzSettings->verticalTabsEnabled = true;
    qzSettings->verticalTabsIconOnly = true;

    BrowserWindow *w = mApp->createWindow(Qz::BW_NewWindow);
    const auto cleanup = qScopeGuard([w, oldEnabled, oldIconOnly] {
        delete w;
        qzSettings->verticalTabsEnabled = oldEnabled;
        qzSettings->verticalTabsIconOnly = oldIconOnly;
        qzSettings->saveSettings();
    });
    w->resize(1000, 700);
    w->show();
    QCoreApplication::processEvents();

    auto *splitter = w->findChild<QSplitter*>(QSL("verticaltabs-splitter"));
    QVERIFY(splitter);
    QVERIFY(w->verticalTabs());
    QVERIFY(w->verticalTabs()->isIconOnly());
    QCOMPARE(w->verticalTabs()->minimumWidth(), 56);
    QTRY_COMPARE(splitter->sizes().constFirst(), 56);

}

void VerticalTabsTest::expandedWidthSurvivesModeToggle()
{
    const bool oldEnabled = qzSettings->verticalTabsEnabled;
    const bool oldIconOnly = qzSettings->verticalTabsIconOnly;
    qzSettings->verticalTabsEnabled = false;
    qzSettings->verticalTabsIconOnly = false;

    BrowserWindow *w = mApp->createWindow(Qz::BW_NewWindow);
    const auto cleanup = qScopeGuard([w, oldEnabled, oldIconOnly] {
        delete w;
        qzSettings->verticalTabsEnabled = oldEnabled;
        qzSettings->verticalTabsIconOnly = oldIconOnly;
        qzSettings->saveSettings();
    });
    w->resize(1000, 700);
    w->showVerticalTabs(true);
    w->show();
    QCoreApplication::processEvents();

    auto *splitter = w->findChild<QSplitter*>(QSL("verticaltabs-splitter"));
    VerticalTabsWidget *tabs = w->verticalTabs();
    QVERIFY(splitter);
    QVERIFY(tabs);

    tabs->setIconOnly(false);
    splitter->setSizes({240, 760});
    QCoreApplication::processEvents();
    const int expandedWidth = splitter->sizes().constFirst();
    QVERIFY(expandedWidth >= VerticalTabsWidget::ExpandedMinWidth);
    QMetaObject::invokeMethod(splitter, "splitterMoved", Qt::DirectConnection,
                              Q_ARG(int, expandedWidth), Q_ARG(int, 1));

    for (int i = 0; i < 2; ++i) {
        tabs->setIconOnly(true);
        QTRY_COMPARE(splitter->sizes().constFirst(), 56);
        tabs->setIconOnly(false);
        QTRY_COMPARE(splitter->sizes().constFirst(), expandedWidth + 1);
    }

}

void VerticalTabsTest::listBlendsAndHidesScrollbar()
{
    BrowserWindow *w = mApp->createWindow(Qz::BW_NewWindow);
    TabListView view(w);

    QCOMPARE(view.palette().color(QPalette::Base).alpha(), 0);
    QCOMPARE(view.viewport()->palette().color(QPalette::Base).alpha(), 0);
    QVERIFY(!view.viewport()->autoFillBackground());
    QCOMPARE(view.verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);

    view.setAutoHeight(false);
    QCOMPARE(view.verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
    delete w;
}

void VerticalTabsTest::hiddenScrollbarStillScrolls()
{
    BrowserWindow *w = mApp->createWindow(Qz::BW_NewWindow);
    TabListView view(w);
    auto *model = new TabFilterModel(&view);
    model->setFilterPinnedTabs(true);
    model->setSourceModel(w->tabModel());
    view.setModel(model);
    view.setAutoHeight(false);

    for (int i = 0; i < 5; ++i) {
        w->tabWidget()->addView(QUrl());
    }
    view.resize(180, TabListDelegate::IconOnlyCell * 2);
    view.show();
    QCoreApplication::processEvents();

    QScrollBar *scrollBar = view.verticalScrollBar();
    QVERIFY(scrollBar->maximum() > scrollBar->minimum());
    QVERIFY(!scrollBar->isVisible());
    const int before = scrollBar->value();
    const QPointF localPos(20, 20);
    QWheelEvent event(localPos,
                      view.viewport()->mapToGlobal(localPos.toPoint()),
                      QPoint(), QPoint(0, -120), Qt::NoButton,
                      Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(view.viewport(), &event);
    QTRY_VERIFY(scrollBar->value() > before);

    delete w;
}

void VerticalTabsTest::panelHasBottomInset()
{
    BrowserWindow *w = mApp->createWindow(Qz::BW_NewWindow);
    VerticalTabsWidget tabs(w);
    QCOMPARE(tabs.layout()->contentsMargins().bottom(), 8);
    delete w;
}

FALKONTEST_MAIN(VerticalTabsTest)
