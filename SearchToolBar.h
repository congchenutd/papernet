#ifndef SEARCHTOOLBAR_H
#define SEARCHTOOLBAR_H

#include <QToolBar>

class QLineEdit;
class QPushButton;

class SearchToolBar : public QToolBar
{
	Q_OBJECT

public:
	SearchToolBar(QWidget *parent = 0);

public slots:
	void onClear();

signals:
	void search(const QString& target);

private:
	QLineEdit*   leSearch;
	QPushButton* btClear;
};

#endif // SEARCHTOOLBAR_H
