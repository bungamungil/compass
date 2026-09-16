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
#include "tablistdelegate.h"
#include "tablistview.h"
#include "loadinganimator.h"

#include "tabmodel.h"
#include "tabicon.h"

#include <QPainter>
#include <QApplication>
#include <QStyleOptionButton>
#include <QCursor>

TabListDelegate::TabListDelegate(TabListView *view)
    : QStyledItemDelegate()
    , m_view(view)
{
    m_padding = qMax(5, m_view->style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1);

    m_loadingAnimator = new LoadingAnimator(this);
    connect(m_loadingAnimator, &LoadingAnimator::updateIndex, m_view, &TabListView::updateIndex);
}

QRect TabListDelegate::audioButtonRect(const QModelIndex &index) const
{
    // In icon-only mode the audio button is not drawn, and its hit-target would
    // overlap the centered favicon - clicking a tab would toggle mute instead.
    if (m_iconOnly) {
        return QRect();
    }
    if (!index.data(TabModel::AudioPlayingRole).toBool() && !index.data(TabModel::AudioMutedRole).toBool()) {
        return QRect();
    }
    const QRect rect = m_view->visualRect(index);
    const int center = rect.height() / 2 + rect.top();
    return QRect(rect.right() - 16, center - 16 / 2, 16, 16);
}

QRect TabListDelegate::closeButtonRect(const QModelIndex &index) const
{
    const QRect rect = m_view->visualRect(index);
    if (!rect.isValid()) {
        return QRect();
    }
    const QStyle *style = m_view->style();
    const int w = qMax(16, style->pixelMetric(QStyle::PM_TabCloseIndicatorWidth, nullptr, m_view));
    const int h = qMax(16, style->pixelMetric(QStyle::PM_TabCloseIndicatorHeight, nullptr, m_view));
    const int center = rect.height() / 2 + rect.top();
    if (m_iconOnly) {
        if (!index.data(TabModel::CurrentTabRole).toBool()) {
            return QRect();
        }
        return QRect(rect.center().x() - w / 2, center - h / 2, w, h);
    }
    return QRect(rect.right() - m_padding - w, center - h / 2, w, h);
}

void TabListDelegate::drawCloseButton(QPainter *painter, const QRect &rect) const
{
    const QStyle *style = m_view->style();
    QStyleOptionButton o;
    o.initFrom(m_view->viewport());
    o.rect = rect;
    const bool cursorOver = rect.contains(m_view->viewport()->mapFromGlobal(QCursor::pos()));
    const bool pressed = cursorOver && QApplication::mouseButtons() == Qt::LeftButton;
    o.state = QStyle::State_AutoRaise | QStyle::State_Enabled;
    o.state.setFlag(QStyle::State_Raised, cursorOver && !pressed);
    o.state.setFlag(QStyle::State_Sunken, pressed);
    o.state.setFlag(QStyle::State_MouseOver, cursorOver);
    style->drawPrimitive(QStyle::PE_IndicatorTabClose, &o, painter, nullptr);
}

void TabListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QWidget *w = option.widget;
    const QStyle *style = w ? w->style() : m_view->style();

    const bool isRestoredTab = index.data(TabModel::RestoredRole).toBool();

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    m_view->adjustStyleOption(&opt);

    const int height = opt.rect.height();
    const int center = height / 2 + opt.rect.top();

    int leftPosition = opt.rect.left() + m_padding;
    int rightPosition = opt.rect.right() - m_padding * 2;

    const QPalette::ColorRole colorRole = opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text;

    QPalette::ColorGroup cg = opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
    if (!isRestoredTab) {
        cg = QPalette::Disabled;
    }
    if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active)) {
        cg = QPalette::Inactive;
    }

#ifdef Q_OS_WIN
    opt.palette.setColor(QPalette::All, QPalette::HighlightedText, opt.palette.color(QPalette::Active, QPalette::Text));
    opt.palette.setColor(QPalette::All, QPalette::Highlight, opt.palette.base().color().darker(108));
#endif

    QPalette textPalette = opt.palette;
    textPalette.setCurrentColorGroup(cg);

    // Draw background
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, w);

    if (m_iconOnly) {
        if ((opt.state & QStyle::State_MouseOver) && (opt.state & QStyle::State_Selected)) {
            drawCloseButton(painter, closeButtonRect(index));
            return;
        }
        const int iconSize = 16;
        const QRect iconRect(opt.rect.center().x() - iconSize / 2, center - iconSize / 2, iconSize, iconSize);
        QPixmap pixmap;
        if (index.data(TabModel::LoadingRole).toBool()) {
            pixmap = m_loadingAnimator->pixmap(index);
        } else {
            pixmap = index.data(Qt::DecorationRole).value<QIcon>().pixmap(iconSize);
        }
        painter->drawPixmap(iconRect, pixmap);
        return;
    }

    // Draw icon
    const int iconSize = 16;
    const int iconYPos = center - (iconSize / 2);
    QRect iconRect(leftPosition, iconYPos, iconSize, iconSize);
    QPixmap pixmap;
    if (index.data(TabModel::LoadingRole).toBool()) {
        pixmap = m_loadingAnimator->pixmap(index);
    } else {
        pixmap = index.data(Qt::DecorationRole).value<QIcon>().pixmap(iconSize);
    }
    painter->drawPixmap(iconRect, pixmap);
    leftPosition += iconRect.width() + m_padding;

    // Reserve the close-button column so the title does not re-elide on hover.
    const int closeW = qMax(16, style->pixelMetric(QStyle::PM_TabCloseIndicatorWidth, nullptr, w));
    rightPosition -= closeW + m_padding;

    // Draw audio icon
    const bool audioMuted = index.data(TabModel::AudioMutedRole).toBool();
    const bool audioPlaying = index.data(TabModel::AudioPlayingRole).toBool();
    if (audioMuted || audioPlaying) {
        QSize audioSize(16, 16);
        QPoint pos(rightPosition - audioSize.width(), center - audioSize.height() / 2);
        QRect audioRect(pos, audioSize);
        painter->drawPixmap(audioRect, audioMuted ? TabIcon::data()->audioMutedPixmap : TabIcon::data()->audioPlayingPixmap);
        rightPosition -= audioSize.width() + m_padding;
    }

    // Draw title
    QRect titleRect(leftPosition, center - opt.fontMetrics.height() / 2, opt.rect.width(), opt.fontMetrics.height());
    titleRect.setRight(rightPosition - m_padding);
    QString title = opt.fontMetrics.elidedText(index.data().toString(), Qt::ElideRight, titleRect.width());
    style->drawItemText(painter, titleRect, Qt::AlignLeft, textPalette, true, title, colorRole);

    // Draw the close indicator on hover or on the current tab.
    if (opt.state & (QStyle::State_MouseOver | QStyle::State_Selected)) {
        drawCloseButton(painter, closeButtonRect(index));
    }
}

QSize TabListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    if (m_iconOnly) {
        return QSize(IconOnlyCell, IconOnlyCell);
    }

    return QSize(m_padding * 4 + 16, qMax(IconOnlyCell, m_padding * 2 + opt.fontMetrics.height()));
}
