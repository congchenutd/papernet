#include "PagePapers.h"
#include "Common.h"
#include "PaperDlg.h"
#include "QuoteDlg.h"
#include "Navigator.h"
#include "TagDlg.h"
#include "MainWindow.h"
#include "RefFormatSpec.h"
#include "RefExporter.h"
#include "RefParser.h"
#include "PaperList.h"
#include "OptionDlg.h"
#include "WebImporter.h"
#include "Reference.h"
#include "PaperWidgetMapper.h"
#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QFileDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QFileInfo>
#include <QProgressDialog>
#include <QClipboard>
#include <QProcess>
#include <QDate>

PagePapers::PagePapers(QWidget *parent)
	: Page(parent)
{
	_currentRow = -1;
	_currentID  = -1;
	_setting    = UserSetting::getInstance();

	ui.setupUi(this);

	ui.tvPapers->init("PagePapers", _setting);   // set table name for the view
	resetModel();

	_mapper = new PaperWidgetMapper(this);
	_mapper->setModel(&_model);
	_mapper->addMapping(ui.teAbstract, PAPER_ABSTRACT);
	_mapper->addMapping(ui.teNote,     PAPER_NOTE);
	_mapper->setAttachmentWidget(ui.widgetAttachments);
	_mapper->setRelatedWidget   (ui.widgetRelated);
	_mapper->setCoauthoredWidget(ui.widgetCoauthered);
	_mapper->setQuotesWidget    (ui.widgetQuotes);
	_mapper->setTagsWidget      (ui.widgetTags);

	ui.tvPapers->setModel(&_model);
	ui.tvPapers->hideColumn(PAPER_ID);
	for(int col = PAPER_TYPE; col <= PAPER_NOTE; ++col)
		ui.tvPapers->hideColumn(col);

	ui.tvPapers->resizeColumnToContents(PAPER_TITLE);

	// tag table, relation table, foreign key in the relation table
	ui.widgetTags->setTableNames("Tags", "PaperTag", "Paper");

	loadGeometry();

	connect(ui.tvPapers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
		_mapper, SLOT(setCurrentModelIndex(QModelIndex)));
	connect(ui.tvPapers->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
		this, SLOT(onSelectionChanged(QItemSelection)));

	connect(ui.tvPapers, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEditPaper()));
	connect(ui.tvPapers, SIGNAL(clicked(QModelIndex)),       this, SLOT(onClicked()));
	connect(ui.tvPapers, SIGNAL(addQuote()),     this, SLOT(onAddQuote()));
	connect(ui.tvPapers, SIGNAL(printMe(bool)),  this, SLOT(onPrintMe(bool)));
	connect(ui.tvPapers, SIGNAL(bookmark(bool)), this, SLOT(onBookmark(bool)));
	connect(ui.tvPapers, SIGNAL(addPDF()),       this, SLOT(onAddPDF()));
	connect(ui.tvPapers, SIGNAL(readPDF()),      this, SLOT(onReadPDF()));

	connect(ui.widgetTags, SIGNAL(filter(bool)), this, SLOT(onFilterPapersByTags(bool)));
	connect(ui.widgetTags, SIGNAL(newTag()),     this, SLOT(onNewTag()));
	connect(ui.widgetTags, SIGNAL(addTag()),     this, SLOT(onAddTagToPaper()));
	connect(ui.widgetTags, SIGNAL(removeTag()),  this, SLOT(onDelTagFromPaper()));
	connect(ui.widgetTags, SIGNAL(doubleClicked(QString)), this, SLOT(onTagDoubleClicked(QString)));

	connect(ui.widgetRelated,    SIGNAL(doubleClicked(int)), this, SLOT(onRelatedDoubleClicked(int)));
	connect(ui.widgetCoauthered, SIGNAL(doubleClicked(int)), this, SLOT(onRelatedDoubleClicked(int)));
}

