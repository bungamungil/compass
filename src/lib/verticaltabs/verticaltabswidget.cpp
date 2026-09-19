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
#include "verticaltabswidget.h"
#include "tablistview.h"
#include "tabfiltermodel.h"
#include "tablistdelegate.h"

#include "webtab.h"
#include "tabmodel.h"
#include "tabwidget.h"
#include "toolbutton.h"
#include "browserwindow.h"
#include "iconprovider.h"
#include "qzsettings.h"

#include <QIcon>
#include <QUrl>
#include <QScrollBar>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QWheelEvent>

static constexpr int PanelHMargin = 8;

VerticalTabsWidget::VerticalTabsWidget(BrowserWindow *window, QWidget *parent)
    : QWidget(parent)
    , m_window(window)
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(PanelHMargin, 0, PanelHMargin, PanelHMargin);

    auto *collapseButton = new ToolButton(this);
    collapseButton->setObjectName(QSL("verticaltabs-button-collapse"));
    collapseButton->setAutoRaise(true);
    collapseButton->setFocusPolicy(Qt::NoFocus);
    collapseButton->setToolTip(tr("Collapse tab panel"));
    collapseButton->setIcon(QIcon::fromTheme(QSL("sidebar-collapse"), QIcon::fromTheme(QSL("go-previous"))));
    m_collapseButton = collapseButton;
    connect(collapseButton, &QAbstractButton::clicked, this, [this]() {
        setIconOnly(!m_iconOnly);
    });

    auto *searchButton = new ToolButton(this);
    searchButton->setObjectName(QSL("verticaltabs-button-search"));
    searchButton->setAutoRaise(true);
    searchButton->setFocusPolicy(Qt::NoFocus);
    searchButton->setToolTip(tr("Search Tabs"));
    QIcon searchIcon = QIcon::fromTheme(QSL("search"));
    if (searchIcon.isNull()) {
        searchIcon = QIcon(QSL(":/icons/menu/search-icon.svg"));
    }
    searchButton->setIcon(searchIcon);
    m_searchButton = searchButton;
    connect(searchButton, &QAbstractButton::clicked, this, &VerticalTabsWidget::searchRequested);

    m_pinnedView = new TabListView(m_window, this);
    auto *pinnedModel = new TabFilterModel(m_pinnedView);
    pinnedModel->setFilterPinnedTabs(false);
    pinnedModel->setRejectDropOnLastIndex(true);
    pinnedModel->setSourceModel(m_window->tabModel());
    m_pinnedView->setModel(pinnedModel);
    m_pinnedView->setHideWhenEmpty(true);

    m_normalView = new TabListView(m_window, this);
    m_normalView->setAutoHeight(false);
    auto *normalModel = new TabFilterModel(m_normalView);
    normalModel->setFilterPinnedTabs(true);
    normalModel->setSourceModel(m_window->tabModel());
    m_normalView->setModel(normalModel);
    m_pinnedView->setFocusProxy(m_normalView);

    auto *newTabButton = new ToolButton(this);
    newTabButton->setObjectName(QSL("verticaltabs-button-addtab"));
    newTabButton->setAutoRaise(true);
    newTabButton->setFocusPolicy(Qt::NoFocus);
    newTabButton->setToolTip(tr("New Tab"));
    newTabButton->setIcon(IconProvider::newTabIcon());
    newTabButton->setFixedSize(TabListDelegate::IconOnlyCell, TabListDelegate::IconOnlyCell);
    connect(newTabButton, &QAbstractButton::clicked, this, [this]() {
        m_window->tabWidget()->addView(QUrl(), Qz::NT_SelectedNewEmptyTab);
    });
    m_newTabButton = newTabButton;

    m_headerLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    m_headerLayout->addWidget(m_collapseButton);
    m_headerLayout->addStretch();
    m_headerLayout->addWidget(m_searchButton);

    m_separator = new QFrame(this);
    m_separator->setObjectName(QSL("verticaltabs-separator"));
    m_separator->setFrameShape(QFrame::HLine);
    m_separator->setFrameShadow(QFrame::Sunken);

    layout->addLayout(m_headerLayout);
    layout->addWidget(m_separator);
    layout->addWidget(m_pinnedView);
    layout->addWidget(m_normalView);
    layout->addSpacing(4);
    layout->addWidget(m_newTabButton);
    layout->addStretch(1);
}

