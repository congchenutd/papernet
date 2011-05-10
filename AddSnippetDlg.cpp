#include "AddSnippetDlg.h"
#include "PaperList.h"
#include "Common.h"
#include "MainWindow.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlQuery>

AddSnippetDlg::AddSnippetDlg(QWidget *parent)
	: QDialog(parent), snippetID(-1)
{
	ui.setupUi(this);
	ui.listView->setModel(&model);

	connect(ui.listView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowChanged()));
	connect(ui.btAdd,              SIGNAL(clicked()), this, SLOT(onAddRef()));
	connect(ui.btDel,              SIGNAL(clicked()), this, SLOT(onDelRef()));
	connect(ui.btSelect,           SIGNAL(clicked()), this, SLOT(onSelectRef()));
	connect(ui.btSwitchToPapers,   SIGNAL(clicked()), this, SLOT(onSwitchToPapers()));
	connect(ui.btSwitchToSnippets, SIGNAL(clicked()), this, SLOT(onSwitchToSnippets()));
	connect(ui.listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onSwitchToPapers()));
}

void AddSnippetDlg::onAddRef()
{
	bool ok;
	QString ref = QInputDialog::getText(this, tr("Add Reference"), tr("Reference"), 
							QLineEdit::Normal, QString(""), &ok);
	if(ok && !ref.isEmpty())
		addRef(ref);
}

void AddSnippetDlg::onDelRef()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", 
			QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.listView->selectionModel()->selectedRows();
		
		// sort reversely, s.t. removeRow doesn't damage the index
		qSort(idxList.begin(), idxList.end(), qGreater<QModelIndex>());
		foreach(QModelIndex idx, idxList)
			model.removeRow(idx.row());
	}
}

void AddSnippetDlg::onCurrentRowChanged()
{
	QModelIndexList idxList = ui.listView->selectionModel()->selectedRows();
	bool valid = !idxList.isEmpty();
	ui.btDel->setEnabled(valid);
}

void AddSnippetDlg::accept()
{
	if(ui.teContent->toPlainText().isEmpty())  // force user to input content
	{
		ui.teContent->setFocus();
		return;
	}

	QSqlDatabase::database().transaction();
	updateSnippet(snippetID, ui.leTitle->text(), ui.teContent->toPlainText());

	QSqlQuery query;
	query.exec(QObject::tr("delete from PaperSnippet where Snippet = %1").arg(snippetID));

	QStringList list = model.stringList();
	foreach(QString title, list)
	{
		int paperID = getPaperID(title);
		if(paperID == -1)  // not exist
		{
			paperID = getNextID("Papers", "ID");
			::addSimplePaper(paperID, title);
		}
		addPaperSnippet(paperID, snippetID);
	}

	QSqlDatabase::database().commit();
	
	deleteLater();
	return QDialog::accept();
}

void AddSnippetDlg::addRef(const QString& title)
{
	if(model.stringList().indexOf(title) > -1)  // exists
		return;
	int lastRow = model.rowCount();
	model.insertRow(lastRow);
	model.setData(model.index(lastRow, 0), title);
	model.sort(0);
}

void AddSnippetDlg::setSnippetID(int id)
{
	snippetID = id;

	// load snippet
	QSqlQuery query;
	query.exec(tr("select Title, Snippet from Snippets where ID = %1").arg(id));
	if(query.next())
	{
		ui.leTitle  ->setText     (query.value(0).toString());
		ui.teContent->setPlainText(query.value(1).toString());
	}

	// load refs
	query.exec(tr("select Paper from PaperSnippet where Snippet = %1").arg(id));
	while(query.next())
		addRef(getPaperTitle(query.value(0).toInt()));
}

void AddSnippetDlg::onSelectRef()
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
void AddSnippetDlg::resizeEvent(QResizeEvent*) {
	ui.splitter->setSizes(QList<int>() << height()  * 0.6 << height()  * 0.4);
}

void AddSnippetDlg::onSwitchToPapers()
{
	QModelIndexList idxList = ui.listView->selectionModel()->selectedRows();
	QString paper = idxList.isEmpty() 
				  ? QString() : model.data(idxList.front(), Qt::DisplayRole).toString();
	MainWindow::getInstance()->jumpToPaper(paper);
}

void AddSnippetDlg::onSwitchToSnippets() {
	MainWindow::getInstance()->jumpToSnippet(snippetID);
}

