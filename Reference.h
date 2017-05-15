#ifndef PAPER_H
#define PAPER_H

#include <QVariant>
#include <QMap>

// A reference (paper) record
// case insensitive
// all field names are converted to lower case
// NOTE: startpage, endpage <-> pages auto convered, and may be redundant
// because some references use pages (e.g., 1-2), while some use startpage and endpage
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
    void     touch();         // set modified date to today

private:
    Fields _fields;
};

#endif // PAPER_H
