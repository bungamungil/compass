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

#include <QFrame>

class QListView;
class QModelIndex;

class BrowserWindow;
class LineEdit;
class TabSearchModel;

class FALKON_EXPORT TabSearchPopup : public QFrame
{
    Q_OBJECT

public:
    explicit TabSearchPopup(BrowserWindow *window, QWidget *parent = nullptr);

    void showAt(const QPoint &globalTopRight);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private Q_SLOTS:
    void activateIndex(const QModelIndex &index);

private:
    BrowserWindow *m_window;
    LineEdit *m_lineEdit;
    QListView *m_listView;
    TabSearchModel *m_model;
};
