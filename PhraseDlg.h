#ifndef PHRASEDLG_H
#define PHRASEDLG_H

#include <QDialog>
#include "ui_PhraseDlg.h"

// A dialog to add and edit phrase
// Three fields: phrase, explanation, and tags (separated by ;)
class PhraseDlg : public QDialog
{
	Q_OBJECT

public:
	PhraseDlg(QWidget *parent = 0);
	virtual void accept();

    QString     getPhrase()      const;
    QString     getExplanation() const;
    QStringList getTags()        const;
    void setPhrase     (const QString&     phrase);
    void setExplanation(const QString&     explanation);
	void setTags       (const QStringList& tags);

private:
    Ui::PhraseDlg ui;
    bool _newPhrase;
};

#endif // PHRASEDLG_H
