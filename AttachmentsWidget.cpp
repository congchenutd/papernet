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
#include <QFile>

extern QString attachmentDir;
extern QString emptyDir;         // a subdir under attachmentDir

AttachmentsWidget::AttachmentsWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    _paperID = -1;

	_model.setRootPath(attachmentDir);
	_model.setResolveSymlinks(false);
	ui.listView->setModel(&_model);
	ui.listView->setRootIndex(_model.index(emptyDir));

	connect(ui.actionAddFile, SIGNAL(triggered()), this, SLOT(onAddFile()));
	connect(ui.actionAddLink, SIGNAL(triggered()), this, SLOT(onAddLink()));
	connect(ui.actionRename,  SIGNAL(triggered()), this, SLOT(onRename()));
	connect(ui.actionDel,     SIGNAL(triggered()), this, SLOT(onDel()));
    connect(ui.actionExport,  SIGNAL(triggered()), this, SLOT(onExport()));
	connect(ui.listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onOpen(QModelIndex)));
}

void AttachmentsWidget::contextMenuEvent(QContextMenuEvent* event)
{
	_currentIndex = ui.listView->indexAt(event->pos());
	ui.actionAddFile->setEnabled(_paperID > -1);
	ui.actionAddLink->setEnabled(_paperID > -1);
	ui.actionRename ->setEnabled(_currentIndex.isValid());
	ui.actionDel    ->setEnabled(_currentIndex.isValid());
    ui.actionExport ->setEnabled(_currentIndex.isValid());

	QMenu contextMenu(this);
	contextMenu.addAction(ui.actionAddFile);
	contextMenu.addAction(ui.actionAddLink);
	contextMenu.addAction(ui.actionRename);
	contextMenu.addAction(ui.actionDel);
    contextMenu.addAction(ui.actionExport);
    contextMenu.exec(event->globalPos());
}

void AttachmentsWidget::showEvent(QShowEvent *) {
    update();
}

void AttachmentsWidget::setPaper(int id)
{
	if(id == -1)
        return;
	_paperID = id;

    if(isVisible())
        update();
}

void AttachmentsWidget::onAddFile()
{
	// file name
    UserSetting* setting = UserSetting::getInstance();
    QString lastPath = setting->getLastAttachmentPath();
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
	if(!addAttachment(_paperID, attachmentName, filePath))
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

	if(!addLink(_paperID, dlg.getName(), dlg.getUrl()))
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
		delAttachment(_paperID, _model.fileName(_currentIndex));
		update();
	}
}

void AttachmentsWidget::onOpen(const QModelIndex& idx) {
    openAttachment(_paperID, _model.data(idx).toString());
}

void AttachmentsWidget::onExport()
{
    // last path
    UserSetting* setting = UserSetting::getInstance();
    QString lastPath = setting->getLastAttachmentPath();

    // generate file name
    QFileInfo fileInfo = _model.fileInfo(_currentIndex);
    QString srcFileName = fileInfo.fileName();
    QString destFileName = fileInfo.fileName();
    if(srcFileName.toLower() == "paper.pdf")   // Paper.pdf -> [title].pdf
        destFileName = ::getFileSystemCompatibleTitle(_paperID) + ".pdf";

    QString destFilePath = QFileDialog::getSaveFileName(this, tr("Export File"),
                               lastPath + "/" + destFileName,
                               tr("All files (*.*)"));

    // copy
    if(!destFilePath.isEmpty())
    {
        QFile(fileInfo.absoluteFilePath()).copy(destFilePath);
        setting->setLastAttachmentPath(QFileInfo(destFilePath).absolutePath());
    }
}

void AttachmentsWidget::update()   // refresh
{
	QString dir = getAttachmentDir(_paperID);
    if(QDir(dir).entryList().isEmpty())
        dir = emptyDir;
	ui.listView->setRootIndex(_model.index(dir));
}

void AttachmentsWidget::onRename()
{
	QString oldName = _model.data(_currentIndex).toString();
	bool ok;
	QString newName = QInputDialog::getText(this, tr("Attachment name"),
		tr("Attachment name"), QLineEdit::Normal, oldName, &ok);
	if(!ok || newName.isEmpty())
		return;

	if(!renameAttachment(_paperID, oldName, newName))
		QMessageBox::critical(this, tr("Error"), tr("Rename failed!"));
}
