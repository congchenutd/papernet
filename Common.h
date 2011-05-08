#ifndef Common_h__
#define Common_h__

#include <QString>

bool openDB(const QString& name);
void createTables();
int getNextID(const QString& tableName, const QString& sectionName);
void delPaper(int paperID);
void delTag  (int tagID);
void addPaperTag(int paperID, int tagID);
void delPaperTag(int paperID, int tagID);
bool addAttachment(int paperID, const QString& attachmentName, const QString& fileName);
void delAttachment(int paperID, const QString& attachmentName);
void delAttachments(int paperID);
bool addLink(int paperID, const QString& linkName, const QString& url);
QString getPaperTitle(int paperID);
QString makeValidTitle(const QString& title);
QString getValidTitle(int paperID);
QString getAttachmentDir(int paperID);
void openAttachment(int paperID, const QString& attachmentName);
QString getAttachmentPath(int paperID, const QString& attachmentName);
bool renameAttachment(int paperID, const QString& oldName, const QString& newName);
bool renameTitle(const QString& oldName, const QString& newName);
int getMaxProximity();
int getMaxCoauthor();
bool titleExists(const QString& title);
void updateSnippet(int id, const QString& title, const QString& content);
int getPaperID(const QString& title);
void addPaperSnippet(int paperID, int snippetID);
void addPaper(int id, const QString& title);
void delSnippet(int id);
QStringList getPaperList(int snippetID);
bool isTagged(int paperID);
int getSnippetID(const QString& title);
void setPaperRead(int paperID);
void updateTagged(int paperID);
void updateAttached(int paperID);
bool fullTextSearch(int paperID, const QString& target);
void makeFullTextFiles();
void hideFile(const QString& filePath);
QString getPDFPath(int paperID);
void makePDFLink();
QString convertSlashes(const QString& link);
void openUrl(const QString& url);

typedef enum {ATTACH_NONE, ATTACH_ENDNOTE, ATTACH_PAPER, ATTACH_ALL} AttachmentStatus;
AttachmentStatus isAttached(int paperID);

enum {PAPER_ID, PAPER_READ, PAPER_TAGGED, PAPER_ATTACHED, PAPER_TITLE, 
	  PAPER_AUTHORS, PAPER_YEAR, PAPER_JOURNAL, PAPER_ABSTRACT, PAPER_NOTE, 
	  PAPER_PROXIMITY, PAPER_COAUTHOR, PAPER_ADDEDTIME};
enum {TAG_ID, TAG_NAME};
enum {SNIPPET_ID, SNIPPET_TITLE, SNIPPET_SNIPPET};

#endif // Common_h__
