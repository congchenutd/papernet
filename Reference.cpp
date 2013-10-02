#include "Reference.h"
#include "EnglishName.h"
#include <QStringList>
#include <QDate>

bool Reference::fieldExists(const QString& field) const {
	return _fields.contains(field.toLower());
}

void Reference::setValue(const QString& fieldName, const QVariant& fieldValue)
{
	QString name = fieldName.toLower();

	// pages -> startpage, endpage
	if(name == "pages")
	{
		QStringList pages = fieldValue.toString().split("-");
		if(pages.size() == 2)
		{
			_fields["startpage"] = pages[0];
			_fields["endpage"]   = pages[1];
		}
	}

	// startpage, endpage -> pages
	else if(name == "startpage" || name == "endpage")
	{
		_fields[name] = fieldValue;
		if(fieldExists("startpage") && fieldExists("endpage"))
			_fields["pages"] = _fields["startpage"].toString() + "-" + _fields["endpage"].toString();
	}

	else
		_fields[name] = fieldValue;
}

QVariant Reference::getValue(const QString& fieldName) const
{
	QString name = fieldName.toLower();

	// startpage, endpage -> pages
	if(name == "pages" && fieldExists("startpage") && fieldExists("endpage"))
		return _fields["startpage"].toString() + "-" + _fields["endpage"].toString();

	else
		return fieldExists(name) ? _fields[name] : QVariant();
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

void Reference::touch() {
	_fields["modified"] = QDate::currentDate().toString("yyyy/MM/dd");
}

void Reference::clear() {
	_fields.clear();
}
