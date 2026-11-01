#include "textsearch.h"

#include <QPlainTextEdit>
#include <QTextCursor>
#include <QTextDocument>

namespace TextSearch {

bool findNext(QPlainTextEdit *editor,
              const QString &key,
              bool backward,
              bool caseSensitive,
              bool *wrapped)
{
    if (wrapped)
        *wrapped = false;
    if (!editor || key.isEmpty())
        return false;

    QTextDocument::FindFlags flags;
    if (backward)
        flags |= QTextDocument::FindBackward;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;

    // 从当前选区（上一次命中）的另一端开始，避免原地重复命中同一条
    QTextCursor start = editor->textCursor();
    if (start.hasSelection())
        start.setPosition(backward ? start.selectionStart() : start.selectionEnd());

    QTextCursor found = editor->document()->find(key, start, flags);

    if (found.isNull()) {
        // 剩余部分没有命中，回绕到另一端再找一次（连续查找）
        QTextCursor edge(editor->document());
        if (backward)
            edge.movePosition(QTextCursor::End);
        found = editor->document()->find(key, edge, flags);
        if (!found.isNull() && wrapped)
            *wrapped = true;
    }

    if (found.isNull())
        return false;

    editor->setTextCursor(found);
    editor->centerCursor();
    return true;
}

} // namespace TextSearch
