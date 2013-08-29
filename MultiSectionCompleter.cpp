#include "MultiSectionCompleter.h"
#include <QAbstractItemModel>
#include <QListView>
#include <QEvent>
#include <QKeyEvent>
#include <QSortFilterProxyModel>
#include <QLineEdit>

MultiSectionCompleter::MultiSectionCompleter(QObject* parent)
    : QObject(parent),
      _edit(0),
      _model(0),
      _modelColumn(-1)
{
    _popup = new QListView;
    _popup->setWindowFlags(Qt::Popup);
    _popup->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _popup->setSelectionBehavior(QAbstractItemView::SelectRows);
    _popup->setSelectionMode(QAbstractItemView::SingleSelection);
    _popup->installEventFilter(this);  // take over popup's events
    _popup->hide();

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

    _model = model;
    _modelColumn = column;

    _proxy->setSourceModel(model);
    _proxy->sort(_modelColumn);
    _proxy->setFilterKeyColumn(_modelColumn);

    _popup->setModel(_proxy);
    _popup->setModelColumn(_modelColumn);
}

void MultiSectionCompleter::showPopup()
{
    QPoint topLeft = _edit->mapToGlobal(QPoint(0, _edit->height()));
    _popup->setGeometry(topLeft.x(), topLeft.y(), _edit->width(), 100);
    // TODO: how to determine height dynamically?

    _popup->show();
}

bool MultiSectionCompleter::eventFilter(QObject* o, QEvent* e)
{
    // don't let popup takes focus away from edit
    if(o == _edit && e->type() == QEvent::FocusOut && _popup->isVisible())
        return true;

    if (o != _popup)
        return QObject::eventFilter(o, e);

    if(o == _popup)
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
                if(_popup->currentIndex().row() == 0)
                {
                    QModelIndex lastIndex = _model->index(_popup->model()->rowCount()-1, _modelColumn);
                    _popup->setCurrentIndex(lastIndex);
                    return true;
                }
                return false;
            case Qt::Key_Down:
                if (_popup->currentIndex().row() == _model->rowCount() - 1)
                {
                    QModelIndex firstIndex = _model->index(0, _modelColumn);
                    _popup->setCurrentIndex(firstIndex);
                    return true;
                }
                return false;

            default:
                break;
            }

            // redirect other key events to edit
            _edit->event(ke);

            return true;  // default is to ignore (eat) the event
        }

        // mouse events
        else if(e->type() == QEvent::MouseButtonPress)
        {
            if(!_popup->underMouse())  // clicking outside of popup closes the popup
            {
                _popup->hide();
                return true;
            }
        }

        // TODO: double click to select an item
    }

    return false;  // other events go through
}

void MultiSectionCompleter::filter(const QString& text)
{
    if(_edit == 0 || _model == 0 ||
       _modelColumn < 0 || _modelColumn > _model->columnCount())
        return;

    QString section = _separator.isEmpty() ?
                text        // entire line is one section
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
    if(_separator.isEmpty())      // just one section
    {
        _edit->setText(section);
        return;
    }

    // remove the last (uncomplete) section
    QString oldText = _edit->text();
    int indexOfSeparator = oldText.lastIndexOf(_separator.trimmed());
    oldText.truncate(indexOfSeparator);

    // add separator before this section
    if(indexOfSeparator != -1)
        oldText = oldText + _separator;

    // append this section
    _edit->setText(oldText + section + _separator);
}
