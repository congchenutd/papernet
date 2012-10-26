######################################################################
# Automatically generated by qmake (2.01a) Thu Oct 6 17:33:55 2011
######################################################################

TEMPLATE = app
TARGET =
INCLUDEPATH +=	../EnglishName \
				../WordCloud \
				../MySetting \
                ../MyComboBox

QT += sql network
win32 {
	RC_FILE = PaperNet.rc
}
macx {
	ICON = Images/ToiletPaper.icns
}

# Input
HEADERS += AddPhraseDlg.h \
           AddQuoteDlg.h \
           AddTagDlg.h \
           AttachmentsWidget.h \
           AutoSizeTableView.h \
           Common.h \
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
           TagsWidget.h \
           ToolBar.h \
           ../MySetting/MySetting.h \
           ../WordCloud/WordCloudWidget.h \
		   ../WordCloud/FlowLayout.h \
		   ../EnglishName/EnglishName.h \
		   Thesaurus.h \
    NewReferenceDlg.h \
    ThesaurusCache.h \
    RelatedPapersWidget.h \
    CoauthoredPapersWidgdet.h \
    QuotesWidget.h \
    RelatedPhraseWidget.h \
	RefParser.h \
	RefFormatSpec.h \
	RefExporter.h \
	Reference.h \
    ../MyComboBox/SetTextComboBox.h \
    ../BibFixer/Convertor.h
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
         PaperList.ui \
    NewReferenceDlg.ui \
    RelatedPapersWidget.ui \
    CoauthoredPapersWidgdet.ui \
    QuotesWidget.ui \
	RelatedPhraseWidget.ui
SOURCES += AddPhraseDlg.cpp \
           AddQuoteDlg.cpp \
           AddTagDlg.cpp \
           AttachmentsWidget.cpp \
           AutoSizeTableView.cpp \
           Common.cpp \
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
           TagsWidget.cpp \
           ToolBar.cpp \
           ../WordCloud/WordCloudWidget.cpp \
		   ../WordCloud/FlowLayout.cpp      \
           ../EnglishName/EnglishName.cpp \
		   Thesaurus.cpp \
	NewReferenceDlg.cpp \
    ThesaurusCache.cpp \
    RelatedPapersWidget.cpp \
    CoauthoredPapersWidgdet.cpp \
    QuotesWidget.cpp \
    RelatedPhraseWidget.cpp \
	RefParser.cpp \
	RefFormatSpec.cpp \
	RefExporter.cpp \
	Reference.cpp \
    ../MyComboBox/SetTextComboBox.cpp \
    ../BibFixer/Convertor.cpp
RESOURCES += MainWindow.qrc









