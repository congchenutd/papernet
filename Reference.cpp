#include "Reference.h"
#include "EnglishName.h"
#include <QStringList>

bool Reference::fieldExists(const QString& field) const {
    return fields.contains(field);
}

void Reference::setValue(const QString& fieldName, const QVariant& fieldValue)
{
    // pages -> startpage, endpage
    if(fieldName == "pages")
    {
        QStringList pages = fieldValue.toString().split("-");
        if(pages.size() == 2)
        {
            fields["startpage"] = pages[0];
            fields["endpage"]   = pages[1];
        }
    }

    // startpage, endpage -> pages
    else if(fieldName == "startpage" || fieldName == "endpage")
    {
        fields[fieldName] = fieldValue;
        if(fieldExists("startpage") && fieldExists("endpage"))
            fields["pages"] = fields["startpage"].toString() + "-" + fields["endpage"].toString();
    }

    // append to existing authors
    else if(fieldName == "authors" && fieldExists(fieldName))
        fields[fieldName] = fields[fieldName].toStringList() << fieldValue.toStringList();

    else
        fields[fieldName] = fieldValue;
}

QVariant Reference::getValue(const QString& fieldName) const
{
    // startpage, endpage -> pages
    if(fieldName == "pages" && fieldExists("startpage") && fieldExists("endpage"))
        return fields["startpage"].toString() + "-" + fields["endpage"].toString();

    else
        return fieldExists(fieldName) ? fields[fieldName] : QVariant();
}

void Reference::generateID()
{
	// get first author's last name
    if(!fields.contains("authors"))
        return;
    QStringList authors = fields["authors"].toStringList();
    if(authors.isEmpty())
        return;
    QString lastNameOfFirstAuthor = EnglishName(authors.front()).getLastName();

	// year
    if(!fields.contains("year"))
        return;
    QString year = fields["year"].toString();

    fields["id"] = lastNameOfFirstAuthor + year;
}