void PagePapers::onSelectionChanged(const QItemSelection& selected)
{
	if(!selected.isEmpty())
	{
		_currentRow = selected.indexes().front().row();
		_currentID  = rowToID(_currentRow);
		emit hasPDF(pdfAttached(_currentID));    // let MainWindow update actionPDF
	}
	emit selectionValid(!selected.isEmpty());
}

// only triggered by mouse click, not programmatically
void PagePapers::onClicked() {
	Navigator::getInstance()->addFootStep(this, _currentID);
}

void PagePapers::addRecord()
{
	PaperDlg dlg(this);
	dlg.setWindowTitle(tr("Add Reference"));
	if(dlg.exec() == QDialog::Accepted)
		insertReference(dlg.getReference());
}

void PagePapers::onEditPaper()
{
	PaperDlg dlg(this);
	dlg.setWindowTitle(tr("Edit Reference"));
	Reference oldRef = exportRef(_currentID);
	dlg.setReference(oldRef);

	if(dlg.exec() == QDialog::Accepted)
	{
		Reference newRef = dlg.getReference();
		updateRefByID(_currentID, newRef);   // apply the change

		// renaming title affects attachments
		QString oldTitle = oldRef.getValue("title").toString();
		QString newTitle = newRef.getValue("title").toString();
		if(oldTitle != newTitle)
			::renameTitle(oldTitle, newTitle);

        // make it read if note was changed and pdf attached
        int id = titleToID(newRef.getValue("title").toString());
        if(newRef.getValue("note") != oldRef.getValue("note") && pdfAttached(id))
            setPaperRead();

        // show the newly inserted
        reset();
		jumpToCurrent();
	}
}

void PagePapers::insertReference(const Reference& ref)
{
	int id = titleToID(ref.getValue("title").toString());
	if(id > -1)     // replace existing paper
	{
		_currentID = id;
		updateRefByID(_currentID, ref);
	}
	else            // insert as a new one
	{
		// FIXME: rowCount() may not return the actual number
		int lastRow = _model.rowCount();
		_model.insertRow(lastRow);

		// create a new ID and a new row
		_currentID = getNextID("Papers", "ID");
		_model.setData(_model.index(lastRow, PAPER_ID), _currentID);
		submit();    // submit s.t. later code can find the row and ID

		updateRefByRow(lastRow, ref);
		onBookmark(true);    // attach the ReadMe tag
	}

    // show the newly inserted
    _model.select();
    fetchAll(&_model);
    ui.tvPapers->reSort();
    jumpToCurrent();
}

// works when the row exists and is visible
// does not call reset to preserve possible filtering
// submit() will keep the highlighting
void PagePapers::updateRefByRow(int row, const Reference& r)
{
	if(row < 0 || row > _model.rowCount())
		return;

	Reference ref = r;
	ref.touch();   // add modified date

	// pull data from ref based on the fields in table,
	// because ref may contain extra fields not in table, such as "pdf" and "tags"
	QSqlRecord record = _model.record(row);
	for(int col = 0; col < record.count(); ++col)
	{
		QString colName = record.fieldName(col).toLower();
		if(colName != "id")    // id is not changable
			_model.setData(_model.index(row, col), ref.getValue(colName));
	}

	// tags are stored in a relations table separately
	recreateTagsRelations(splitLine(ref.getValue("tags").toString(), ";"));

	// add pdf
	QString pdfPath = ref.getValue("PDF").toString();
	if(!pdfPath.isEmpty())
		addAttachment(_currentID, suggestAttachmentName(pdfPath), pdfPath);
}

