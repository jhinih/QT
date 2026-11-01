#ifndef TEXTSEARCH_H
#define TEXTSEARCH_H

#include <QString>

class QPlainTextEdit;

/**
 * 查找/替换对话框共用的查找算法（单一实现，避免两处逻辑不一致）：
 * 从当前光标或选区继续查找；到文档末尾（或开头）没找到时自动回绕到另一端。
 */
namespace TextSearch {

// 返回是否找到；找到时编辑器选区已定位到命中处。
// wrapped 输出是否发生了回绕（用于提示"已到文件末尾…"）。
bool findNext(QPlainTextEdit *editor,
              const QString &key,
              bool backward,
              bool caseSensitive,
              bool *wrapped = nullptr);

} // namespace TextSearch

#endif // TEXTSEARCH_H
