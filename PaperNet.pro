cache()
QT += core sql network widgets

TEMPLATE = app
TARGET = PaperNet
INCLUDEPATH +=	../EnglishName \
				../WordCloud \
				../MySettings \
				../Thesaurus \
				../EmbeddableLineEdit

win32 {
	RC_FILE = PaperNet.rc
}
macx {
	ICON = Images/ToiletPaper.icns
}

# Input
HEADERS += \
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
		   ../MySettings/MySetting.h \
           ../WordCloud/WordCloudWidget.h \
		   ../WordCloud/FlowLayout.h \
		   ../EnglishName/EnglishName.h \
    NewReferenceDlg.h \
    RelatedPapersWidget.h \
	CoauthoredPapersWidget.h \
    QuotesWidget.h \
    RelatedPhraseWidget.h \
	RefParser.h \
	RefFormatSpec.h \
	RefExporter.h \
	Reference.h \
    ../BibFixer/Convertor.h \
    FieldEdit.h \
    PhraseDlg.h \
    QuoteDlg.h \
    TagDlg.h \
    SingleInstance.h \
    ../Thesaurus/ThesaurusCache.h \
    ../Thesaurus/Thesaurus.h \
    WebImporter.h \
    MultiSectionCompleter.h \
    ToolBar.h \
    SearchLineEdit.h \
	../EmbeddableLineEdit/EmbeddableLineEdit.h \
    PaperWidgetMapper.h
FORMS += \
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
	CoauthoredPapersWidget.ui \
    QuotesWidget.ui \
	RelatedPhraseWidget.ui \
    RefDlg.ui \
    PhraseDlg.ui \
    QuoteDlg.ui \
    TagDlg.ui
SOURCES += \
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
           ../WordCloud/WordCloudWidget.cpp \
		   ../WordCloud/FlowLayout.cpp      \
           ../EnglishName/EnglishName.cpp \
	NewReferenceDlg.cpp \
    RelatedPapersWidget.cpp \
	CoauthoredPapersWidget.cpp \
    QuotesWidget.cpp \
    RelatedPhraseWidget.cpp \
	RefParser.cpp \
	RefFormatSpec.cpp \
	RefExporter.cpp \
	Reference.cpp \
    ../BibFixer/Convertor.cpp \
    FieldEdit.cpp \
    PhraseDlg.cpp \
    QuoteDlg.cpp \
    TagDlg.cpp \
    SingleInstance.cpp \
    ../Thesaurus/ThesaurusCache.cpp \
    ../Thesaurus/Thesaurus.cpp \
    WebImporter.cpp \
    MultiSectionCompleter.cpp \
    ToolBar.cpp \
    SearchLineEdit.cpp \
	../EmbeddableLineEdit/EmbeddableLineEdit.cpp \
    PaperWidgetMapper.cpp
RESOURCES += MainWindow.qrc









