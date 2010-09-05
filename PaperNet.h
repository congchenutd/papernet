#ifndef PAPERNET_H
#define PAPERNET_H

#include <QWidget>
#include <map>
#include "ui_PaperNet.h"

class PaperNet : public QWidget
{
	Q_OBJECT

public:
	PaperNet(QWidget *parent = 0);

private:
	int makeKey(int id1, int id2) const;
	void updateWeights();

private:
	Ui::PaperNetClass ui;
	std::map<int, int> weights;
};

#endif // PAPERNET_H
