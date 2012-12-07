#ifndef COAUTHOREDPAPERSWIDGDET_H
#define COAUTHOREDPAPERSWIDGDET_H

#include "ui_CoauthoredPapersWidgdet.h"
#include <QStandardItemModel>

// widget for showing coauthored papers
class CoauthoredPapersWidgdet : public QWidget
{
	Q_OBJECT
	
public:
	CoauthoredPapersWidgdet(QWidget *parent = 0);
	void setCentralPaper(int paperID);
    void saveGeometry();

protected:
    void showEvent(QShowEvent*);

private:
    void update();    // update all the records

    // update the coauthor record for the paper with paperID and paperTitle
    void updateCoauthorRecord(int paperID, const QString& paperTitle,
                              const QString& authors, int year);

private slots:
	void onPaperDoubleClicked(const QModelIndex& idx);

signals:
    void doubleClicked(int paperID);
	
private:
	Ui::CoauthoredPapersWidgdet ui;
    QStandardItemModel _model;
    int                _centralPaperID;

    enum {COL_ID, COL_TITLE, COL_COAUTHERED, COL_AUTHORS, COL_YEAR};
};

#endif // COAUTHOREDPAPERSWIDGDET_H
