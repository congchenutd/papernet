#ifndef Common_h__
#define Common_h__

#include <QString>

typedef enum {ATTACH_NONE, ATTACH_ENDNOTE, ATTACH_PAPER, ATTACH_ALL} AttachmentStatus;

bool openDB(const QString& name);
void createTables();
int getNextID(const QString& tableName, const QString& sectionName);
void delPaper(int paperID);
bool isTagged(int paperID);
void setPaperRead(int paperID);
void updateAttached(int paperID);
int getMaxProximity();
int getMaxCoauthor();
AttachmentStatus isAttached(int paperID);

void delTag  (const QString& tagName);
int getTagID(const QString& tagName);
void updateTagSize(int tagID);
QStringList getTags(int paperID);
void renameTag(const QString& oldName, const QString& newName);
void addTag(int id, const QString& name);

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
bool fullTextSearch(int paperID, const QString& target);
void makeFullTextFiles();
void hideFile(const QString& filePath);
QString getPDFPath(int paperID);
QString convertSlashes(const QString& link);
void openUrl(const QString& url);

void updateSnippet(int id, const QString& title, const QString& content);
int getPaperID(const QString& title);
void addPaperSnippet(int paperID, int snippetID);
void addSimplePaper(int id, const QString& title);
void delSnippet(int id);
int getSnippetID(const QString& title);

enum {PAPER_ID, PAPER_READ, PAPER_TAGGED, PAPER_ATTACHED, PAPER_TITLE,
	  PAPER_AUTHORS, PAPER_YEAR, PAPER_JOURNAL, PAPER_ABSTRACT, PAPER_NOTE,
	  PAPER_PROXIMITY, PAPER_COAUTHOR, PAPER_ADDEDTIME};
enum {TAG_ID, TAG_NAME, TAG_SIZE};
enum {SNIPPET_ID, SNIPPET_TITLE, SNIPPET_SNIPPET};

#endif // Common_h__
