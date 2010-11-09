#include "SearchToolBar.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

SearchToolBar::SearchToolBar(QWidget *parent)
	: QToolBar(parent)
{
	addWidget(new QLabel(tr(" Search ")));
	addWidget(leSearch = new QLineEdit(this));
	btClear = new QPushButton(QIcon(":/MainWindow/Images/Cancel.png"), QString(), this);
	btClear->setShortcut(QKeySequence(Qt::Key_Escape));
	addWidget(btClear);

	connect(leSearch, SIGNAL(textEdited(QString)), this, SIGNAL(search(QString)));
	connect(btClear,  SIGNAL(clicked()),           this, SLOT(onClear()));
}

void SearchToolBar::onClear()
{
	leSearch->clear();
	emit search(QString());
}
