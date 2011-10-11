######################################################################
# Automatically generated by qmake (2.01a) Thu Oct 6 17:33:55 2011
######################################################################

TEMPLATE = app
TARGET =
DEPENDPATH += . zlib/include
INCLUDEPATH += . zlib/include

QT += sql xml network
ICON = Images/ToiletPaper.icns
win32 {
	QMAKE_LIBDIR += zlib/lib
	RC_FILE = PaperNet.rc
}
macx {
	LIBS += /usr/lib/libz.dylib
}

# Input
HEADERS += AddPhraseDlg.h \
           AddQuoteDlg.h \
           AddTagDlg.h \
           AttachmentsWidget.h \
           AutoSizeTableView.h \
           Common.h \
           DictionaryTableView.h \
           Importer.h \
           LinkDlg.h \
           MainWindow.h \
           Navigator.h \
           OptionDlg.h \
           Page.h \
           PageDictionary.h \
           PagePapers.h \
           PageQuotes.h \
           PaperDlg.h \
           PaperList.h \
           PaperModel.h \
           PaperTableView.h \
           Pdf2Text.h \
           QuotesTableView.h \
           TagsWidget.h \
           ToolBar.h \
           zlib/include/zconf.h \
           zlib/include/zlib.h \
           ../MySetting/MySetting.h \
           ../WordCloud/WordCloudWidget.h \
		   ../WordCloud/FlowLayout.h \
		   ../EnglishName/EnglishName.h \
			DictionaryModel.h \
    Thesaurus.h
FORMS += AddPhraseDlg.ui \
         AddQuoteDlg.ui \
         AddTagDlg.ui \
         AttachmentsWidget.ui \
         LinkDlg.ui \
         MainWindow.ui \
         OptionDlg.ui \
         PageDictionary.ui \
         PagePapers.ui \
         PageQuotes.ui \
         PaperDlg.ui \
         PaperList.ui
SOURCES += AddPhraseDlg.cpp \
           AddQuoteDlg.cpp \
           AddTagDlg.cpp \
           AttachmentsWidget.cpp \
           AutoSizeTableView.cpp \
           Common.cpp \
           DictionaryTableView.cpp \
           Importer.cpp \
           LinkDlg.cpp \
           Main.cpp \
           MainWindow.cpp \
           Navigator.cpp \
           OptionDlg.cpp \
           PageDictionary.cpp \
           PagePapers.cpp \
           PageQuotes.cpp \
           PaperDlg.cpp \
           PaperList.cpp \
           PaperModel.cpp \
           PaperTableView.cpp \
           Pdf2Text.cpp \
           QuotesTableView.cpp \
           TagsWidget.cpp \
           ToolBar.cpp \
           ../WordCloud/WordCloudWidget.cpp \
		   ../WordCloud/FlowLayout.cpp      \
           ../EnglishName/EnglishName.cpp \
		   DictionaryModel.cpp \
    Thesaurus.cpp
RESOURCES += MainWindow.qrc




