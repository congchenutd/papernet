#ifndef QUOTESWIDGET_H
#define QUOTESWIDGET_H

#include "ui_QuotesWidget.h"
#include <QSqlQueryModel>

class QuotesWidget : public QWidget
{
    Q_OBJECT
    
public:
    QuotesWidget(QWidget* parent = 0);
    void setCentralPaper(int paperID);

protected:
    void showEvent(QShowEvent*);

private slots:
	void onQuoteDoubleClicked(const QModelIndex& idx);

private:
    void update();

signals:
    void quotesChanged();
    
private:
    Ui::QuotesWidget ui;
    int            _paperID;
    QSqlQueryModel _model;

	enum {COL_ID, COL_TITLE, COL_QUOTE};
};

#endif // QUOTESWIDGET_H