void PagePapers::updateRefByID(int id, const Reference& r)
{
	// id must exists
	QSqlQuery query;
	query.exec(tr("select * from Papers where ID = %1").arg(id));
	if(!query.next())
		return;

	int row = idToRow(&_model, PAPER_ID, id);

	// row is visible
	if(row > -1)
	{
		updateRefByRow(row, r);
		return;
	}

	Reference ref = r;
	ref.touch();   // add modified date

	// the row being updated is not visible
	QStringList clause;
	QSqlRecord record = _model.record(-1);
	for(int col = 0; col < record.count(); ++col)
	{
		QString colName = record.fieldName(col).toLower();
		if(colName != "id")   // don't change ID
			clause << colName + "=\'" + ref.getValue(colName).toString() + "\'";
	}
	query.exec(tr("update Papers set %1 where ID = %2").arg(clause.join(","))
													   .arg(id));

	// tags are stored in a relations table separately
	recreateTagsRelations(splitLine(ref.getValue("tags").toString(), ";"));

	// add pdf
	QString pdfPath = ref.getValue("PDF").toString();
	if(!pdfPath.isEmpty())
		addAttachment(_currentID, suggestAttachmentName(pdfPath), pdfPath);
}

void PagePapers::recreateTagsRelations(const QStringList& tags)
{
	// remove all relations to tags
	QSqlQuery query;
	query.exec(tr("delete from PaperTag where Paper = %1").arg(_currentID));

	// add relations back
	foreach(QString tagName, tags)
		attachNewTag(tagName);
}

void PagePapers::delRecord()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?",
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QModelIndexList idxList = ui.tvPapers->selectionModel()->selectedRows();
		QSqlDatabase::database().transaction();
		foreach(QModelIndex idx, idxList)     // find all selected indexes
			delPaper(rowToID(idx.row()));     // delete in the db
		QSqlDatabase::database().commit();
		_model.select();                      // reload db
	}
}

void PagePapers::editRecord() {
	onEditPaper();
}

// the row must be visible
int PagePapers::rowToID(int row) const {
	return row > -1 && row < _model.rowCount() ?
		_model.data(_model.index(row, PAPER_ID)).toInt() : -1;
}

void PagePapers::importFromFiles(const QStringList& filePaths)
{
	foreach(const QString& filePath, filePaths)
	{
		// reference only
		QString extension = QFileInfo(filePath).suffix().toLower();
		if((extension != "bib" && extension != "enw" && extension != "ris") || extension == "pdf")
			continue;

		// find spec
		RefSpec* spec = RefSpecFactory::getInstance()->getSpec(extension);
		if(!spec)
		{
			QMessageBox::critical(this, tr("Error"),
				tr("Can not find the specification for this reference format!"));
			return;
		}

		// open file
		QFile file(filePath);
		if(!file.open(QFile::ReadOnly))
		{
			QMessageBox::critical(this, tr("Error"),
				tr("Can not open file %1").arg(filePath));
			return;
		}

		// parse
		QList<Reference> references = spec->getParser()->parse(file.readAll(), spec);
		importReferences(references);  // currentPaperID = -1 if failed
	}
}

void PagePapers::importReferences(const QList<Reference>& references)
{
	if(references.isEmpty())
		return;

	QSqlDatabase::database().transaction();
	foreach(const Reference& ref, references)
	{
		QString title = ref.getValue("title").toString();
		if(title.isEmpty())
			continue;

		// show preview
		PaperDlg dlg(this);
		dlg.setWindowTitle(tr("Import Reference"));

		// load existing ref
		int id = titleToID(title);
		if(id > -1)
		{
			Reference oldRef = exportRef(id);
			dlg.setReference(oldRef);
			dlg.showMergeMark();
		}

		dlg.setReference(ref);   // merge new ref
		dlg.onDirty();           // enable OK button

		if(dlg.exec() == QDialog::Accepted)
			insertReference(dlg.getReference());   // currentPaperID will be equal to the ID of the newly added
	}
	QSqlDatabase::database().commit();
}

QList<Reference> PagePapers::parseContent(const QString& content)
{
	// try all specs
	QFileInfoList fileInfos = QDir("./Specifications").entryInfoList(QStringList() << "*.xml");
	foreach(QFileInfo fileInfo, fileInfos)
		if(RefSpec* spec = RefSpecFactory::getInstance()->getSpec(fileInfo.baseName()))
		{
			// use the parser associated with the spec
			QList<Reference> references = spec->getParser()->parse(content, spec);
			if(!references.isEmpty())
				return references;
		}
		return QList<Reference>();   // no parser can parse
}

