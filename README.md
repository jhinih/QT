# 实验2：简单文本编辑器（SimpleTextEditor）

Qt Widgets 实现的多窗口文本编辑器：菜单栏 + 工具栏（Action 分组）+ 带行号的编辑器 + 查找/替换对话框 + 状态栏，支持文件读写、格式设置、自动换行、定时自动保存。

## 目录结构

```
TextEditor/
├── CMakeLists.txt          构建脚本（Qt6 / Qt5 均可）
├── main.cpp                程序入口
├── mainwindow.ui           ★ UI 设计器文件：菜单栏、工具栏、Action、状态栏、layoutMargins
├── mainwindow.h / .cpp     主窗口逻辑：文件、格式、视图、状态栏、动作使能
├── codeeditor.h / .cpp     ★ 从 QPlainTextEdit 派生的带行号编辑器（Designer 中 promote 使用）
├── finddialog.h / .cpp     查找对话框（未找到 / 连续查找 / 回绕）
├── replacedialog.h / .cpp  替换对话框（单条替换 / 全部替换 / 死循环防护）
└── textsearch.h / .cpp     查找算法（两个对话框共用，单一实现）
```

## 构建与运行

> ⚠️ **重要：工程必须放在纯英文路径下构建！**
> 中文路径（如 `实验2/TextEditor`）会让 Qt 的 moc / CMake AUTOMOC 失败，
> 报错形如 `TextEditor_autogen/timestamp Error 1`、`The info file ... is not readable`。
> 实测可用的英文路径副本：`D:\QT\TextEditor`（Qt 6.11.2 + MinGW 13.1.0 + CMake 3.30.5 编译通过）。

Qt Creator：`文件 → 打开文件或项目 → CMakeLists.txt`，选好 Kit 后 Ctrl+R。

命令行（Qt 6.11.2 + CMake + MinGW 实测路径）：

```bash
cmake -B build -S .
cmake --build build
./build/TextEditor        # Windows: build\TextEditor.exe
```

## 功能一览

| 分组 | 功能 | 入口 |
|---|---|---|
| 文件 | 新建 / 打开 / 保存 / 另存为 / 退出 | 文件菜单、工具栏、Ctrl+N/O/S/Shift+S/Q |
| 编辑 | 撤销 / 重做 / 剪切 / 复制 / 粘贴 / 全选 | 编辑菜单、工具栏、Ctrl+Z/Y/X/C/V/A |
| 查找 | 查找下一个 / 上一个、区分大小写、未找到提示、到末尾自动回绕 | Ctrl+F |
| 替换 | 单条替换 / 全部替换（合并为一次撤销）、区分大小写 | Ctrl+H |
| 格式 | 字体、字体大小（6~72）、字体颜色（有选区只改选区）、背景颜色（自动配黑/白字） | 格式菜单 |
| 视图 | 显示行号、自动换行、显示工具栏、显示状态栏、定时自动保存（60 s） | 视图菜单 |
| 状态栏 | 行、列、字符数、临时提示（已保存 / 自动保存时间 / 未找到…） | — |
| 其他 | 标题栏随修改状态显示 `*`、关闭前提示保存、二进制文件与超大文件拦截 | — |

## 关键技术点（写实验报告可直接对照）

| 概念 | 在哪个文件 | 说明 |
|---|---|---|
| Action 编辑器 | `mainwindow.ui` | 22 个 `<action>`，带 text / shortcut / checkable；工具栏用 `<addaction>` 引用同一批 Action，菜单与工具栏天然同步 |
| UI 设计器 + 提升（promote） | `mainwindow.ui` 的 `<customwidgets>` | 普通 `QPlainTextEdit` 提升为 `CodeEditor`，界面文件不用手写控件 |
| layoutMargins | `mainwindow.ui` | 中央 widget 的 `leftMargin/topMargin=8`，配合 `documentMargin(8)` 做四周留白 |
| 派生控件 | `codeeditor.*` | 行号栏 `LineNumberArea` 挂在 `setViewportMargins` 的左边距里，随滚动同步重绘 |
| 信号与槽 | `mainwindow.cpp` 构造函数 | 编辑动作直接连 `QPlainTextEdit` 自带槽（undo/cut/copy…），不重复写实现 |
| 动作使能 | `updateEditActions()` | 由 `copyAvailable` / `undoAvailable` / `redoAvailable` / `modificationChanged` 驱动 |
| 修改状态与标题 | `updateTitle()` | `setWindowTitle("%1[*] - 简单文本编辑器")` + `setWindowModified()` |
| 文件读写 | `loadFromFile()` / `saveToFile()` | UTF-8 优先、失败回落 GBK；拦截二进制与 >20 MB 文件 |
| 定时器 | `autoSave()` | `QTimer` 每 60 s 检查，仅在有修改且已有路径时落盘 |

## 手动测试清单

| 场景 | 预期现象 |
|---|---|
| 新建后直接关闭窗口 | 弹出"是否保存"，选取消则不关闭 |
| 修改后标题栏 | 文件名后出现 `*`，保存后消失 |
| 打开不存在的文件路径 | 提示"文件不存在或不是普通文件"，原文档不被清空 |
| 打开一个 `.png` / `.exe` | 提示"二进制文件，文本编辑器无法打开" |
| 打开 GBK 编码的 txt | 正常显示中文（UTF-8 解码失败后回落本地编码） |
| 未保存过的文档按 Ctrl+S | 自动转为"另存为"对话框 |
| 无选中文字 | 剪切/复制灰显；选中后恢复 |
| 无可撤销内容 | 撤销灰显；输入后恢复 |
| 查找不存在的词 | 红字提示"未找到 xxx"，文档不跳动 |
| 从中间连续查找同一个词 | 依次命中，到末尾后提示"已到文件末尾，从另一端继续查找" |
| 把 "a" 全部替换为 "aa" | 正常结束、不卡死，提示共替换 N 处 |
| 全部替换后按一次 Ctrl+Z | 一次性还原全部替换 |
| 关闭自动换行后输入超长行 | 出现水平滚动条，不折行 |
| 隐藏行号 / 工具栏 / 状态栏 | 对应区域消失，菜单勾选状态同步 |
| 深色背景 | 文字自动转为白色，仍可读 |
