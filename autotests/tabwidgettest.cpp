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
#include "tabwidgettest.h"
#include "autotests.h"
#include "browserwindow.h"
#include "tabbar.h"
#include "tabwidget.h"

#include <QApplication>
#include <QEvent>

static QList<AddTabButton*> addTabButtons(TabWidget *tabs)
{
    return tabs->tabBar()->findChildren<AddTabButton*>();
}

void TabWidgetTest::addTabButtonsAreCompact()
{
    BrowserWindow *w = mApp->createWindow(Qz::BW_NewWindow);
    const QList<AddTabButton*> buttons = addTabButtons(w->tabWidget());
    QCOMPARE(buttons.size(), 2);
    for (AddTabButton *button : buttons) {
        QCOMPARE(button->toolButtonStyle(), Qt::ToolButtonIconOnly);
        QCOMPARE(button->width(), button->height());
    }
    delete w;
}

void TabWidgetTest::addTabButtonsStayCompactAfterStyleChange()
{
    BrowserWindow *w = mApp->createWindow(Qz::BW_NewWindow);
    const QList<AddTabButton*> buttons = addTabButtons(w->tabWidget());
    QCOMPARE(buttons.size(), 2);
    for (AddTabButton *button : buttons) {
        QEvent event(QEvent::StyleChange);
        QApplication::sendEvent(button, &event);
        QCOMPARE(button->width(), button->height());
    }
    delete w;
}

void TabWidgetTest::inlineButtonStaysInBoundsInRightToLeftMode()
{
    const Qt::LayoutDirection oldDirection = QApplication::layoutDirection();
    QApplication::setLayoutDirection(Qt::RightToLeft);
    BrowserWindow *w = mApp->createWindow(Qz::BW_NewWindow);
    w->resize(900, 600);
    w->show();
    QCoreApplication::processEvents();

    AddTabButton *button = w->tabWidget()->buttonAddTab();
    w->tabWidget()->moveAddTabButton(120);
    QVERIFY(w->tabWidget()->tabBar()->rect().contains(button->geometry()));

    delete w;
    QApplication::setLayoutDirection(oldDirection);
}

FALKONTEST_MAIN(TabWidgetTest)
