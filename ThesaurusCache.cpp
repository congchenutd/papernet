#include "ThesaurusCache.h"
#include <QSqlQuery>
#include <QVariant>
#include <QSet>
#include <QSqlError>

ThesaurusCache* ThesaurusCache::instance = 0;

ThesaurusCache::ThesaurusCache(QObject* parent) : QObject(parent)
{
	QSqlQuery query;
	query.exec("create table ThesaurusCache ( \
				   lhs varchar, rhs varchar,  \
				   primary key (lhs, rhs)     \
			   )");
}

// Though the graph is un-directed,
// the pair is not symmetrically stored, to save space
void ThesaurusCache::addPair(const QString &lhs, const QString &rhs)
{
	QSqlQuery query;
	query.prepare(tr("insert into ThesaurusCache values (:lhs, :rhs)"));
	query.bindValue(":lhs", lhs.toLower());
	query.bindValue(":rhs", rhs.toLower());
	query.exec();
}

// return true if local cache is updated by online result
bool ThesaurusCache::update(const QString& center, const QStringList& words)
{
	QSet<QString> onlineSet = words.toSet();
	QSet<QString> localSet = search(center).toSet();
	QSet<QString> newWords = onlineSet - localSet;
	if(!newWords.isEmpty())    // has something new
	{
		QSqlDatabase::database().transaction();
		foreach(QString word, newWords)
			addPair(center, word);
		QSqlDatabase::database().commit();
		return true;
	}
	return false;
}

// Search from both directions
// because the pair is stored uni-directionally
QStringList ThesaurusCache::search(const QString& target) const
{
	QStringList result;
	QSqlQuery query;
	query.prepare(tr("select rhs from ThesaurusCache where lhs = :lhs order by rhs"));
	query.bindValue(":lhs", target);
	query.exec();
	while(query.next())
		result << query.value(0).toString();

	query.prepare(tr("select lhs from ThesaurusCache where rhs = :rhs order by lhs"));
	query.bindValue(":rhs", target);
	query.exec();
	while(query.next())
		result << query.value(0).toString();

	return result;
}

ThesaurusCache* ThesaurusCache::getInstance(QObject* parent)
{
	if(instance == 0)
		instance = new ThesaurusCache(parent);
	return instance;
}

void ThesaurusCache::clear()
{
	QSqlQuery query;
	query.exec("delete from ThesaurusCache");
}
