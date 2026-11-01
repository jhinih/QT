#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>

class QWidget;
class QPaintEvent;
class QResizeEvent;

/**
 * 从 QPlainTextEdit 派生的编辑器，额外支持左侧行号栏。
 * 在 Qt Designer 中通过「提升为（promote）」把普通的 QPlainTextEdit
 * 提升成 CodeEditor，界面文件不必改动。
 */
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);

    // 行号栏宽度（按总行数位数动态计算）
    int lineNumberAreaWidth() const;

    // 供行号栏控件回调的绘制入口
    void lineNumberAreaPaintEvent(QPaintEvent *event);

    void setLineNumberVisible(bool visible);
    bool isLineNumberVisible() const { return m_lineNumberVisible; }

    // 跳到指定行（1-based），查找结果定位用
    void gotoLine(int line, int column = 0);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);
    void highlightCurrentLine();

private:
    QWidget *m_lineNumberArea = nullptr;
    bool m_lineNumberVisible = true;
};

#endif // CODEEDITOR_H
