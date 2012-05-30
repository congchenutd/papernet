#include "Convertor.h"
#include "OptionDlg.h"
#include <QTextStream>
#include <QFile>
#include <QRegExp>
#include <QtAlgorithms>
#include <QDebug>

CaseConvertor::CaseConvertor(QObject* parent) : Convertor(parent)
{
	// Prepositions
	lowercaseWords << "about" << "above" << "across" << "after" <<
					  "against" << "along" << "among" << "around" <<
					  "at" << "before" << "behind" << "below" <<
					  "beneath" << "beside" << "between" << "beyond" <<
					  "but" << "by" << "despite" << "down" <<
					  "during" << "except" << "for" << "from" <<
					  "in" << "inside" << "into" << "like" <<
					  "near" << "of" << "off" << "on" <<
					  "onto" << "out" << "outside" << "over" <<
					  "past" << "since" << "through" << "throughout" <<
					  "till" << "to" << "toward" << "under" <<
					  "underneath" << "until" << "up" << "upon" <<
					  "with" << "within" << "without";

	// Conjunctions
	lowercaseWords << "after" << "although" << "as" << "because" <<
					  "before" << "how" << "if" << "once" <<
					  "since" << "than" << "that" << "though" <<
					  "till" << "until" << "when" << "where" <<
					  "whether" << "while" << "and";

	// Articles
	lowercaseWords << "the" << "a" << "an";
}

QString CaseConvertor::convert(const QString& input) const
{
	QStringList convertedWords;
	QStringList words = input.split(' ');
	QString lastWord;
	foreach(QString word, words)
	{
		if(lowercaseWords.contains(word, Qt::CaseInsensitive) && !lastWord.endsWith(':'))
			convertedWords << word.toLower();               // lower case unless it follows a colon
		else
			convertedWords << toFirstCharUpperCase(word);   // convert
		lastWord = word;
	}

	// the first char of the sentence must be upper case
	QString result = toFirstCharUpperCase(convertedWords.join(" "));
	emit converted(input, result);
	return result;
}

QString CaseConvertor::toFirstCharUpperCase(const QString& word) const
{
	if(word.isEmpty())
		return word;

	return word.at(0).toUpper() + word.right(word.length() - 1);
}


////////////////////////////////////////////////////////////////////////////////
ProtectionConvertor::ProtectionConvertor(QObject* parent) : Convertor(parent) {}

QString ProtectionConvertor::convert(const QString& input) const
{
	QStringList convertedWords;
	QStringList words = input.split(' ');
	foreach(QString word, words)
		convertedWords << toFirstCharProtected(word);   // convert

	QString result = convertedWords.join(" ");
	emit converted(input, result);
	return result;
}

QString ProtectionConvertor::toFirstCharProtected(const QString &word) const
{
	if(word.isEmpty())
		return word;

	// skip {XXX}
	QRegExp rxAllProtected("^\\{.+\\}$");
	if(rxAllProtected.indexIn(word) > -1)
		return word;

	// skip {X}xxx
	QRegExp rxFirstProtected("^\\{w\\}");
	if(rxFirstProtected.indexIn(word) > -1)
		return word;

	// skip non-letter chars
	QString result = word;
	int idx = 0;
	while(idx < result.length() && !result.at(idx).isLetter())
		++ idx;

	// X... -> {X...}
	if(idx < result.length() && result.at(0).isUpper())
	{
		result.insert(idx, '{');
		result.insert(idx + 2, '}');
	}
	return result;
}


///////////////////////////////////////////////////////////////////////
//AbbreviationConvertor::AbbreviationConvertor(QObject* parent) : Convertor(parent) {}

//QString AbbreviationConvertor::convert(const QString& input) const
//{
//	QStringList rules = UserSetting::getInstance()->getSelectedAbbreviationRules();
//	QString result = input;
//	for(int i = rules.size()-1; i >=0; --i)      // reverse order: long rule first
//	{
//		QStringList sections = rules.at(i).split(';');  // two parts of a rule
//		if(sections.size() == 2)
//		{
//			QString fullName        = sections[0];
//			QString abbreviatedName = sections[1];
//			QRegExp rxBracket("\\([^\\)]+\\)");
//			if(rxBracket.indexIn(fullName) > -1)
//			{
//				QString longFullName = fullName;
//				longFullName.remove('(');
//				longFullName.remove(')');

//				QString shortFullName = fullName.remove(rxBracket).simplified();

//				result.replace(longFullName, abbreviatedName, Qt::CaseInsensitive);
//				result.replace(shortFullName, abbreviatedName, Qt::CaseInsensitive);
//			}
//			else
//				result.replace(fullName, abbreviatedName, Qt::CaseInsensitive);
//		}
//	}

//	if(input != result)
//		emit converted(input, result);
//	return result;
//}


