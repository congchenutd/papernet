QT += core sql network widgets webkitwidgets

TEMPLATE = app
TARGET = PaperNet
INCLUDEPATH +=	../EnglishName \
				../WordCloud \
				../MySetting \
                ../MyComboBox \
				../Thesaurus \
				../EmbeddableLineEdit \
				../BibFixer

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
           ../MySetting/MySetting.h \
           ../WordCloud/WordCloudWidget.h \
		   ../WordCloud/FlowLayout.h \
		   ../EnglishName/EnglishName.h \
    NewReferenceDlg.h \
    RelatedPapersWidget.h \
    CoauthoredPapersWidgdet.h \
    QuotesWidget.h \
    RelatedPhraseWidget.h \
	RefParser.h \
	RefFormatSpec.h \
	RefExporter.h \
	Reference.h \
    ../MyComboBox/SetTextComboBox.h \
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
	../EmbeddableLineEdit/EmbeddableLineEdit.h
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
    CoauthoredPapersWidgdet.ui \
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
    CoauthoredPapersWidgdet.cpp \
    QuotesWidget.cpp \
    RelatedPhraseWidget.cpp \
	RefParser.cpp \
	RefFormatSpec.cpp \
	RefExporter.cpp \
	Reference.cpp \
    ../MyComboBox/SetTextComboBox.cpp \
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
	../EmbeddableLineEdit/EmbeddableLineEdit.cpp
RESOURCES += MainWindow.qrc









