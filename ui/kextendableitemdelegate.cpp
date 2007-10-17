/* This file is part of the KDE libraries
    Copyright (C) 2006,2007 Andreas Hartmetz (ahartmetz@gmail.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#include "kextendableitemdelegate.h"
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QTreeView>
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>

#include <kdebug.h>

//TODO:listen for removal of rows and columns to clean out dead persistent indexes/editors.
//(not needed ATM, the only user is KShortcutsEditor)

class KExtendableItemDelegatePrivate {
public:
	//this will trigger a lot of auto-casting QModelIndex <-> QPersistentModelIndex
	QHash<QPersistentModelIndex, QWidget *> extenders;
	QHash<QWidget *, QPersistentModelIndex> extenderIndices;
	QPixmap extendIcon;
	QPixmap contractIcon;
	//mostly for quick startup - don't look for extenders while the view
	//is being populated.
	int stateTick;
	bool hasExtenders;
};


KExtendableItemDelegate::KExtendableItemDelegate(QAbstractItemView* parent)
 : QItemDelegate(parent),
   d(new KExtendableItemDelegatePrivate)
{
	d->hasExtenders = false;
	d->stateTick = 0;
	//parent->installEventFilter(this); //not sure if this is good
}


KExtendableItemDelegate::~KExtendableItemDelegate()
{
	delete d;
}


void KExtendableItemDelegate::extendItem(QWidget *ext, const QModelIndex &index)
{
	if (!ext || !index.isValid())
		return;
	//maintain the invariant "zero or one extender per row"
	d->stateTick++;
	contractItem(indexOfExtendedColumnInSameRow(index));
	d->stateTick++;
	//reparent, as promised in the docs
	QAbstractItemView *aiv = qobject_cast<QAbstractItemView *>(parent());
	if (!aiv)
		return;
	ext->setParent(aiv->viewport());
	d->extenders.insert(index, ext);
	d->extenderIndices.insert(ext, index);
	d->hasExtenders = true;
	connect(ext, SIGNAL(destroyed(QObject *)), this, SLOT(extenderDestructionHandler(QObject *)));
	emit extenderCreated(ext, index);
	scheduleUpdateViewLayout();
}


void KExtendableItemDelegate::contractItem(const QModelIndex& index)
{
	QWidget *extender = d->extenders.value(index);
	if (!extender)
		return;

	extender->hide();
	extender->deleteLater();

	scheduleUpdateViewLayout();
}


//slot
void KExtendableItemDelegate::extenderDestructionHandler(QObject *destroyed)
{
	QWidget *extender = static_cast<QWidget *>(destroyed);
	d->stateTick++;

	if(d->extenderIndices.value(extender).isValid()) {
	    if (receivers(SIGNAL(extenderDestroyed(QWidget *, QModelIndex)))) {
		    QPersistentModelIndex persistentIndex = d->extenderIndices.take(extender);
		    QModelIndex index = persistentIndex;
		    emit extenderDestroyed(extender, index);
		    d->extenders.remove(persistentIndex);
	    } else
		    d->extenders.remove(d->extenderIndices.take(extender));

	    if (d->extenders.isEmpty())
		    d->hasExtenders = false;

	    scheduleUpdateViewLayout();
    }
}


bool KExtendableItemDelegate::isExtended(const QModelIndex &index) const {
	return d->extenders.value(index);
}


QSize KExtendableItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QSize ret;

	if (d->hasExtenders)
		ret = maybeExtendedSize(option, index);
	else
		ret = QItemDelegate::sizeHint(option, index);

	bool showExtensionIndicator = index.model()->data(index, ShowExtensionIndicatorRole).toBool();
	if (showExtensionIndicator)
		ret.rwidth() += d->extendIcon.width();

	return ret;
}


void KExtendableItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	int indicatorX = 0;
	int indicatorY = 0;

	QStyleOptionViewItem indicatorOption(option);
	QStyleOptionViewItem itemOption(option);
	bool showExtensionIndicator = index.model()->data(index, ShowExtensionIndicatorRole).toBool();
	
	if (showExtensionIndicator) {
		if (QApplication::isRightToLeft()) {
			indicatorX = option.rect.right() - d->extendIcon.width();
			itemOption.rect.setRight(option.rect.right() - d->extendIcon.width());
			indicatorOption.rect.setLeft(option.rect.right() - d->extendIcon.width());
		}
		else {
			indicatorX = option.rect.left();
			indicatorOption.rect.setRight(option.rect.left() + d->extendIcon.width());
			itemOption.rect.setLeft(option.rect.left() + d->extendIcon.width());
		}
		indicatorY = option.rect.top() + ((option.rect.height() - d->extendIcon.height()) >> 1);
	}
	
	//fast path
	if (!d->hasExtenders) {
		QItemDelegate::paint(painter, itemOption, index);
		if (showExtensionIndicator) {
			QItemDelegate::drawBackground(painter, indicatorOption, index);
			painter->drawPixmap(indicatorX, indicatorY,  d->extendIcon);
		}
		return;
	}

	//indexOfExtendedColumnInSameRow() is very expensive, try to avoid calling it.
	static int cachedStateTick = -1;
	static int cachedRow = -20; //Qt uses -1 for invalid indices
	static QModelIndex cachedParentIndex;
	static QWidget *extender = 0;
	static int extenderHeight;
	int row = index.row();
	QModelIndex parentIndex = index.parent();

	if (row != cachedRow || cachedStateTick != d->stateTick
		|| cachedParentIndex != parentIndex) {
		extender = d->extenders.value(indexOfExtendedColumnInSameRow(index));
		cachedStateTick = d->stateTick;
		cachedRow = row;
		cachedParentIndex = parentIndex;
		if (extender) {
			extenderHeight = extender->sizeHint().height();
		}
	}

	if (!extender) {
		QItemDelegate::paint(painter, itemOption, index);
		if (showExtensionIndicator) {
			QItemDelegate::drawBackground(painter, indicatorOption, index);
			painter->drawPixmap(indicatorX, indicatorY, d->extendIcon);
		}
		return;
	}

	//an extender is present - make two rectangles: one to paint the original item, one for the extender
	if (isExtended(index)) {
		QStyleOptionViewItem extOption(option);
		extOption.rect = extenderRect(extender, option, index);
		updateExtenderGeometry(extender, extOption, index);
		//if we show it before, it will briefly flash in the wrong location.
		//the downside is, of course, that an api user effectively can't hide it.
		extender->show();
	}
	
	indicatorOption.rect.setHeight(option.rect.height() - extenderHeight);
	itemOption.rect.setHeight(option.rect.height() - extenderHeight);
	//tricky:make sure that the modified options' rect really has the
	//same height as the unchanged option.rect if no extender is present
	//(seems to work OK)
	QItemDelegate::paint(painter, itemOption, index);
	
	if (showExtensionIndicator) {
		//indicatorOption's height changed, change this too
		indicatorY = indicatorOption.rect.top() + ((indicatorOption.rect.height() - d->extendIcon.height()) >> 1);
		QItemDelegate::drawBackground(painter, indicatorOption, index);
		
		if (d->extenders.contains(index))
			painter->drawPixmap(indicatorX, indicatorY, d->contractIcon);
		else
			painter->drawPixmap(indicatorX, indicatorY, d->extendIcon);
	}
}


QRect KExtendableItemDelegate::extenderRect(QWidget *extender, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_ASSERT(extender);
	QRect rect(option.rect);
	rect.setTop(rect.bottom() + 1 - extender->sizeHint().height());
	
	rect.setLeft(0);
	QTreeView *tv = qobject_cast<QTreeView *>(parent());
	if (tv)
		for (QModelIndex idx(index.parent()); idx.isValid(); idx = idx.parent())
			rect.translate(tv->indentation(), 0);
	
	QAbstractScrollArea *container = qobject_cast<QAbstractScrollArea *>(parent());
	Q_ASSERT(container);
	rect.setRight(container->viewport()->width() - 1);
	return rect;
}


QSize KExtendableItemDelegate::maybeExtendedSize(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QWidget *extender = d->extenders.value(index);
	QSize size(QItemDelegate::sizeHint(option, index));
	if (!extender)
		return size;

	//add extender height to maximum height of any column in our row
	int itemHeight = size.height();
	
	int row = index.row();
	int thisColumn = index.column();
	QModelIndex parentIndex(index.parent());

	//this is quite slow, but Qt is smart about when to call sizeHint().
	for (int column=0;column<6; column++) {
		if (column == thisColumn)
			continue;

		QModelIndex neighborIndex(index.sibling(row, column));
		if (!neighborIndex.isValid())
			break;
		itemHeight = qMax(itemHeight, QItemDelegate::sizeHint(option, neighborIndex).height());
	}
	
	//we only want to reserve vertical space
	size.rheight() = itemHeight + extender->sizeHint().height();
	return size;
}


QModelIndex KExtendableItemDelegate::indexOfExtendedColumnInSameRow(const QModelIndex &index) const
{
	const QAbstractItemModel *const model = index.model();
	const QModelIndex parentIndex(index.parent());
	const int row = index.row();
	const int columnCount = model->columnCount();

	//slow, slow, slow
	for (int column = 0; column < columnCount; column++) {
		QModelIndex indexOfExt(model->index(row, column, parentIndex));
		if (d->extenders.value(indexOfExt))
			return indexOfExt;
	}

	return QModelIndex();
}


void KExtendableItemDelegate::updateExtenderGeometry(QWidget *extender, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(index);
	extender->setGeometry(option.rect);
}


//make the view re-ask for sizeHint() and redisplay items with their new size
void KExtendableItemDelegate::scheduleUpdateViewLayout() const
{
	QAbstractItemView *aiv = qobject_cast<QAbstractItemView *>(parent());
	//prevent crashes during destruction of the view
	if (aiv)
		//dirty hack to call aiv's protected scheduleDelayedItemsLayout()
		aiv->setRootIndex(aiv->rootIndex());
}


void KExtendableItemDelegate::setExtendIcon(const QPixmap &icon)
{
	d->extendIcon = icon;
}


void KExtendableItemDelegate::setContractIcon(const QPixmap &icon)
{
	d->contractIcon = icon;
}


QPixmap KExtendableItemDelegate::extendIcon()
{
	return d->extendIcon;
}


QPixmap KExtendableItemDelegate::contractIcon()
{
	return d->contractIcon;
}

#include "kextendableitemdelegate.moc"
