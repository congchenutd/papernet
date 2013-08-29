#ifndef PAPER_H
#define PAPER_H

#include <QVariant>
#include <QMap>

// A reference record
// case sensitive
// all field names should use lower case
class Reference
{
public:
    typedef QMap<QString, QVariant> Fields;  // field name -> field value

public:
    bool     fieldExists(const QString& field) const;
    void     setValue(const QString& fieldName, const QVariant& fieldValue);
    QVariant getValue(const QString& fieldName) const;
	void     generateID();                   // last name + year
    Fields   getAllFields() const { return _fields; }
    bool     isValid() const { return _fields.contains("type"); }
    void     clear();

private:
    Fields _fields;
	// NOTE: authors is stored as a stringlist
    // startpage, endpage <-> pages auto convered, and may be redundant
};

#endif // PAPER_H