void PagePapers::onImport()
{
	// try clipboard first
	QClipboard* clipboard = QApplication::clipboard();
	QString content = clipboard->text();
	if(!content.isEmpty())
	{
		// a webpage
		//QUrl url(content);
		//if(url.isValid())
		//{
		//    WebImporter* webImporter = WebImporter::getInstance();
		//    webImporter->parse(url);
		//    importReferences(QList<Reference>() << webImporter->getReference());
		//    return;
		//}

		// content of the reference
		QList<Reference> references = parseContent(content);
		if(!references.isEmpty())
		{
			importReferences(references);
			return;
		}
	}

	// import from local files
	QStringList files = QFileDialog::getOpenFileNames(
		this, "Import references", _setting->getLastImportPath(),
		"Reference (*.enw *.ris *.bib);;All files (*.*)");
	if(files.isEmpty())
		return;
	_setting->setLastImportPath(QFileInfo(files.front()).absolutePath());

	importFromFiles(files);
}

void PagePapers::search(const QString& target)
{
	// filter papers
	_model.setFilter(
		tr("Title       like \"%%1%\" or \
		   Authors     like \"%%1%\" or \
		   Year        like \"%%1%\" or \
		   Publication like \"%%1%\" or \
		   Abstract    like \"%%1%\" or \
		   Note        like \"%%1%\" ").arg(target));

	// highlight tags
	ui.widgetTags->search(target);
}

void PagePapers::onNewTag()
{
	TagDlg dlg("Tags", this);
	if(dlg.exec() == QDialog::Accepted)
	{
		int tagID = getNextID("Tags", "ID");
		ui.widgetTags->addTag(tagID, dlg.getText());     // create tag
		ui.widgetTags->addTagToItem(tagID, _currentID);  // add to paper
	}
}

void PagePapers::onAddTagToPaper()
{
	QModelIndexList rows = ui.tvPapers->selectionModel()->selectedRows(PAPER_ID);
    QList<WordLabel*> tags = ui.widgetTags->getSelected();
	foreach(QModelIndex idx, rows)  // for all selected papers
	{
		int paperID = _model.data(idx).toInt();
		foreach(WordLabel* tag, tags)   // add all selected tags to selected papers
			ui.widgetTags->addTagToItem(getTagID("Tags", tag->text()), paperID);
	}
}

// same structure as onAddTagToPaper()
void PagePapers::onDelTagFromPaper()
{
	QModelIndexList rows = ui.tvPapers->selectionModel()->selectedRows(PAPER_ID);
    QList<WordLabel*> tags = ui.widgetTags->getSelected();
	foreach(QModelIndex idx, rows)
	{
		int paperID = _model.data(idx).toInt();
		foreach(WordLabel* tag, tags)
			ui.widgetTags->removeTagFromItem(getTagID("Tags", tag->text()), paperID);
	}
}

void PagePapers::submit()
{
	if(!_model.submit())
		QMessageBox::critical(this, tr("Error"), _model.lastError().text());
}

void PagePapers::resetModel()
{
    _model.setEditStrategy(QSqlTableModel::OnFieldChange);
    _model.setTable("Papers");
	_model.select();
    fetchAll(&_model);
    ui.tvPapers->reSort();
    ui.tvPapers->loadSectionSizes();
}

void PagePapers::jumpToID(int id)
{
	_currentRow = idToRow(&_model, PAPER_ID, id);
	if(_currentRow < 0)
		_currentRow = 0;
	ui.tvPapers->selectRow(_currentRow);  // will trigger onSelectionChanged()
	ui.tvPapers->scrollTo(_model.index(_currentRow, PAPER_TITLE));
	ui.tvPapers->setFocus();
}

