#include "noteeditwidget.h"
#include <QFont>
#include <QFontMetrics>
#include <QTextBlock>

const int NoteEditWidget::TextEditMargin(100);
const QString NoteEditWidget::CiceroTextSample("Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium");
const int NoteEditWidget::LineHeightPercentage(122);
const float NoteEditWidget::NoteEditWidthMultiplier(0.875);
const int NoteEditWidget::TextEditAnimationDuration(400);
const int NoteEditWidget::TextEditTextCursorCaretDeadZone(7);

NoteEditWidget::NoteEditWidget(QWidget *parent) :
    QFrame(parent),
    appModel(AppModel::getInstance()),
    textEdit(new QTextEdit(this)),
    visualCover(new QFrame(this)),
    textEditAnimation(new QPropertyAnimation(textEdit, "geometry"))
{
    integrateWithAppModel();
    setupVisualCover();
    connect(textEdit, SIGNAL(textChanged()), this, SLOT(reportNoteState()));
    connect(textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(reportSelectionState()));
    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textEditAnimation->setDuration(TextEditAnimationDuration);
}

void NoteEditWidget::integrateWithAppModel()
{
    connect(appModel, SIGNAL(modelWasUpdated(AppModel::ModelEvent, ModelInfo *)),
            this, SLOT(onModelStateChanged(AppModel::ModelEvent, ModelInfo *)));
}

void NoteEditWidget::setFocus()
{
    textEdit->setFocus();
}

void NoteEditWidget::resizeEvent(QResizeEvent *)
{
    int screenWidth = width();
    int screenHeight = height();
    if (screenWidth > screenHeight) { // we have a classical landscape monitor
        noteEditHeight = screenHeight - TextEditMargin;
        noteEditWidth = static_cast<int>(noteEditHeight * NoteEditWidthMultiplier);
    } else { // we have monitor standing in portrait
        noteEditWidth = screenWidth - TextEditMargin;
        noteEditHeight = static_cast<int>(noteEditWidth / NoteEditWidthMultiplier);
    }
    noteEditXPos = (screenWidth - noteEditWidth) / 2;
    noteEditYPos = (screenHeight - noteEditHeight) / 2;

    visualCover->setGeometry(0, 0, width(), height());
    textEdit->setGeometry(noteEditXPos, noteEditYPos, noteEditWidth, noteEditHeight);
    textEdit->setFocus();
    textEdit->setFont(getFontForTextEditWith(noteEditWidth));

    for (QTextBlock block = textEdit->document()->begin(); block.isValid(); block = block.next())
    {
        QTextCursor tc = QTextCursor(block);
        QTextBlockFormat fmt = block.blockFormat();
        fmt.setLineHeight(LineHeightPercentage, QTextBlockFormat::ProportionalHeight);
        tc.setBlockFormat(fmt);
    }
}

void NoteEditWidget::onModelStateChanged(AppModel::ModelEvent modelEvent, ModelInfo * infoPointer)
{
    if (AppModel::NoteChanged == modelEvent)
    {
        NoteModelInfo* newInfo = dynamic_cast<NoteModelInfo*>(infoPointer);
        if(newInfo && (textEdit->toPlainText() != newInfo->text))
        {
            textEdit->setPlainText(newInfo->text);
#ifndef Q_OS_LINUX
            textEdit->setTextCursor(newInfo->textCursor);
#endif
            if(!textEdit->isActiveWindow())
            {
                textEdit->ensureCursorVisible();
                resetTextEditPosition();
            }
        }
    }
    if (AppModel::CursorChanged == modelEvent)
    {
        NoteModelInfo* newInfo = dynamic_cast<NoteModelInfo*>(infoPointer);
        if(newInfo && (textEdit->textCursor() != newInfo->textCursor))
        {
            textEdit->setTextCursor(newInfo->textCursor);
            textEdit->ensureCursorVisible();
        }
        resetTextEditPosition();
    }
}

void NoteEditWidget::reportNoteState()
{
    appModel->reportNoteState(textEdit->toPlainText());
    if(textEdit->isActiveWindow())
    {
        adjustTextEditPosition();
    }
}

void NoteEditWidget::reportSelectionState()
{
    if(textEdit->isActiveWindow())
    {
        appModel->reportSelectionState(textEdit->textCursor());
    }
}

void NoteEditWidget::setupVisualCover()
{
    visualCover->setObjectName("visualCover");
    visualCover->setAttribute(Qt::WA_TransparentForMouseEvents);
    visualCover->setFocusPolicy(Qt::NoFocus);
    visualCover->raise();
}

const QFont NoteEditWidget::getFontForTextEditWith(const int width)
{
    int textEditWidth(0);
    int fontSize(2);
    QFont font("Arial", fontSize, QFont::Bold, true);
    while (textEditWidth < width)
    {
        font.setPointSize(fontSize);
        QFontMetrics fm(font);
        textEditWidth = fm.width(CiceroTextSample);
        ++fontSize;
    }
    font.setPointSize(fontSize - 1);
    return font;
}

void NoteEditWidget::adjustTextEditPosition()
{
    textEditAnimation->stop();
    if((textEdit->cursorRect().y() <= (textEdit->size().height()) - textEdit->font().pixelSize() * 2))
    {
        int adjustedNoteEditYPos;
        if (textEdit->cursorRect().left() > TextEditTextCursorCaretDeadZone)
        {
            adjustedNoteEditYPos = (noteEditHeight / 2) - textEdit->cursorRect().bottom();
        }
        else
        {
            adjustedNoteEditYPos = (noteEditHeight / 2) - lastTextCursorBottom;
        }
        lastTextCursorBottom = textEdit->cursorRect().bottom();
        textEditAnimation->setEndValue(QRect(noteEditXPos, adjustedNoteEditYPos, noteEditWidth, noteEditHeight));
        textEditAnimation->start();
        textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
}

void NoteEditWidget::resetTextEditPosition()
{
    textEditAnimation->stop();
    textEditAnimation->setEndValue(QRect(noteEditXPos, noteEditYPos, noteEditWidth, noteEditHeight));
    textEditAnimation->start();
    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}
