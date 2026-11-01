#include "finddialog.h"
#include "textsearch.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>

FindDialog::FindDialog(QPlainTextEdit *editor, QWidget *parent)
    : QDialog(parent)
    , m_editor(editor)
{
    setWindowTitle(tr("查找"));
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    m_editSearch = new QLineEdit(this);
    m_editSearch->setPlaceholderText(tr("请输入要查找的内容"));
    m_editSearch->setMinimumWidth(220);

    m_caseCheck = new QCheckBox(tr("区分大小写"), this);

    auto *btnNext = new QPushButton(tr("查找下一个"), this);
    btnNext->setDefault(true);
    auto *btnPrev = new QPushButton(tr("查找上一个"), this);
    auto *btnClose = new QPushButton(tr("关闭"), this);

    m_hint = new QLabel(this);
    m_hint->setStyleSheet(QStringLiteral("color:#c0392b;"));   // 提示信息用红色小字
    m_hint->setMinimumHeight(18);

    auto *layout = new QGridLayout(this);
    layout->addWidget(new QLabel(tr("查找内容:"), this), 0, 0);
    layout->addWidget(m_editSearch, 0, 1, 1, 3);
    layout->addWidget(m_caseCheck, 1, 1, 1, 3);
    layout->addWidget(m_hint, 2, 0, 1, 4);
    layout->addWidget(btnPrev, 3, 0);
    layout->addWidget(btnNext, 3, 1);
    layout->addWidget(btnClose, 3, 3);
    layout->setColumnStretch(2, 1);

    connect(btnNext, &QPushButton::clicked, this, &FindDialog::findNext);
    connect(btnPrev, &QPushButton::clicked, this, &FindDialog::findPrevious);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::close);
    connect(m_editSearch, &QLineEdit::returnPressed, this, &FindDialog::findNext);

    setTabOrder(m_editSearch, m_caseCheck);
    setTabOrder(m_caseCheck, btnNext);
    setTabOrder(btnNext, btnPrev);
    setTabOrder(btnPrev, btnClose);
}

void FindDialog::setSearchText(const QString &text)
{
    m_editSearch->setText(text);
    m_editSearch->selectAll();
}

void FindDialog::activate()
{
    show();
    raise();
    activateWindow();
    m_editSearch->setFocus();
    m_editSearch->selectAll();
}

void FindDialog::findNext()
{
    doFind(false);
}

void FindDialog::findPrevious()
{
    doFind(true);
}

bool FindDialog::doFind(bool backward)
{
    const QString key = m_editSearch->text();
    if (key.isEmpty()) {
        m_hint->setText(tr("请输入要查找的内容"));
        m_editSearch->setFocus();
        return false;
    }

    bool wrapped = false;
    const bool ok = TextSearch::findNext(m_editor, key, backward,
                                         m_caseCheck->isChecked(), &wrapped);

    if (!ok) {
        m_hint->setText(tr("未找到“%1”").arg(key));
        return false;
    }

    m_editor->setFocus();
    if (wrapped)
        m_hint->setText(tr("已到文件%1，从另一端继续查找")
                            .arg(backward ? tr("开头") : tr("末尾")));
    else
        m_hint->clear();
    return true;
}
