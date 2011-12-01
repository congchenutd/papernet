#ifndef THESAURUSCACHE_H
#define THESAURUSCACHE_H

#include <QObject>
#include <QStringList>

class ThesaurusCache : public QObject
{
    Q_OBJECT
public:
	static ThesaurusCache* getInstance(QObject* parent = 0);

	bool update(const QString& center, const QStringList& words);
	QStringList search(const QString& center) const;
	void clear();

private:
	ThesaurusCache(QObject* parent = 0);
	void addPair(const QString& lhs, const QString& rhs);

private:
	static ThesaurusCache* instance;
};

#endif // THESAURUSCACHE_H
