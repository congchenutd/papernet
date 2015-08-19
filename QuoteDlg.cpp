#include "QuoteDlg.h"
#include "PaperList.h"
#include "Common.h"
#include "MainWindow.h"
#include "NewReferenceDlg.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlQuery>
#include <QDesktopWidget>

QuoteDlg::QuoteDlg(QWidget *parent)
    : QDialog(parent), _quoteID(-1), _selectedPaperID(-1)
{
	ui.setupUi(this);
    resize(800, 500);

    ui.listView->setModel(&_model);

    // hide goto quotes page button on the quotes page
    ui.btGotoQuotePage->setHidden(
                MainWindow::getInstance()->getCurrentPageIndex() == MainWindow::PAGE_QUOTES);

	connect(ui.listView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(onCurrentRowChanged()));
    connect(ui.btAdd,           SIGNAL(clicked()), this, SLOT(onAddRef()));
    connect(ui.btDel,           SIGNAL(clicked()), this, SLOT(onDelRef()));
    connect(ui.btSelect,        SIGNAL(clicked()), this, SLOT(onSelectRef()));
    connect(ui.btViewPDF,       SIGNAL(clicked()), this, SLOT(onViewPDF()));
    connect(ui.btGotoQuotePage, SIGNAL(clicked()), this, SLOT(onGotoQuotesPage()));
    connect(ui.listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onGotoPapersPage()));
}

void QuoteDlg::onAddRef()
{
    NewReferenceDlg dlg(this);

    centerWindow(&dlg);    // HACK: center a exec()ed dialog
    dlg.show();            //

    if(dlg.exec() == QDialog::Accepted && !dlg.getTitle().isEmpty())
        addRef(::correctCaseInTitle(dlg.getTitle()));
}

void QuoteDlg::onDelRef()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?",
			QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.listView->selectionModel()->selectedRows();

        // sort reversely, s.t. removeRow() won't affect the index
		qSort(idxList.begin(), idxList.end(), qGreater<QModelIndex>());
		foreach(QModelIndex idx, idxList)
            _model.removeRow(idx.row());
	}
}

void QuoteDlg::onCurrentRowChanged()
{
	QModelIndexList idxList = ui.listView->selectionModel()->selectedRows();
	bool valid = !idxList.isEmpty();
	ui.btDel->setEnabled(valid);

    _selectedPaperID = idxList.isEmpty() ? -1
                    : titleToID(_model.data(idxList.front(), Qt::DisplayRole).toString());

    ui.btViewPDF->setEnabled(_selectedPaperID > -1 &&
                             pdfAttached(_selectedPaperID));
}

void QuoteDlg::accept()
{
	if(ui.leTitle->text().isEmpty())  // force user to input title
	{
		ui.leTitle->setFocus();
		return;
	}

	QSqlDatabase::database().transaction();

	// update existing quote, or insert a new one
    updateQuote(_quoteID, ui.leTitle->text(), ui.teContent->toPlainText());

    // update quote-paper relations by deleting all the old and adding new ones
	QSqlQuery query;
    query.exec(QObject::tr("delete from PaperQuote where Quote = %1").arg(_quoteID));

    QStringList list = _model.stringList();
	foreach(QString title, list)
	{
		int paperID = titleToID(title);
		if(paperID == -1)  // the paper not exist, create one
		{
			paperID = getNextID("Papers", "ID");
			::addSimplePaper(paperID, title);
		}
        addPaperQuote(paperID, _quoteID);
	}

	QSqlDatabase::database().commit();

	return QDialog::accept();
}

void QuoteDlg::addRef(const QString& title)
{
    if(_model.stringList().indexOf(title) > -1)  // exists
		return;
    int lastRow = _model.rowCount();
    _model.insertRow(lastRow);
    _model.setData(_model.index(lastRow, 0), title);
    _model.sort(0);
}

void QuoteDlg::setQuoteID(int id)
{
    _quoteID = id;

    // load the quote
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
		addRef(idToTitle(query.value(0).toInt()));
}

void QuoteDlg::onSelectRef()
{
    PaperList dlg(this);

    centerWindow(&dlg);    // HACK: center a exec()ed dialog
    dlg.show();            //

    if(dlg.exec() == QDialog::Accepted)
    {
        QStringList papers = dlg.getSelected();
        foreach(QString paper, papers)
            addRef(paper);
    }
}

void QuoteDlg::onGotoPapersPage()
{
    MainWindow::getInstance()->jumpToPaper(_selectedPaperID);
    accept();
}

void QuoteDlg::onGotoQuotesPage()
{
    MainWindow::getInstance()->jumpToQuote(_quoteID);
    accept();
}

void QuoteDlg::onViewPDF() {
    openAttachment(_selectedPaperID, "Paper.pdf");
}
