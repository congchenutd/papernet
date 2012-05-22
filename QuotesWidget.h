#ifndef QUOTESWIDGET_H
#define QUOTESWIDGET_H

#include "ui_QuotesWidget.h"

class QuotesWidget : public QWidget
{
    Q_OBJECT
    
public:
    QuotesWidget(QWidget* parent = 0);
    void setCentralPaper(int paperID);

protected:
    void showEvent(QShowEvent*);

private:
    void update();

signals:
    void doubleClicked(int);
    
private:
    Ui::QuotesWidget ui;
    int centralPaperID;
};

#endif // QUOTESWIDGET_H
