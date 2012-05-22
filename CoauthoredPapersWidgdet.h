#ifndef COAUTHOREDPAPERSWIDGDET_H
#define COAUTHOREDPAPERSWIDGDET_H

#include "ui_CoauthoredPapersWidgdet.h"
#include <QStandardItemModel>

class CoauthoredPapersWidgdet : public QWidget
{
	Q_OBJECT
	
public:
	CoauthoredPapersWidgdet(QWidget *parent = 0);
	void setCentralPaper(int paperID);

protected:
    void showEvent(QShowEvent*);

private:
    void update();
    void updateRecord(int id, const QString& title);

private slots:
	void onPaperDoubleClicked(const QModelIndex& idx);

signals:
	void doubleClicked(int);
	
private:
	Ui::CoauthoredPapersWidgdet ui;
	QStandardItemModel model;
	int centralPaperID;

	enum {COL_ID, COL_TITLE, COL_COAUTHERED};
};

#endif // COAUTHOREDPAPERSWIDGDET_H
