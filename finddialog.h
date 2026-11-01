#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QLabel;
class QPlainTextEdit;

/**
 * 查找对话框（非模态，可一直显示在编辑器旁边）。
 * 查找逻辑本身放在对话框里：统一处理「未找到」「连续查找（到末尾/开头自动回绕）」。
 */
class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QPlainTextEdit *editor, QWidget *parent = nullptr);

    // 把编辑器里选中的文字带进查找框
    void setSearchText(const QString &text);
    // 显示并把焦点放到输入框
    void activate();

public slots:
    void findNext();
    void findPrevious();

private:
    bool doFind(bool backward);

    QPlainTextEdit *m_editor = nullptr;
    QLineEdit *m_editSearch = nullptr;
    QCheckBox *m_caseCheck = nullptr;
    QLabel *m_hint = nullptr;
};

#endif // FINDDIALOG_H
