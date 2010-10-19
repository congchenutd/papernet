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
bool delAttachment(int paperID, const QString& attachmentName);
bool delAttachments(int paperID);
bool addLink(int paperID, const QString& linkName, const QString& url);
QString getPaperTitle(int paperID);
QString makeValidTitle(const QString& title);
QString getValidTitle(int paperID);
QString getAttachmentDir(int paperID);
void openAttachment(int paperID, const QString& attachmentName);
QString getFilePath(int paperID, const QString& attachmentName);
bool renameAttachment(int paperID, const QString& oldName, const QString& newName);
bool attachmentExists(int paperID, const QString& name);
bool renameTitle(const QString& oldName, const QString& newName);
int getMaxProximity();
int getMaxCoauthor();

const int MAX_PAPER_COUNT = 1000;

#endif // Common_h__