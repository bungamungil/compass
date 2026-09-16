/* ============================================================
* VerticalTabs plugin for Falkon
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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
#pragma once

#include "qzcommon.h"
#include "wheelhelper.h"

#include <QWidget>

class BrowserWindow;
class TabListView;
class WebTab;

class QAbstractButton;
class QBoxLayout;
class QFrame;

class FALKON_EXPORT VerticalTabsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VerticalTabsWidget(BrowserWindow *window, QWidget *parent = nullptr);

    void setIconOnly(bool enable);
    bool isIconOnly() const;

    QAbstractButton *searchButton() const;

    void switchToNextTab();
    void switchToPreviousTab();

Q_SIGNALS:
    void searchRequested();
    void iconOnlyChanged(bool iconOnly);

protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    WebTab *nextTab() const;
    WebTab *previousTab() const;

    BrowserWindow *m_window;
    TabListView *m_pinnedView;
    TabListView *m_normalView;
    QAbstractButton *m_collapseButton;
    QAbstractButton *m_searchButton;
    QAbstractButton *m_newTabButton;
    QBoxLayout *m_headerLayout;
    QFrame *m_separator;
    WheelHelper m_wheelHelper;
    bool m_iconOnly = false;
};
