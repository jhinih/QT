# 实验1：带键盘事件的计算器

Qt Widgets 实现的四则运算计算器，支持鼠标点击与键盘输入，两套输入复用同一套槽函数。

## 功能
- 四则运算（+ − × ÷）、小数输入、退格（⌫）、清除（C）
- 异常输入处理：连续小数点、连续操作符、前导 0、除数为 0、计算完成后继续输入
- 键盘输入：数字 0~9、`.`/`,`、`+` `-` `*` `/`、`Enter`/`=`、`Backspace`、`C`/`Del`/`Esc`

## 构建（Qt 6.9.2 + CMake）
```bash
cmake -B build -S .
cmake --build build
./build/Calculator   # Windows: build\Calculator.exe
```
