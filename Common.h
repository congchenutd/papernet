#ifndef Common_h__
#define Common_h__

#include <QString>

class QAbstractItemModel;

// emun
typedef enum {ATTACH_NONE, ATTACH_ENDNOTE, ATTACH_PAPER, ATTACH_ALL} AttachmentStatus;
enum {PAPER_ID, PAPER_READ, PAPER_TAGGED, PAPER_ATTACHED, PAPER_TITLE,
	  PAPER_AUTHORS, PAPER_YEAR, PAPER_JOURNAL, PAPER_ABSTRACT, PAPER_NOTE,
	  PAPER_PROXIMITY, PAPER_COAUTHOR, PAPER_ADDEDTIME};
enum {TAG_ID, TAG_NAME, TAG_SIZE};
enum {QUOTE_ID, QUOTE_TITLE, QUOTE_QUOTE};
enum {DICTIONARY_ID, DICTIONARY_PHRASE, DICTIONARY_EXPLANATION, DICTIONARY_PROXIMITY};

// common
bool openDB(const QString& name);
void createTables();
int getNextID(const QString& tableName, const QString& sectionName);
int idToRow(QAbstractItemModel* model, int idSection, int id);

// papers
void delPaper(int paperID);
bool isTagged(int paperID);
void setPaperRead(int paperID);
int getMaxCoauthor();
int getPaperID(const QString& title);
bool paperExists(const QString& title);
void updateAttached(int paperID);
AttachmentStatus isAttached(int paperID);

// attachments
bool addAttachment(int paperID, const QString& attachmentName, const QString& fileName);
void delAttachment(int paperID, const QString& attachmentName);
void delAttachments(int paperID);
bool addLink(int paperID, const QString& linkName, const QString& url);
QString getAttachmentDir(int paperID);
void openAttachment(int paperID, const QString& attachmentName);
QString getAttachmentPath(int paperID, const QString& attachmentName);
bool renameAttachment(int paperID, const QString& oldName, const QString& newName);
bool fullTextSearch(int paperID, const QString& target);
void makeFullTextFiles();
void hideFile(const QString& filePath);
QString getPDFPath(int paperID);
void openUrl(const QString& url);

// tags
int getMaxProximity(const QString& tableName);
int  getTagID (const QString& tableName, const QString& tagName);
void delTag   (const QString& tableName, const QString& tagName);
void renameTag(const QString& tableName, const QString& oldName, const QString& newName);
QStringList getTagsOfPaper (int paperID);
QStringList getTagsOfPhrase(int phraseID);

// paper title
QString getPaperTitle(int paperID);
QString makeValidTitle(const QString& title);
QString getValidTitle(int paperID);
void renameTitle(const QString& oldName, const QString& newName);
QString convertSlashes(const QString& link);

// quotes
void updateQuote(int id, const QString& title, const QString& content);
void addPaperQuote(int paperID, int quoteID);
void addSimplePaper(int id, const QString& title);
void delQuote(int id);
int getQuoteID(const QString& title);

// dictionary
void delPhrase(int id);
bool phraseExists(const QString& phrase);

#endif // Common_h__
