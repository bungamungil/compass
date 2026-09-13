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
#include "tabsearchpopup.h"
#include "tabsearchmodel.h"

#include "browserwindow.h"
#include "tabwidget.h"
#include "webtab.h"
#include "lineedit.h"

#include <QAction>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QListView>
#include <QPainter>
#include <QPushButton>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

namespace {

// Renders each row as an icon + two text lines (title, url), and paints a
// small section label ("Open Tabs" / "Recently Closed") above the first row
// of each new section. Every row stays a real, activatable model entry -
// the section labels are pure paint-time decoration, not extra rows.
class TabSearchDelegate : public QStyledItemDelegate
{
public:
    explicit TabSearchDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    static const int IconSize = 16;
    static const int Margin = 6;
    static const int SectionHeaderHeight = 22;
    static const int RowHeight = 40;

    bool isSectionStart(const QModelIndex &index) const
    {
        if (index.row() == 0) {
            return true;
        }
        const QModelIndex previous = index.sibling(index.row() - 1, index.column());
        return previous.data(TabSearchModel::SectionRole) != index.data(TabSearchModel::SectionRole);
    }

    QString sectionLabel(const QModelIndex &index) const
    {
        const int section = index.data(TabSearchModel::SectionRole).toInt();
        if (section == TabSearchModel::RecentlyClosed) {
            return TabSearchPopup::tr("Recently Closed");
        }
        return TabSearchPopup::tr("Open Tabs");
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        Q_UNUSED(option)
        int height = RowHeight;
        if (isSectionStart(index)) {
            height += SectionHeaderHeight;
        }
        return QSize(200, height);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        painter->save();

        QRect rect = option.rect;

        if (isSectionStart(index)) {
            QRect headerRect(rect.left(), rect.top(), rect.width(), SectionHeaderHeight);
            painter->fillRect(headerRect, option.palette.alternateBase());
            QFont headerFont = option.font;
            headerFont.setBold(true);
            headerFont.setPointSizeF(headerFont.pointSizeF() * 0.9);
            painter->setFont(headerFont);
            painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
            painter->drawText(headerRect.adjusted(Margin, 0, -Margin, 0), Qt::AlignVCenter | Qt::AlignLeft, sectionLabel(index));
            rect.setTop(rect.top() + SectionHeaderHeight);
        }

        if (option.state & QStyle::State_Selected) {
            painter->fillRect(rect, option.palette.highlight());
        }

        const QIcon icon = index.data(TabSearchModel::IconRole).value<QIcon>();
        const QRect iconRect(rect.left() + Margin, rect.top() + (rect.height() - IconSize) / 2, IconSize, IconSize);
        if (!icon.isNull()) {
            icon.paint(painter, iconRect);
        }

        const int textLeft = iconRect.right() + Margin;
        const QRect textRect(textLeft, rect.top(), rect.right() - textLeft - Margin, rect.height());

        const QString title = index.data(TabSearchModel::TitleRole).toString();
        const QString url = index.data(TabSearchModel::UrlRole).toUrl().toString();

        const QColor textColor = (option.state & QStyle::State_Selected)
            ? option.palette.color(QPalette::HighlightedText)
            : option.palette.color(QPalette::Text);
        const QColor mutedColor = (option.state & QStyle::State_Selected)
            ? option.palette.color(QPalette::HighlightedText)
            : option.palette.color(QPalette::Disabled, QPalette::WindowText);

        QFont titleFont = option.font;
        QFontMetrics titleMetrics(titleFont);
        painter->setFont(titleFont);
        painter->setPen(textColor);
        const QRect titleRect(textRect.left(), textRect.top() + 4, textRect.width(), titleMetrics.height());
        painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, titleMetrics.elidedText(title, Qt::ElideRight, titleRect.width()));

        QFont urlFont = option.font;
        urlFont.setPointSizeF(urlFont.pointSizeF() * 0.85);
        QFontMetrics urlMetrics(urlFont);
        painter->setFont(urlFont);
        painter->setPen(mutedColor);
        const QRect urlRect(textRect.left(), titleRect.bottom(), textRect.width(), urlMetrics.height());
        painter->drawText(urlRect, Qt::AlignLeft | Qt::AlignVCenter, urlMetrics.elidedText(url, Qt::ElideRight, urlRect.width()));

        painter->restore();
    }
};

}

TabSearchPopup::TabSearchPopup(BrowserWindow *window, QWidget *parent)
    : QFrame(parent)
    , m_window(window)
{
    setWindowFlags(Qt::Popup);
    setFrameShape(QFrame::StyledPanel);
    resize(360, 400);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    m_lineEdit = new LineEdit(this);
    m_lineEdit->setPlaceholderText(tr("Search tabs..."));

    auto *closeButton = new QPushButton(m_lineEdit);
    closeButton->setFlat(true);
    closeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    m_lineEdit->addWidget(closeButton, LineEdit::RightSide);
    connect(closeButton, &QAbstractButton::clicked, this, &TabSearchPopup::close);

    m_model = new TabSearchModel(window, this);

    m_listView = new QListView(this);
    m_listView->setModel(m_model);
    m_listView->setItemDelegate(new TabSearchDelegate(m_listView));
    m_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listView->setUniformItemSizes(false);

    layout->addWidget(m_lineEdit);
    layout->addWidget(m_listView);

    connect(m_lineEdit, &QLineEdit::textChanged, m_model, &TabSearchModel::setFilter);
    connect(m_listView, &QListView::activated, this, &TabSearchPopup::activateIndex);
    connect(m_listView, &QListView::clicked, this, &TabSearchPopup::activateIndex);

    // Keep a valid current row at all times so Enter always has a target.
    // TabSearchModel resets itself on every filter change.
    auto selectFirstRow = [this]() {
        m_listView->setCurrentIndex(m_model->rowCount() > 0 ? m_model->index(0, 0) : QModelIndex());
    };
    connect(m_model, &QAbstractItemModel::modelReset, this, selectFirstRow);
    selectFirstRow();
}

void TabSearchPopup::activateIndex(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    auto *webTab = index.data(TabSearchModel::WebTabRole).value<WebTab*>();
    if (webTab) {
        webTab->makeCurrentTab();
        close();
        return;
    }

    const int closedIndex = index.data(TabSearchModel::ClosedIndexRole).toInt();
    if (closedIndex >= 0 && m_window && m_window->tabWidget()) {
        QAction action;
        action.setData(closedIndex);
        m_window->tabWidget()->restoreClosedTab(&action);
        close();
    }
}

void TabSearchPopup::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        close();
        return;
    case Qt::Key_Down:
    case Qt::Key_Up:
    case Qt::Key_PageDown:
    case Qt::Key_PageUp:
        QCoreApplication::sendEvent(m_listView, event);
        return;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        activateIndex(m_listView->currentIndex());
        return;
    default:
        break;
    }
    QFrame::keyPressEvent(event);
}

void TabSearchPopup::showAt(const QPoint &globalTopRight)
{
    move(globalTopRight.x() - width(), globalTopRight.y());
    show();
    m_lineEdit->setFocus();
}
