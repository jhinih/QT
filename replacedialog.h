#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QLabel;
class QPlainTextEdit;

/**
 * 替换对话框（非模态）。
 * 支持"替换（单条）"与"全部替换"，并处理：未找到、当前选区不匹配时先向后找、
 * 替换文本包含查找文本时的死循环防护。
 */
class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(QPlainTextEdit *editor, QWidget *parent = nullptr);

    void setSearchText(const QString &text);
    void activate();

public slots:
    void findNext();
    void replaceOne();
    void replaceAll();

private:
    QString searchKey() const;
    Qt::CaseSensitivity caseSensitivity() const;

    QPlainTextEdit *m_editor = nullptr;
    QLineEdit *m_editFind = nullptr;
    QLineEdit *m_editReplace = nullptr;
    QCheckBox *m_caseCheck = nullptr;
    QLabel *m_hint = nullptr;
};

#endif // REPLACEDIALOG_H
