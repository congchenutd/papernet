#ifndef Common_h__
#define Common_h__

#include <QString>

bool openDB(const QString& name);
void createTables();
int getNextID(const QString& tableName, const QString& sectionName);
void delPaper(int paperID);
void delTag  (int tagID);
void delPDF  (int paperID);
void addPaperTag(int paperID, int tagID);
void delPaperTag(int paperID, int tagID);
void delPaperTagByPaper(int paperID);
void delPaperTagByTag(int tagID);

const int MAX_PAPER_COUNT = 1000;

#endif // Common_h__