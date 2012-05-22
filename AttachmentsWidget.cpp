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
    paperID = -1;
//	setPaper(-1);

	QDir(".").mkdir(attachmentDir);
	QDir(".").mkdir(emptyDir);

	model.setIconProvider(iconProvider = new AttachmentIconProvider);
	model.setRootPath(attachmentDir);
	model.setResolveSymlinks(false);
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
	ui.actionAddFile->setEnabled(paperID > -1);
	ui.actionAddLink->setEnabled(paperID > -1);
	ui.actionRename ->setEnabled(currentIndex.isValid());
	ui.actionDel    ->setEnabled(currentIndex.isValid());

	QMenu contextMenu(this);
	contextMenu.addAction(ui.actionAddFile);
	contextMenu.addAction(ui.actionAddLink);
	contextMenu.addAction(ui.actionRename);
	contextMenu.addAction(ui.actionDel);
    contextMenu.exec(event->globalPos());
}

void AttachmentsWidget::showEvent(QShowEvent *) {
    update();
}

void AttachmentsWidget::setPaper(int id)
{
    if(id == -1 || paperID == id)
        return;
	paperID = id;

    if(isVisible())
        update();
}

void AttachmentsWidget::onAddFile()
{
	// file name
	UserSetting* setting = MySetting<UserSetting>::getInstance();
	const QString lastPath = setting->getLastAttachmentPath();
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"),
													lastPath, tr("All files (*.*)"));
	if(filePath.isEmpty())
		return;
	setting->setLastAttachmentPath(QFileInfo(filePath).absolutePath());

	// attachment name
	bool ok;
	QString attachmentName = QInputDialog::getText(this, tr("Attachment name"),
		tr("Attachment name"), QLineEdit::Normal, suggestAttachmentName(filePath), &ok);
	if(!ok || attachmentName.isEmpty())
		return;

	// add
	if(!addAttachment(paperID, attachmentName, filePath))
	{
		QMessageBox::critical(this, tr("Error"), tr("Add attachment error!"));
		return;
	}

	update();       // update the view
}

void AttachmentsWidget::onAddLink()
{
	LinkDlg dlg(this);
	if(dlg.exec() != QDialog::Accepted)
		return;

	if(!addLink(paperID, dlg.getName(), dlg.getUrl()))
	{
		QMessageBox::critical(this, tr("Error"), tr("Add link error!"));
		return;
	}

	update();
}

void AttachmentsWidget::onDel()
{
	if(QMessageBox::warning(this, "Warning", "Are you sure to delete?",
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		delAttachment(paperID, model.fileName(currentIndex));
		update();
	}
}

void AttachmentsWidget::onOpen(const QModelIndex& idx) {
	openAttachment(paperID, model.data(idx).toString());
}

void AttachmentsWidget::update()   // refresh
{
	QString dir = getAttachmentDir(paperID);
	if(QDir(dir).entryList().isEmpty())
		dir = emptyDir;
	ui.listView->setRootIndex(model.index(dir));
	updateAttached(paperID);	   // update the model's attached status
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
		QMessageBox::critical(this, tr("Error"), tr("Rename failed!"));
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
