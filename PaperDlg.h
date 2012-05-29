#ifndef PAPERDLG_H
#define PAPERDLG_H

#include <QDialog>
#include "ui_PaperDlg.h"
#include "Importer.h"

class Reference;

// add/edit paper
class PaperDlg : public QDialog
{
	Q_OBJECT

public:
	PaperDlg(QWidget *parent = 0);
	virtual void accept();

    QString getType     () const;
    QString getJournal  () const;
    QString getAbstract () const;
    int     getVolume   () const;
    int     getIssue    () const;
    int     getStartPage() const;
    int     getEndPage  () const;
    QString getPublisher() const;
    QString getEditors  () const;
    QString getUrl      () const;
    QString getNote     () const;

    QStringList getTags () const;

    void setTitle    (const QString& title);
    void setAuthors  (const QString& authors);
    void setYear     (int year);
    void setType     (const QString& type);
    void setJournal  (const QString& journal);
    void setAbstract (const QString& ab);
    void setNote     (const QString& note);
    void setTags     (const QStringList& tags);

    Reference getReference() const;
    void      setReference(const Reference& ref);

private:
	Ui::PaperDlgClass ui;
	bool newPaper;
};

#endif // PAPERDLG_H
