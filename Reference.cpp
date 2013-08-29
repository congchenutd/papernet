#include "Reference.h"
#include "EnglishName.h"
#include <QStringList>
#include <QSet>

bool Reference::fieldExists(const QString& field) const {
    return _fields.contains(field);
}

void Reference::setValue(const QString& fieldName, const QVariant& fieldValue)
{
    // pages -> startpage, endpage
    if(fieldName == "pages")
    {
        QStringList pages = fieldValue.toString().split("-");
        if(pages.size() == 2)
        {
            _fields["startpage"] = pages[0];
            _fields["endpage"]   = pages[1];
        }
    }

    // startpage, endpage -> pages
    else if(fieldName == "startpage" || fieldName == "endpage")
    {
        _fields[fieldName] = fieldValue;
        if(fieldExists("startpage") && fieldExists("endpage"))
            _fields["pages"] = _fields["startpage"].toString() + "-" + _fields["endpage"].toString();
    }

    // append to existing authors
    else if(fieldName == "authors" && fieldExists(fieldName))
    {
        QStringList oldList = _fields[fieldName].toStringList();
        QStringList newList = fieldValue.toStringList();
        foreach(const QString& name, newList)
            if(!oldList.contains(name, Qt::CaseInsensitive))
                oldList << name;
        _fields[fieldName] = oldList;
    }

    else
        _fields[fieldName] = fieldValue;
}

QVariant Reference::getValue(const QString& fieldName) const
{
    // startpage, endpage -> pages
    if(fieldName == "pages" && fieldExists("startpage") && fieldExists("endpage"))
        return _fields["startpage"].toString() + "-" + _fields["endpage"].toString();

    else
        return fieldExists(fieldName) ? _fields[fieldName] : QVariant();
}

void Reference::generateID()
{
	// get first author's last name
    if(!_fields.contains("authors"))
        return;
    QStringList authors = _fields["authors"].toStringList();
    if(authors.isEmpty())
        return;
    QString lastNameOfFirstAuthor = EnglishName(authors.front()).getLastName();

	// year
    if(!_fields.contains("year"))
        return;
    QString year = _fields["year"].toString();

    _fields["id"] = lastNameOfFirstAuthor + year;
}

void Reference::clear() {
    _fields.clear();
}
