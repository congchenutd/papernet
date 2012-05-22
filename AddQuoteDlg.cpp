#include "AddQuoteDlg.h"
#include "PaperList.h"
#include "Common.h"
#include "MainWindow.h"
#include "NewReferenceDlg.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlQuery>

AddQuoteDlg::AddQuoteDlg(QWidget *parent)
	: QDialog(parent), quoteID(-1)
{
	ui.setupUi(this);
	ui.listView->setModel(&model);

	connect(ui.listView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowChanged()));
	connect(ui.btAdd,            SIGNAL(clicked()), this, SLOT(onAddRef()));
	connect(ui.btDel,            SIGNAL(clicked()), this, SLOT(onDelRef()));
	connect(ui.btSelect,         SIGNAL(clicked()), this, SLOT(onSelectRef()));
	connect(ui.listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onSwitchToPapers()));
}

void AddQuoteDlg::onAddRef()
{
	NewReferenceDlg dlg(this);
	if(dlg.exec() == QDialog::Accepted && !dlg.getTitle().isEmpty())
		addRef(dlg.getTitle());
}

void AddQuoteDlg::onDelRef()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?",
			QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.listView->selectionModel()->selectedRows();

		// sort reversely, s.t. removeRow won't affect the index
		qSort(idxList.begin(), idxList.end(), qGreater<QModelIndex>());
		foreach(QModelIndex idx, idxList)
			model.removeRow(idx.row());
	}
}

void AddQuoteDlg::onCurrentRowChanged()
{
	QModelIndexList idxList = ui.listView->selectionModel()->selectedRows();
	bool valid = !idxList.isEmpty();
	ui.btDel->setEnabled(valid);
}

void AddQuoteDlg::accept()
{
	if(ui.leTitle->text().isEmpty())  // force user to input title
	{
		ui.leTitle->setFocus();
		return;
	}

	QSqlDatabase::database().transaction();

	// update existing quote, or insert a new one
	updateQuote(quoteID, ui.leTitle->text(), ui.teContent->toPlainText());

	QSqlQuery query;
	query.exec(QObject::tr("delete from PaperQuote where Quote = %1").arg(quoteID));

	QStringList list = model.stringList();
	foreach(QString title, list)
	{
		int paperID = getPaperID(title);
		if(paperID == -1)  // the paper not exist, create one
		{
			paperID = getNextID("Papers", "ID");
			::addSimplePaper(paperID, title);
		}
		addPaperQuote(paperID, quoteID);
	}

	QSqlDatabase::database().commit();

	return QDialog::accept();
}

void AddQuoteDlg::addRef(const QString& title)
{
	if(model.stringList().indexOf(title) > -1)  // exists
		return;
	int lastRow = model.rowCount();
	model.insertRow(lastRow);
	model.setData(model.index(lastRow, 0), title);
	model.sort(0);
}

void AddQuoteDlg::setQuoteID(int id)
{
	quoteID = id;

	// load quote
	QSqlQuery query;
	query.exec(tr("select Title, Quote from Quotes where ID = %1").arg(id));
	if(query.next())
	{
		ui.leTitle  ->setText     (query.value(0).toString());
		ui.teContent->setPlainText(query.value(1).toString());
	}

	// load refs
	query.exec(tr("select Paper from PaperQuote where Quote = %1").arg(id));
	while(query.next())
		addRef(getPaperTitle(query.value(0).toInt()));
}

void AddQuoteDlg::onSelectRef()
{
	PaperList dlg(this);
	if(dlg.exec() == QDialog::Accepted)
	{
		QStringList papers = dlg.getSelected();
		foreach(QString paper, papers)
			addRef(paper);
	}
}

// keep the splitter sizes
void AddQuoteDlg::resizeEvent(QResizeEvent*) {
	ui.splitter->setSizes(QList<int>() << height()  * 0.6 << height()  * 0.4);
}

void AddQuoteDlg::onSwitchToPapers()
{
	QModelIndexList idxList = ui.listView->selectionModel()->selectedRows();
	QString paper = idxList.isEmpty() ? QString()
									  : model.data(idxList.front(), Qt::DisplayRole).toString();
	MainWindow::getInstance()->jumpToPaper(paper);   // select the paper
    reject();
}
