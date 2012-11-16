#ifndef REFDLG_H
#define REFDLG_H

#include "ui_RefDlg.h"
#include "Reference.h"

class QGridLayout;

class RefDlg : public QDialog
{
    Q_OBJECT

    typedef QPair<QLabel*, QLineEdit*> Field;
    
public:
    explicit RefDlg(QWidget* parent = 0);

    // 2 ways to init
    void setSource(const QString& bibtexSource);
    void setReference(const Reference& reference);

private slots:
    void onTypeChanged(const QString& typeName);

private:
    void addField(const QString& fieldName, const QString& fieldValue);
    void clearFields();
    int  findField(const QString& fieldName) const;
    void loadTypeSpec(const QString& typeName);
    void restoreReference(const Reference& reference);
    
private:
    Ui::RefDlg ui;
    QGridLayout* _layout;
    QList<Field> _fields;
    Reference    _currentRef;
    QSpacerItem* _spacer;
};

#endif // REFDLG_H
