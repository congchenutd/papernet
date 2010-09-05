#include "PaperNet.h"
#include "Common.h"
#include <QtSql>

PaperNet::PaperNet(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

int PaperNet::makeKey(int id1, int id2) const {
	return id1 * MAX_PAPER_COUNT + id2;
}

void PaperNet::updateWeights()
{
	QSqlQuery query;
//	query.exec("select ")
}


