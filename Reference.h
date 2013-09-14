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
    Fields   getAllFields() const { return _fields; }
    bool     isValid() const { return _fields.contains("type"); }
    void     clear();
    void     generateID();    // last name + year
    void     touch();         // set modified date

private:
    Fields _fields;
    // NOTE: startpage, endpage <-> pages auto convered, and may be redundant
};

#endif // PAPER_H
