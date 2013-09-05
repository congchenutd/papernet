#include "MultiSectionCompleter.h"
#include <QAbstractItemModel>
#include <QListView>
#include <QEvent>
#include <QKeyEvent>
#include <QSortFilterProxyModel>
#include <QLineEdit>

MultiSectionCompleter::MultiSectionCompleter(QObject* parent)
    : QObject(parent),
      _edit(0)
{
    _popup = new MyListView;
    _popup->setWindowFlags(Qt::Popup);
    _popup->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _popup->setSelectionBehavior(QAbstractItemView::SelectRows);
    _popup->setSelectionMode(QAbstractItemView::SingleSelection);
    _popup->installEventFilter(this);  // take over popup's events
    _popup->hide();
	connect(_popup, SIGNAL(completed()), this, SLOT(complete()));

    _proxy = new QSortFilterProxyModel(this);
}

void MultiSectionCompleter::setEdit(QLineEdit* edit)
{
    if(edit == 0)
        return;

    _edit = edit;
    _edit->installEventFilter(this);   // take over edit's eventts
    connect(_edit, SIGNAL(textEdited(QString)), this, SLOT(filter(QString)));
}

void MultiSectionCompleter::setModel(QAbstractItemModel* model, int column)
{
    if(model == 0)
        return;

    _proxy->setSourceModel(model);
    _proxy->sort(column);
    _proxy->setFilterKeyColumn(column);

    _popup->setModel(_proxy);
    _popup->setModelColumn(column);
}

void MultiSectionCompleter::showPopup()
{
    QPoint topLeft = _edit->mapToGlobal(QPoint(0, _edit->height()));
    _popup->setGeometry(topLeft.x(), topLeft.y(), _edit->width(), 100);
    // TODO: how to determine height dynamically?

    _popup->setCurrentIndex(model()->index(0, modelColumn()));  // highlight row 0
    _popup->show();
}

QAbstractItemModel *MultiSectionCompleter::model() {
    return _popup->model();
}

int MultiSectionCompleter::modelColumn() const {
    return _popup->modelColumn();
}

bool MultiSectionCompleter::eventFilter(QObject* o, QEvent* e)
{
    if(o == _edit)
	{
		// don't let popup takes focus away from edit
		if(e->type() == QEvent::FocusOut && _popup->isVisible())
			return true;   // true means the event is swallowed

		if(e->type() == QEvent::KeyPress)
		{
			QKeyEvent* ke = static_cast<QKeyEvent*>(e);
			if(ke->key() == Qt::Key_Down)   // Down key shows the popup
				showPopup();
		}
	}

    else if(o == _popup)
    {
        // key events
        if(e->type() == QEvent::KeyPress)
        {
            QKeyEvent* ke = static_cast<QKeyEvent*>(e);
            switch(ke->key())
            {
            // escape hides the popup
            case Qt::Key_Escape:
                _popup->hide();
                break;

            // enter completes the selection
            case Qt::Key_Return:
            case Qt::Key_Enter:
                complete();
                _popup->hide();
                break;

            // navitate (wrap) with up and down
            case Qt::Key_Up:
                if(model() && _popup->currentIndex().row() == 0)
                {
                    QModelIndex lastIndex = model()->index(model()->rowCount()-1,
                                                           modelColumn());
                    _popup->setCurrentIndex(lastIndex);
                    return true;
                }
                break;
            case Qt::Key_Down:
                if (_popup->model() && _popup->currentIndex().row() == model()->rowCount() - 1)
                {
                    QModelIndex firstIndex = model()->index(0, modelColumn());
                    _popup->setCurrentIndex(firstIndex);
                    return true;
                }
                break;

            // redirect other key events to edit
            default:
                _edit->event(ke);
                return true;
            }
        }

		// mouse events
		if(e->type() == QEvent::MouseButtonPress)
		{
			if(!_popup->underMouse())  // clicking outside of popup closes the popup
			{
				_popup->hide();
				return true;
			}
		}
    }

    return QObject::eventFilter(o, e);  // other events go through
}

void MultiSectionCompleter::filter(const QString& text)
{
    if(_edit == 0 || model() == 0 ||
       modelColumn() < 0 || modelColumn() > model()->columnCount())
        return;

    QString section = _separator.isEmpty() ?
                text        // no separator, the entire line is one section
              : text.split(_separator.trimmed()).last().trimmed();  // no extra space for filter

    // match from beginning
    _proxy->setFilterRegExp( QRegExp("^" + section, Qt::CaseInsensitive) );

    if(_proxy->rowCount() == 0)
        _popup->hide();     // hide when no match
    else
        showPopup();
}

void MultiSectionCompleter::complete()
{
    if(!_popup->currentIndex().isValid())
        return;

    QString section = _proxy->data(_popup->currentIndex()).toString();  // selected text
    if(_separator.isEmpty())      // no separator defined, just one section
    {
        _edit->setText(section);
        return;
    }

    // remove the last (incomplete) section
    QString oldText = _edit->text();
    int indexOfSeparator = oldText.lastIndexOf(_separator.trimmed());
    oldText.truncate(indexOfSeparator);

    // add separator before this section
    if(indexOfSeparator != -1)
        oldText += _separator;

    // append this section
    _edit->setText(oldText + section + _separator);
}


//////////////////////////////////////////////////////////////////////////
MyListView::MyListView(QWidget* parent) : QListView(parent) {}

void MyListView::mouseDoubleClickEvent(QMouseEvent* event)
{
    // mouse event cannot be intercepted by eventFilter when clicked/double clicked
    // on the list view
	if(underMouse())
	{
		emit completed();
		hide();
	}
	QListView::mousePressEvent(event);
}
