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
void delPaperTagByPaper(int paperID);
void delPaperTagByTag(int tagID);
bool addAttachment(int paperID, const QString& attachmentName, const QString& fileName);
void delAttachment(int paperID, const QString& attachmentName);
void delAttachments(int paperID);
bool addLink(int paperID, const QString& linkName, const QString& url);
QString getPaperTitle(int paperID);
QString makeValidTitle(const QString& title);
QString getAttachmentDir(int paperID);
void openAttachment(int paperID, const QString& attachmentName);
QString getFilePath(int paperID, const QString& attachmentName);
bool rename(int paperID, const QString& oldName, const QString& newName);
bool exists(int paperID, const QString& name);

const int MAX_PAPER_COUNT = 1000;

#endif // Common_h__