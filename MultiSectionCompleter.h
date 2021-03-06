#ifndef COMPLETER_H
#define COMPLETER_H

#include <QObject>
#include <QListView>

class QLineEdit;
class QAbstractItemModel;
class QListView;
class QSortFilterProxyModel;
class MyListView;

///////////////////////////////////////////////////////////////////////////////
/// MultiSectionCompleter works like QCompleter
/// But it allows an edit to have multiple separated sections
/// Each section can trigger filtering the content of the list
/// The selected text in the list will be appended to the edit as a new section
///
/// e.g., when typing "hello; wor",
/// "wor" will be used as the filter, instead of the entire string
///
/// When user selects an item from the list, e.g., "world"
/// the item will be appended to the edit as a new section, i.e. "hello; world"
///
class MultiSectionCompleter : public QObject
{
    Q_OBJECT

public:
    MultiSectionCompleter(QObject* parent = 0);
    void setEdit(QLineEdit* edit);
    void setModel(QAbstractItemModel* model, int column = 0);
    void setSeparator(const QString& separator) {_separator = separator; }

protected:
    bool eventFilter(QObject* o, QEvent* e);  // take over edit and popup

private slots:
    void filter(const QString& text);
    void complete();      // append selected item to edit

private:
    void showPopup();
    QAbstractItemModel* model();               // for convenience
    int                 modelColumn() const;   // for convenience

private:
    QLineEdit*             _edit;         // only works with QLineEdit
    MyListView*            _popup;
    QSortFilterProxyModel* _proxy;        // for filering
    QString                _separator;    // separating sections
};


class MyListView : public QListView
{
	Q_OBJECT

public:
	MyListView(QWidget* parent = 0);

protected:
	void mouseDoubleClickEvent(QMouseEvent* event);

signals:
	void completed();
};

#endif // COMPLETER_H
