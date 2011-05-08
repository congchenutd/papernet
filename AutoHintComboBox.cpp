#include "AutoHintComboBox.h"
#include <QSortFilterProxyModel>
#include <QKeyEvent>

AutoHintComboBox::AutoHintComboBox(QWidget* parent) : SetTextComboBox(parent)
{
	setEditable(true);
	filterModel = new QSortFilterProxyModel(this);
	setCurrentIndex(-1);

	//setView(new MyListView(this));
}

void AutoHintComboBox::keyPressEvent(QKeyEvent* e)
{
	if(model() == 0)
		return QComboBox::keyPressEvent(e);

	showPopup();
	setFocus();
//	filterModel->setFilterKeyColumn(modelColumn());
//	filterModel->setFilterFixedString(e->text());
	return QComboBox::keyPressEvent(e);
}

void AutoHintComboBox::setModel(QAbstractItemModel* model)
{
	filterModel->setSourceModel(model);
	QComboBox::setModel(filterModel);
}

//MyListView::MyListView(QWidget *parent)
//{
//}

//void MyListView::keyPressEvent(QKeyEvent *event)
//{
//	event->ignore();
//}
