#ifndef AUTOHINTCOMBOBOX_H
#define AUTOHINTCOMBOBOX_H

#include "../MyComboBox/SetTextComboBox.h"
#include <QListView>

class QSortFilterProxyModel;

class AutoHintComboBox : public SetTextComboBox
{
	Q_OBJECT

public:
	AutoHintComboBox(QWidget* parent = 0);
	void setModel(QAbstractItemModel* model);

protected:
	virtual void keyPressEvent(QKeyEvent* e);

private:
	QSortFilterProxyModel* filterModel;
};

//class MyListView : public QListView
//{
//public:
//	MyListView(QWidget* parent = 0);

//protected:
//	virtual void keyPressEvent(QKeyEvent* event);
//};

#endif // AUTOHINTCOMBOBOX_H
