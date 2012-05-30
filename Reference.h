#ifndef PAPER_H
#define PAPER_H

#include <QVariant>
#include <QMap>

// A reference record
class Reference
{
public:
    typedef QMap<QString, QVariant> Fields;  // field name -> field value

public:
    bool     containsField(const QString& field) const;
    void     setValue(const QString& fieldName, const QVariant& fieldValue);
    QVariant getValue(const QString& fieldName) const;
	void     generateID();                   // last name + year
    Fields   getAllFields() const;

private:
	Fields fields;
	// NOTE: authors is stored as a stringlist
	// startpage, endpage <-> pages auto convered
};

#endif // PAPER_H