void PagePapers::onFilterPapersByTags(bool AND)
{
	// get selected tags
	QStringList tagIDs;
	QList<WordLabel*> tags = ui.widgetTags->getSelected();
	foreach(WordLabel* tag, tags)
		tagIDs << tr("%1").arg(getTagID("Tags", tag->text()));

	if(!AND)
		_model.setFilter(
		tr("ID in (select Paper from PaperTag where Tag in (%1))").arg(tagIDs.join(",")));
	else
	{
		dropTempView();
		QSqlQuery query;    // create a temp table for selected tags id
		query.exec(tr("create view SelectedTags as \
					  select * from Tags where ID in (%1)").arg(tagIDs.join(",")));

		// select papers that contain all the selected tags
		_model.setFilter("not exists \
						 (select * from SelectedTags where \
						 not exists \
						 (select * from PaperTag where \
						 Paper=Papers.ID and Tag=SelectedTags.ID))");
	}
}

void PagePapers::onAddQuote()
{
	QuoteDlg dlg(this);
	dlg.setWindowTitle(tr("Add Quote"));
	dlg.setQuoteID(getNextID("Quotes", "ID"));  // create new quote id
	dlg.addRef(idToTitle(_currentID));          // add current paper
	if(dlg.exec() == QDialog::Accepted)
		_mapper->refresh();                     // show the newly added
}

void PagePapers::onAddPDF()
{
	ui.tabWidget->setCurrentWidget(ui.tabAttachments);  // show attachment tab
	ui.widgetAttachments->onAddFile();
}

void PagePapers::onReadPDF() {
	::openAttachment(_currentID, "Paper.pdf");
}

void PagePapers::onFullTextSearch(const QString& target)
{
	resetModel();
	if(target.isEmpty())
		return;

	fetchAll(&_model);
	int rowCount = _model.rowCount();
	QProgressDialog progress(tr("Searching..."), tr("Abort"), 0, rowCount, this);
	progress.setWindowModality(Qt::WindowModal);

	QStringList filter;
	for(int row = 0; row < rowCount; ++row)
	{
		progress.setValue(row);
		qApp->processEvents();
		if(fullTextSearch(rowToID(row), target))        // if found in this paper
			filter << tr("ID = %1").arg(rowToID(row));  // add the paper to filter
		if(progress.wasCanceled())
			break;
	}

	if(filter.isEmpty())
		QMessageBox::information(this, tr("Full text search"), tr("No such paper!"));
	else
		_model.setFilter(filter.join(" OR "));        // filter out papers
}

void PagePapers::loadGeometry()
{
	ui.splitterHorizontal->restoreState(_setting->getSplitterSizes("PapersHorizontal"));
	ui.splitterPapers    ->restoreState(_setting->getSplitterSizes("PapersVertical"));
	ui.tabWidget->setCurrentIndex(_setting->getPapersTabIndex());
}

void PagePapers::saveGeometry()
{
	// closeEvent() does not work for child windows
	ui.tvPapers->saveSectionSizes();

	// splitters
	_setting->setSplitterSizes("PapersHorizontal", ui.splitterHorizontal->saveState());
	_setting->setSplitterSizes("PapersVertical",   ui.splitterPapers    ->saveState());
	_setting->setPapersTabIndex(ui.tabWidget->currentIndex());

    ui.widgetCoauthered ->saveSectionSizes();
    ui.widgetRelated    ->saveSectionSizes();
}

void PagePapers::onTagDoubleClicked(const QString& label)
{
	if(label.isEmpty())
	{
		reset();           // ensure all records visible
		jumpToCurrent();
	}
	else
		onFilterPapersByTags();
}

void PagePapers::onPrintMe(bool print)
{
	if(print)
		attachNewTag("PrintMe");
	else
		ui.widgetTags->removeTagFromItem(getTagID("Tags", "PrintMe"), _currentID);
}

