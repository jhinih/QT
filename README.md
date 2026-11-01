# QT 课程实验合集

东莞理工学院 24 级软件工程（卓越计划）《Qt 程序设计》课程实验源码仓库。

三个工程都用 **Qt Widgets + C++17** 实现，除教材示例 `samp2_4App` 用 qmake 外，其余两个均用 **CMake** 构建。

## 工程一览

| 目录 | 工程 | 构建方式 | 说明 |
|---|---|---|---|
| `exp1-calculator/` | Calculator | CMake | 实验1：带键盘事件的计算器，表达式求值（调度场算法），鼠标/键盘双输入 |
| `exp2-texteditor/` | TextEditor | CMake | 实验2：简单文本编辑器，菜单/工具栏/Action、行号编辑器、查找替换、定时自动保存 |
| `samp2_4App/` | samp2_4 | qmake | 教材第 2 章示例：主窗口 + 状态栏进度条 + 字体工具栏 |
| `docs/` | — | — | 实验1、实验2 的实验报告（.docx） |

各工程的详细功能、测试要点与关键技术点见各自目录下的 `README.md`。

## 环境

实测通过的环境：

| 组件 | 版本 |
|---|---|
| Qt | 6.11.2（mingw_64） |
| 编译器 | MinGW 13.1.0（x86_64-posix-seh） |
| CMake | 3.30.5 |
| Ninja | 1.12.1 |

> ⚠️ **路径必须纯英文**。Qt 的 moc / CMake AUTOMOC 在中文路径下会失败
> （`*_autogen/timestamp Error 1`、`The info file ... is not readable`）。
> 所以本仓库所有目录名均为英文，克隆时也请放在纯英文路径下（例如 `D:\code\QT`）。

## 构建与运行

### exp1-calculator / exp2-texteditor（CMake）

```bash
cd exp1-calculator          # 或 exp2-texteditor
cmake -B build -S .
cmake --build build
./build/Calculator.exe      # exp2 为 ./build/TextEditor.exe
```

Qt Creator：`文件 → 打开文件或项目 → 选中 CMakeLists.txt`，选好 Kit 后 Ctrl+R。

### samp2_4App（qmake）

```bash
cd samp2_4App
mkdir -p build && cd build
qmake ../samp2_4.pro
mingw32-make -j4
./release/samp2_4.exe
```

## 仓库约定

- 只提交源码、`.ui`、`.qrc`、说明文档；`build/`、`*.user`、`.qtcreator/`、`moc_*`、`ui_*.h` 等生成物全部由根目录 `.gitignore` 排除。
- 分支：`main` 为主干；每个实验在独立分支开发（如 `exp1/keyboard-calculator`）。

## 目录结构

```
QT/
├── README.md                 本文件
├── .gitignore                构建产物忽略规则
├── exp1-calculator/          实验1 计算器
├── exp2-texteditor/          实验2 文本编辑器
├── samp2_4App/               教材示例
└── docs/                     实验报告
```