void VerticalTabsWidget::setIconOnly(bool enable)
{
    m_iconOnly = enable;
    m_pinnedView->setIconOnly(enable);
    m_normalView->setIconOnly(enable);

    if (enable) {
        setFixedWidth(TabListDelegate::IconOnlyCell + 2 * PanelHMargin);
    } else {
        setMinimumWidth(ExpandedMinWidth);
        setMaximumWidth(QWIDGETSIZE_MAX);
    }

    m_collapseButton->setIcon(enable ? QIcon::fromTheme(QSL("sidebar-expand"), QIcon::fromTheme(QSL("go-next")))
                                      : QIcon::fromTheme(QSL("sidebar-collapse"), QIcon::fromTheme(QSL("go-previous"))));
    m_collapseButton->setToolTip(enable ? tr("Expand tab panel") : tr("Collapse tab panel"));

    const int cell = TabListDelegate::IconOnlyCell;
    // Collapse, search and "+" are all square/cell-height in both modes, matching
    // the tab row height (Chrome-like uniform cell size).
    m_collapseButton->setFixedSize(cell, cell);
    m_searchButton->setFixedSize(cell, cell);
    if (enable) {
        m_headerLayout->setDirection(QBoxLayout::TopToBottom);
        // remove the middle stretch: rebuild as [collapse, search] centred
        while (m_headerLayout->count() > 0) {
            QLayoutItem *item = m_headerLayout->takeAt(0);
            delete item; // items are stretches or widget-items; widgets survive
        }
        m_headerLayout->addWidget(m_collapseButton, 0, Qt::AlignHCenter);
        m_headerLayout->addWidget(m_searchButton, 0, Qt::AlignHCenter);

        m_newTabButton->setFixedSize(cell, cell);
        static_cast<ToolButton*>(m_newTabButton)->setToolButtonStyle(Qt::ToolButtonIconOnly);
        m_newTabButton->setText(QString());
    } else {
        m_headerLayout->setDirection(QBoxLayout::LeftToRight);
        while (m_headerLayout->count() > 0) {
            QLayoutItem *item = m_headerLayout->takeAt(0);
            delete item;
        }
        m_headerLayout->addWidget(m_collapseButton);
        m_headerLayout->addStretch();
        m_headerLayout->addWidget(m_searchButton);

        auto *tb = static_cast<ToolButton*>(m_newTabButton);
        tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        tb->setText(tr("New Tab"));
        m_newTabButton->setMinimumSize(0, 0);
        m_newTabButton->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        m_newTabButton->setFixedHeight(cell);
        m_newTabButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    qzSettings->verticalTabsIconOnly = enable;
    qzSettings->saveSettings();

    Q_EMIT iconOnlyChanged(enable);
}

bool VerticalTabsWidget::isIconOnly() const
{
    return m_iconOnly;
}

QAbstractButton *VerticalTabsWidget::searchButton() const
{
    return m_searchButton;
}

void VerticalTabsWidget::switchToNextTab()
{
    WebTab *tab = nextTab();
    if (tab) {
        tab->makeCurrentTab();
    }
}

void VerticalTabsWidget::switchToPreviousTab()
{
    WebTab *tab = previousTab();
    if (tab) {
        tab->makeCurrentTab();
    }
}

WebTab *VerticalTabsWidget::nextTab() const
{
    QModelIndex next;
    if (m_window->tabWidget()->webTab()->isPinned()) {
        next = m_pinnedView->indexAfter(m_pinnedView->currentIndex());
        if (!next.isValid()) {
            next = m_normalView->model()->index(0, 0);
        }
    } else {
        next = m_normalView->indexAfter(m_normalView->currentIndex());
        if (!next.isValid()) {
            next = m_pinnedView->model()->index(0, 0);
        }
    }
    return next.data(TabModel::WebTabRole).value<WebTab*>();
}

WebTab *VerticalTabsWidget::previousTab() const
{
    QModelIndex previous;
    if (m_window->tabWidget()->webTab()->isPinned()) {
        previous = m_pinnedView->indexBefore(m_pinnedView->currentIndex());
        if (!previous.isValid()) {
            previous = m_normalView->model()->index(m_normalView->model()->rowCount() - 1, 0);
        }
    } else {
        previous = m_normalView->indexBefore(m_normalView->currentIndex());
        if (!previous.isValid()) {
            previous = m_pinnedView->model()->index(m_pinnedView->model()->rowCount() - 1, 0);
        }
    }
    return previous.data(TabModel::WebTabRole).value<WebTab*>();
}

void VerticalTabsWidget::wheelEvent(QWheelEvent *event)
{
    QScrollBar *scrollBar = m_normalView->verticalScrollBar();
    const bool listOverflows = scrollBar->maximum() > scrollBar->minimum();
    if (!qzSettings->alwaysSwitchTabsWithWheel && listOverflows) {
        event->ignore();
        return;
    }

    m_wheelHelper.processEvent(event);
    while (WheelHelper::Direction direction = m_wheelHelper.takeDirection()) {
        switch (direction) {
        case WheelHelper::WheelUp:
        case WheelHelper::WheelLeft:
            switchToPreviousTab();
            break;

        case WheelHelper::WheelDown:
        case WheelHelper::WheelRight:
            switchToNextTab();
            break;

        default:
            break;
        }
    }
    event->accept();
}