void PagePapers::onBookmark(bool readMe)
{
	if(readMe)
		attachNewTag("ReadMe");
	else
		setPaperRead();
}

void PagePapers::attachNewTag(const QString& tagName)
{
	int tagID = getTagID("Tags", tagName);
	if(tagID < 0)   // new tag
	{
		tagID = getNextID("Tags", "ID");
		ui.widgetTags->addTag(tagID, tagName);
	}
	ui.widgetTags->addTagToItem(tagID, _currentID);
}

void PagePapers::setPaperRead() {
	ui.widgetTags->removeTagFromItem(getTagID("Tags", "ReadMe"), _currentID);
}

void PagePapers::onRelatedDoubleClicked(int paperID)
{
	_currentID = paperID;
	Navigator::getInstance()->addFootStep(this, _currentID);
	onEditPaper();
}

void PagePapers::onQuoteDoubleClicked(int quoteID) {
	MainWindow::getInstance()->jumpToQuote(quoteID);
}

Reference PagePapers::exportRef(int id) const
{
	Reference ref;
	QSqlQuery query;
	query.exec(tr("select * from Papers where ID = %1").arg(id));
	if(query.next())
		ref = recordToRef(query.record());

	// tags are not in the paper table
	ref.setValue("tags", getTagsOfPaper(id).join(";"));
	return ref;
}

Reference PagePapers::recordToRef(const QSqlRecord& record) const
{
	Reference ref;
	for(int col = 0; col < record.count(); ++col)
	{
		QString  fieldName  = record.fieldName(col).toLower();
		QVariant fieldValue = record.value(col);
		ref.setValue(fieldName, fieldValue);
	}
	return ref;
}

QString PagePapers::toString(const QModelIndexList& idxList, const QString& extension)
{
	// find spec
	RefSpec* spec = RefSpecFactory::getInstance()->getSpec(extension);
	if(spec == 0)
		return tr("Cannot find the specification for this reference format!");

	// get the output
	IRefExporter* exporter = ExporterFactory::getInstance()->getExporter(extension);
	QString result;
	QTextStream os(&result);
	foreach(QModelIndex idx, idxList)    // save each row
	{
		Reference ref = exportRef(rowToID(idx.row()));
		ref.generateID();
		os << exporter->toString(ref, *spec) << "\r\n";
	}
	return result;
}

void PagePapers::onExport()
{
	// get selected rows
	QModelIndexList idxSelected = ui.tvPapers->selectionModel()->selectedRows();
	if(idxSelected.isEmpty())
		return;

	// save to BibFixer
	if(_setting->getExportToBibFixer())
	{
		QString bibFixerPath = _setting->getBibFixerPath();
		if(bibFixerPath.isEmpty() || !QFile::exists(bibFixerPath))
		{
			QMessageBox::critical(this, tr("Error"), tr("Cannot find BibFixer!"));
			return;
		}

        QProcess* process = new QProcess;
        process->setProgram(bibFixerPath);
        process->setArguments(QStringList() << toString(idxSelected, "bib"));
        if(process->state() == QProcess::Running)
            process->terminate();
        else
            process->start();
//        process->start(bibFixerPath, QStringList() << toString(idxSelected, "bib"));
//        process->start("/Applications/Calculator.app");
	}
	// save to file
	else
	{
		// get file name
		QString lastPath = _setting->getLastImportPath();
		QString filePath = QFileDialog::getSaveFileName(this, tr("Export reference"),
			lastPath + "/" + idToTitle(_currentID),
			"Bibtex (*.bib);;Endnote (*.enw);;Reference Manager (*.ris);;All files (*.*)");
		if(filePath.isEmpty())
			return;
		_setting->setLastImportPath(QFileInfo(filePath).absolutePath());

		QFile file(filePath);
		if(file.open(QFile::WriteOnly | QFile::Truncate))
		{
			QTextStream os(&file);
			os << toString(idxSelected, QFileInfo(filePath).suffix().toLower());
		}
	}
}
