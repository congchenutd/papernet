#include "AttachmentsWidget.h"
#include "Common.h"
#include "LinkDlg.h"
#include "OptionDlg.h"
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

extern QString attachmentDir;
extern QString emptyDir;

AttachmentsWidget::AttachmentsWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setPaper(-1);

	QDir(".").mkdir(attachmentDir);
	QDir(".").mkdir(emptyDir);

	model.setIconProvider(iconProvider = new AttachmentIconProvider);
	model.setRootPath(attachmentDir);
	model.setResolveSymlinks(false);
//    model.setNameFilters(QStringList() << "*.pdf" << "*.ris" << "*.enw" << "*.xml");
//    model.setNameFilterDisables(false);
	ui.listView->setModel(&model);
	ui.listView->setRootIndex(model.index(emptyDir));

	connect(ui.actionAddFile, SIGNAL(triggered()), this, SLOT(onAddFile()));
	connect(ui.actionAddLink, SIGNAL(triggered()), this, SLOT(onAddLink()));
	connect(ui.actionRename,  SIGNAL(triggered()), this, SLOT(onRename()));
	connect(ui.actionDel,     SIGNAL(triggered()), this, SLOT(onDel()));
	connect(ui.listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onOpen(QModelIndex)));
}

void AttachmentsWidget::contextMenuEvent(QContextMenuEvent* event)
{
	currentIndex = ui.listView->indexAt(event->pos());
	bool valid = currentIndex.isValid();
	ui.actionAddFile->setEnabled(paperID > -1);
	ui.actionAddLink->setEnabled(paperID > -1);
	ui.actionRename->setEnabled(valid);
	ui.actionDel->setEnabled(valid);

	QMenu contextMenu(this);
	contextMenu.addAction(ui.actionAddFile);
	contextMenu.addAction(ui.actionAddLink);
	contextMenu.addAction(ui.actionRename);
	contextMenu.addAction(ui.actionDel);
	contextMenu.exec(event->globalPos());
}

void AttachmentsWidget::setPaper(int id) 
{
	paperID = id;
	update();
}

void AttachmentsWidget::onAddFile()
{
	UserSetting* setting = MySetting<UserSetting>::getInstance();
	QString lastPath = setting->getLastAttachmentPath();
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"),
													lastPath, tr("All files (*.*)"));
	if(filePath.isEmpty())
		return;
	
	setting->setLastAttachmentPath(QFileInfo(filePath).absolutePath());
	
	bool ok;
	QString attachmentName = QInputDialog::getText(this, tr("Attachment name"), 
		tr("Attachment name"), QLineEdit::Normal, guessName(filePath), &ok);
	if(!ok || attachmentName.isEmpty())
		return;

	if(!addAttachment(paperID, attachmentName, filePath))
	{
		QMessageBox::critical(this, tr("Error"), tr("The name already exists!"));
		return;
	}

	update();
	updateAttached(paperID);
}

void AttachmentsWidget::onAddLink()
{
	LinkDlg dlg(this);
	if(dlg.exec() == QDialog::Accepted)
	{
		if(!addLink(paperID, dlg.getName(), dlg.getUrl()))
		{
			QMessageBox::critical(this, tr("Error"), tr("The name already exists!"));
			return;
		}

		update();
		updateAttached(paperID);
	}
}

void AttachmentsWidget::onDel()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?", 
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
        delAttachment(paperID, model.fileName(currentIndex));
		update();
		updateAttached(paperID);
	}
}

void AttachmentsWidget::onOpen(const QModelIndex& idx)
{
	openAttachment(paperID, model.data(idx).toString());
	setPaperRead(paperID);
	emit paperRead();   // let papers refresh
}

void AttachmentsWidget::update()   // refresh
{
	QString dir = getAttachmentDir(paperID);
	if(QDir(dir).entryList().isEmpty())
		dir = emptyDir;
	ui.listView->setRootIndex(model.index(dir));
}

void AttachmentsWidget::onRename()
{
	QString oldName = model.data(currentIndex).toString();
	bool ok;
	QString newName = QInputDialog::getText(this, tr("Attachment name"), 
		tr("Attachment name"), QLineEdit::Normal, oldName, &ok);
	if(!ok || newName.isEmpty())
		return;

	if(!renameAttachment(paperID, oldName, newName))
		QMessageBox::critical(this, tr("Error"), tr("The name already exists!"));
}

QString AttachmentsWidget::guessName(const QString &fileName)
{
	QString result = QFileInfo(fileName).fileName();
	if(fileName.endsWith(".pdf", Qt::CaseInsensitive))
		result = "Paper.pdf";
	else if(fileName.endsWith(".ris", Qt::CaseInsensitive))
		result = "EndNote.ris";
	else if(fileName.endsWith(".enw", Qt::CaseInsensitive))
		result = "EndNote.enw";	
	return result;
}

AttachmentsWidget::~AttachmentsWidget() {
	delete iconProvider;
}


//////////////////////////////////////////////////////////////////////////
AttachmentIconProvider::AttachmentIconProvider()
: QFileIconProvider() {}

QIcon AttachmentIconProvider::icon(const QFileInfo& info) const
{
	// special icon for pdf, looks better than the system's
	if(info.fileName().compare("Paper.pdf", Qt::CaseInsensitive) == 0)
		return QIcon(":/MainWindow/Images/PDF.png");
	return QFileIconProvider::icon(info);
}
