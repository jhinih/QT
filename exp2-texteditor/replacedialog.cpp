#include "replacedialog.h"
#include "textsearch.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextCursor>
#include <QTextDocument>

ReplaceDialog::ReplaceDialog(QPlainTextEdit *editor, QWidget *parent)
    : QDialog(parent)
    , m_editor(editor)
{
    setWindowTitle(tr("替换"));
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    m_editFind = new QLineEdit(this);
    m_editFind->setPlaceholderText(tr("要被替换的内容"));
    m_editReplace = new QLineEdit(this);
    m_editReplace->setPlaceholderText(tr("替换为（可留空表示删除）"));
    m_editFind->setMinimumWidth(220);
    m_editReplace->setMinimumWidth(220);

    m_caseCheck = new QCheckBox(tr("区分大小写"), this);

    auto *btnFind = new QPushButton(tr("查找下一个"), this);
    auto *btnReplace = new QPushButton(tr("替换"), this);
    btnReplace->setDefault(true);
    auto *btnReplaceAll = new QPushButton(tr("全部替换"), this);
    auto *btnClose = new QPushButton(tr("关闭"), this);

    m_hint = new QLabel(this);
    m_hint->setStyleSheet(QStringLiteral("color:#c0392b;"));
    m_hint->setMinimumHeight(18);

    auto *layout = new QGridLayout(this);
    layout->addWidget(new QLabel(tr("查找内容:"), this), 0, 0);
    layout->addWidget(m_editFind, 0, 1, 1, 3);
    layout->addWidget(new QLabel(tr("替换为:"), this), 1, 0);
    layout->addWidget(m_editReplace, 1, 1, 1, 3);
    layout->addWidget(m_caseCheck, 2, 1, 1, 3);
    layout->addWidget(m_hint, 3, 0, 1, 4);
    layout->addWidget(btnFind, 4, 0);
    layout->addWidget(btnReplace, 4, 1);
    layout->addWidget(btnReplaceAll, 4, 2);
    layout->addWidget(btnClose, 4, 3);
    layout->setColumnStretch(3, 1);

    connect(btnFind, &QPushButton::clicked, this, &ReplaceDialog::findNext);
    connect(btnReplace, &QPushButton::clicked, this, &ReplaceDialog::replaceOne);
    connect(btnReplaceAll, &QPushButton::clicked, this, &ReplaceDialog::replaceAll);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::close);
    connect(m_editFind, &QLineEdit::returnPressed, this, &ReplaceDialog::findNext);
    connect(m_editReplace, &QLineEdit::returnPressed, this, &ReplaceDialog::replaceOne);

    setTabOrder(m_editFind, m_editReplace);
    setTabOrder(m_editReplace, m_caseCheck);
    setTabOrder(m_caseCheck, btnFind);
    setTabOrder(btnFind, btnReplace);
    setTabOrder(btnReplace, btnReplaceAll);
    setTabOrder(btnReplaceAll, btnClose);
}

QString ReplaceDialog::searchKey() const
{
    return m_editFind->text();
}

Qt::CaseSensitivity ReplaceDialog::caseSensitivity() const
{
    return m_caseCheck->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
}

void ReplaceDialog::setSearchText(const QString &text)
{
    m_editFind->setText(text);
    m_editFind->selectAll();
}

void ReplaceDialog::activate()
{
    show();
    raise();
    activateWindow();
    m_editFind->setFocus();
    m_editFind->selectAll();
}

void ReplaceDialog::findNext()
{
    const QString key = searchKey();
    if (key.isEmpty()) {
        m_hint->setText(tr("请输入要查找的内容"));
        m_editFind->setFocus();
        return;
    }

    bool wrapped = false;
    if (!TextSearch::findNext(m_editor, key, false, m_caseCheck->isChecked(), &wrapped)) {
        m_hint->setText(tr("未找到“%1”").arg(key));
        return;
    }

    m_editor->setFocus();
    m_hint->setText(wrapped ? tr("已到文件末尾，从开头继续查找") : QString());
}

void ReplaceDialog::replaceOne()
{
    const QString key = searchKey();
    if (key.isEmpty()) {
        m_hint->setText(tr("请输入要查找的内容"));
        m_editFind->setFocus();
        return;
    }

    QTextCursor cursor = m_editor->textCursor();
    const QString selected = cursor.selectedText();
    // 只有当选区正好是查找内容时才替换，否则先找到下一个（避免误替换用户的选中文字）
    const bool matched = cursor.hasSelection()
                         && selected.compare(key, caseSensitivity()) == 0
                         && !selected.contains(QChar::ParagraphSeparator);

    if (matched) {
        cursor.insertText(m_editReplace->text());
        m_editor->setTextCursor(cursor);
        m_hint->setText(tr("已替换 1 处"));
    }

    findNext();
}

void ReplaceDialog::replaceAll()
{
    const QString key = searchKey();
    if (key.isEmpty()) {
        m_hint->setText(tr("请输入要查找的内容"));
        m_editFind->setFocus();
        return;
    }

    const QString with = m_editReplace->text();
    QTextDocument::FindFlags flags;
    if (m_caseCheck->isChecked())
        flags |= QTextDocument::FindCaseSensitively;

    int count = 0;
    QTextCursor work(m_editor->document());
    work.beginEditBlock();            // 全部替换合并为一次撤销
    while (true) {
        QTextCursor found = m_editor->document()->find(key, work, flags);
        if (found.isNull())
            break;
        found.insertText(with);
        work = found;                 // 已越过刚插入的文本，替换文本含查找词也不会死循环
        ++count;
    }
    work.endEditBlock();
    m_editor->setTextCursor(work);

    if (count == 0) {
        m_hint->setText(tr("未找到“%1”").arg(key));
    } else {
        m_hint->setText(tr("共替换 %1 处（可一次撤销）").arg(count));
        m_editor->setFocus();
    }
}
