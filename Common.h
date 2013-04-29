#ifndef Common_h__
#define Common_h__

#include <QString>

class QAbstractItemModel;

// emun
typedef enum {ATTACH_NONE, ATTACH_ENDNOTE, ATTACH_PAPER, ATTACH_ALL} AttachmentStatus;
enum {PAPER_ID, PAPER_ATTACHED, PAPER_TITLE, PAPER_AUTHORS, PAPER_YEAR, PAPER_MODIFIED, PAPER_TYPE,
      PAPER_PUBLICATION, PAPER_ABSTRACT, PAPER_VOLUME, PAPER_ISSUE, PAPER_STARTPAGE,
      PAPER_ENDPAGE, PAPER_PUBLISHER, PAPER_EDITORS, PAPER_ADDRESS, PAPER_URL, PAPER_NOTE};
enum {TAG_ID, TAG_NAME, TAG_SIZE};
enum {QUOTE_ID, QUOTE_TITLE, QUOTE_QUOTE};
enum {DICT_ID, DICT_PHRASE, DICT_EXPLANATION};

// common
bool openDB(const QString& name);
void createTables();
int getNextID(const QString& tableName, const QString& sectionName);
int idToRow(QAbstractItemModel* model, int idSection, int id);

// separated list -> QStringList, unnecessary spaces removed
QStringList splitLine(const QString& line, const QString& separator);

// same as above, name formatted
QStringList splitAuthorsList(const QString& authorsLine,
							 const QString& separator = ";",
							 const QString& format = "L,;m;F");

// papers
void delPaper(int paperID);
bool isTagged(int paperID);
bool isPaperRead(int paperID);
bool isPaperToBePrinted(int paperID);
int getMaxCoauthor();
int getPaperID(const QString& title);
bool paperExists(const QString& title);
void updateAttached(int paperID);

// attachments
AttachmentStatus isAttached(int paperID);
QString getFullTextFilePath(int paperID, const QString& attachmentName);
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
void makeFullTextFile(const QString& pdfPath, const QString& fulltextPath);
void hideFile(const QString& filePath);
void openFile(const QString& filePath);
QString suggestAttachmentName(const QString& fileName);
QString autoRename(const QString& original);

// tags
int getMaxProximity(const QString& tableName);
int  getTagID (const QString& tableName, const QString& tagName);
void delTag   (const QString& tableName, const QString& tagName);
void renameTag(const QString& tableName, const QString& oldName, const QString& newName);
QStringList getTagsOfPaper (int paperID);
QStringList getTagsOfPhrase(int phraseID);
void dropTempView();

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

struct Phrase
{
    Phrase(int i = -1, const QString& n = QString(), const QString& e = QString())
        : id(i), name(n), explanation(e) {}
    int id;
    QString name;
    QString explanation;
};

Phrase findPhrase(int id);
Phrase findPhrase(const QString& phraseName);

#endif // Common_h__
