#include "calculator.h"
#include "ui_calculator.h"

#include <QKeyEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QList>

// 样式表：模拟手机计算器深色配色（实验步骤五）
static const char *kStyleSheet = R"(
Calculator { background-color: #2b2b2b; }
QLineEdit#exprDisplay {
    background-color: #1e1e1e;
    color: #9a9a9a;
    border: none;
    padding: 2px 8px;
}
QLineEdit#display {
    background-color: #1e1e1e;
    color: #ffffff;
    border: none;
    padding: 8px;
}
QPushButton {
    background-color: #4e4e4e;
    color: #ffffff;
    border: none;
    border-radius: 24px;
    font-size: 16px;
}
QPushButton:hover  { background-color: #5e5e5e; }
QPushButton:pressed{ background-color: #7a7a7a; }
QPushButton#btnClear, QPushButton#btnBackspace,
QPushButton#btnLParen, QPushButton#btnRParen {
    background-color: #a5a5a5; color: #000000;
}
QPushButton#btnDiv, QPushButton#btnMul,
QPushButton#btnSub, QPushButton#btnAdd, QPushButton#btnEqual {
    background-color: #ff9f0a;
}
)";

Calculator::Calculator(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Calculator)
{
    ui->setupUi(this);
    setStyleSheet(kStyleSheet);   // 统一按键外观

    // 按键不接收焦点：否则按钮获得焦点后，Enter/空格会直接触发按钮点击，
    // 与 keyPressEvent 重复处理同一按键（事件被处理两次的问题）
    for (QPushButton *btn : findChildren<QPushButton *>())
        btn->setFocusPolicy(Qt::NoFocus);

    connectButtons();
    updateDisplays();
}

Calculator::~Calculator()
{
    delete ui;
}

// 统一连接：数字键共用 digitClicked，鼠标点击与键盘 keyPressEvent
// 最终都走到这里，保证两套输入逻辑完全一致（实验要求 4）
void Calculator::connectButtons()
{
    connect(ui->btn0, &QPushButton::clicked, this, &Calculator::digitClicked);
    connect(ui->btn1, &QPushButton::clicked, this, &Calculator::digitClicked);
    connect(ui->btn2, &QPushButton::clicked, this, &Calculator::digitClicked);
    connect(ui->btn3, &QPushButton::clicked, this, &Calculator::digitClicked);
    connect(ui->btn4, &QPushButton::clicked, this, &Calculator::digitClicked);
    connect(ui->btn5, &QPushButton::clicked, this, &Calculator::digitClicked);
    connect(ui->btn6, &QPushButton::clicked, this, &Calculator::digitClicked);
    connect(ui->btn7, &QPushButton::clicked, this, &Calculator::digitClicked);
    connect(ui->btn8, &QPushButton::clicked, this, &Calculator::digitClicked);
    connect(ui->btn9, &QPushButton::clicked, this, &Calculator::digitClicked);

    connect(ui->btnPoint,   &QPushButton::clicked, this, &Calculator::pointClicked);
    connect(ui->btnAdd,     &QPushButton::clicked, this, &Calculator::operatorClicked);
    connect(ui->btnSub,     &QPushButton::clicked, this, &Calculator::operatorClicked);
    connect(ui->btnMul,     &QPushButton::clicked, this, &Calculator::operatorClicked);
    connect(ui->btnDiv,     &QPushButton::clicked, this, &Calculator::operatorClicked);
    connect(ui->btnLParen,  &QPushButton::clicked, this, &Calculator::lParenClicked);
    connect(ui->btnRParen,  &QPushButton::clicked, this, &Calculator::rParenClicked);
    connect(ui->btnEqual,   &QPushButton::clicked, this, &Calculator::equalClicked);
    connect(ui->btnBackspace, &QPushButton::clicked, this, &Calculator::backspaceClicked);
    connect(ui->btnClear,     &QPushButton::clicked, this, &Calculator::clearClicked);
}

// ==================== 显示 ====================

// 上行显示"已确认表达式 + 正在输入的操作数"，下行显示当前操作数（空则显示 0）
void Calculator::updateDisplays()
{
    ui->exprDisplay->setText(expr + current);
    ui->display->setText(current.isEmpty() ? QStringLiteral("0") : current);
}

// ==================== 按键事件处理 ====================

// 数字键 0~9：共用一个槽，通过 sender() 区分
void Calculator::digitClicked()
{
    if (errorState) return;                 // 错误状态下必须先按 C 清除
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    const QString digit = btn->text();

    if (justEvaluated) {                    // 结果后按数字 → 开始新一轮计算
        expr.clear();
        current.clear();
        justEvaluated = false;
    } else if (expr.endsWith(')')) {        // ")5" 之类 → 补隐式乘号 "×5"
        expr += "×";
    }
    current = (current == "0") ? digit : current + digit;   // 避免前导 0
    updateDisplays();
}

// 小数点：contains('.') 防止同一操作数出现连续小数点
void Calculator::pointClicked()
{
    if (errorState) return;
    if (justEvaluated) {
        expr.clear(); current.clear(); justEvaluated = false;
    } else if (expr.endsWith(')')) {
        expr += "×";                        // ")". 之后按 "." → 隐式乘法
    }
    if (current.isEmpty() || current == "0")
        current = "0.";                     // 操作符后/开头直接按 "." → "0."
    else if (!current.contains('.'))
        current += '.';
    updateDisplays();
}

// 操作符 + - × ÷
void Calculator::operatorClicked()
{
    if (errorState) return;
    const QString op = qobject_cast<QPushButton *>(sender())->text();

    if (justEvaluated) {                    // 结果续算：结果作为表达式开头
        expr = current;
        current.clear();
        justEvaluated = false;
    } else if (!current.isEmpty()) {        // 提交当前操作数到表达式
        expr += current;
        current.clear();
    } else if (!expr.isEmpty()
               && QStringLiteral("+-×÷").contains(expr.right(1))) {
        expr.chop(1);                       // 连续按操作符 → 替换操作符
    } else if (expr.isEmpty() || expr.endsWith('(')) {
        return;                             // 空表达式 / "(" 后不允许直接跟操作符
    }
    expr += op;
    updateDisplays();
}

// 左括号 (：操作数/结果之后按 ( 自动补隐式乘号
void Calculator::lParenClicked()
{
    if (errorState) return;
    if (justEvaluated) {                    // 结果后按 ( → 结果 × (
        expr = current + "×";
        current.clear();
        justEvaluated = false;
    } else if (!current.isEmpty()) {        // 操作数后按 ( → 隐式乘法
        expr += current + "×";
        current.clear();
    } else if (!expr.isEmpty() && expr.endsWith(')')) {
        expr += "×";                        // ")(" → ")×("
    }
    expr += "(";
    updateDisplays();
}

// 右括号 )：必须有未配对的 ( 才有效；先提交当前操作数
void Calculator::rParenClicked()
{
    if (errorState) return;
    const int depth = expr.count('(') - expr.count(')');
    if (depth <= 0) return;                 // 没有 ( 可配对
    if (!current.isEmpty()) {
        expr += current;
        current.clear();
    } else if (!expr.isEmpty()
               && QStringLiteral("+-×÷(").contains(expr.right(1))) {
        return;                             // 操作符或 "(" 后不允许空括号 ")"
    }
    expr += ")";
    updateDisplays();
}

// 等号：对"expr + current"整表达式求值（支持优先级与括号）
void Calculator::equalClicked()
{
    if (errorState) return;
    if (current.isEmpty() && expr.isEmpty()) return;

    QString full = expr + current;
    // 末尾悬空的操作符直接丢弃（如 "3+" = 3）
    while (!full.isEmpty()
           && QStringLiteral("+-×÷").contains(full.right(1)))
        full.chop(1);
    // 自动补全未闭合的左括号；多出的 ) 判为不匹配
    const int depth = full.count('(') - full.count(')');
    if (depth > 0)      full += QString(depth, ')');
    else if (depth < 0) { enterError("括号不匹配"); return; }

    bool ok = false; QString reason;
    const double result = evaluate(full, &ok, &reason);
    if (!ok) { enterError(reason); return; }

    const QString resultStr = QString::number(result, 'g', 15);
    ui->exprDisplay->setText(full + " =");  // 上行定格完整算式
    ui->display->setText(resultStr);        // 下行显示结果
    expr.clear();
    current = resultStr;                    // 结果保留，支持续算
    justEvaluated = true;
}

// 退格：优先编辑当前操作数；当前为空时从表达式末尾回删
void Calculator::backspaceClicked()
{
    if (errorState) return;
    if (justEvaluated) {                    // 结果状态下退格 → 变回可编辑
        justEvaluated = false;
        expr.clear();
    }
    if (!current.isEmpty()) {
        current.chop(1);
    } else if (!expr.isEmpty()) {
        const QChar last = expr.back();
        if (last.isDigit() || last == '.') {    // 末尾是已提交的操作数 → 拿回来继续编辑
            int j = expr.size();
            while (j > 0 && (expr[j - 1].isDigit() || expr[j - 1] == '.'))
                --j;
            current = expr.mid(j);
            expr = expr.left(j);
        } else {                                // 末尾是操作符/括号 → 直接删除
            expr.chop(1);
        }
    }
    updateDisplays();
}

// 清除：回到初始状态
void Calculator::clearClicked()
{
    expr.clear();
    current.clear();
    justEvaluated = false;
    errorState    = false;
    updateDisplays();
}

// ==================== 表达式求值 ====================

// 调度场算法：中缀表达式 → 后缀表达式 → 求值
// ok=false 时 reason 给出错误原因（除数为 0 / 括号不匹配 / 表达式错误）
double Calculator::evaluate(const QString &src, bool *ok, QString *reason) const
{
    *ok = false;
    struct Tok { enum Kind { NUM, OP, LP, RP } kind; double v = 0; QChar op; };
    QList<Tok> toks;

    // 1. 词法分析：数字（含小数）、四则操作符、括号
    int i = 0;
    while (i < src.size()) {
        const QChar c = src[i];
        if (c.isDigit() || c == '.') {
            int j = i;
            while (j < src.size() && (src[j].isDigit() || src[j] == '.')) ++j;
            bool numOk = false;
            const double v = src.mid(i, j - i).toDouble(&numOk);
            if (!numOk) { *reason = "表达式错误"; return 0; }
            toks.append({Tok::NUM, v, QChar()});
            i = j;
        } else if (c == '+' || c == '-' || c == '×' || c == '÷') {
            toks.append({Tok::OP, 0, c});
            ++i;
        } else if (c == '(') {
            toks.append({Tok::LP, 0, QChar()});
            ++i;
        } else if (c == ')') {
            toks.append({Tok::RP, 0, QChar()});
            ++i;
        } else if (c == ' ') {
            ++i;                                // 忽略空格
        } else {
            *reason = "表达式错误"; return 0;
        }
    }
    if (toks.isEmpty()) { *reason = "表达式错误"; return 0; }

    // 2. 中缀 → 后缀：× ÷ 优先级高于 + −，同级左结合
    QList<Tok> rpn, ops;
    const auto prec = [](const Tok &t) {
        return (t.op == '+' || t.op == '-') ? 1 : 2;
    };
    for (const Tok &t : toks) {
        if (t.kind == Tok::NUM) {
            rpn.append(t);
        } else if (t.kind == Tok::OP) {
            while (!ops.isEmpty() && ops.last().kind == Tok::OP
                   && prec(ops.last()) >= prec(t))
                rpn.append(ops.takeLast());
            ops.append(t);
        } else if (t.kind == Tok::LP) {
            ops.append(t);
        } else {                                // RP：弹栈直到对应的 (
            while (!ops.isEmpty() && ops.last().kind != Tok::LP)
                rpn.append(ops.takeLast());
            if (ops.isEmpty()) { *reason = "括号不匹配"; return 0; }
            ops.takeLast();
        }
    }
    while (!ops.isEmpty()) {
        if (ops.last().kind == Tok::LP) { *reason = "括号不匹配"; return 0; }
        rpn.append(ops.takeLast());
    }

    // 3. 后缀求值；除数为 0 视为错误而不是 inf
    QList<double> st;
    for (const Tok &t : rpn) {
        if (t.kind == Tok::NUM) { st.append(t.v); continue; }
        if (st.size() < 2) { *reason = "表达式错误"; return 0; }
        const double b = st.takeLast();
        const double a = st.takeLast();
        switch (t.op.unicode()) {
        case '+': st.append(a + b); break;
        case '-': st.append(a - b); break;
        case '×': st.append(a * b); break;
        default:
            if (b == 0.0) { *reason = "除数为 0"; return 0; }
            st.append(a / b);
        }
    }
    if (st.size() != 1) { *reason = "表达式错误"; return 0; }
    *ok = true;
    return st.first();
}

// ==================== 错误状态 ====================

// 进入错误状态：显示提示并锁定，直到按 C 清除
void Calculator::enterError(const QString &reason)
{
    ui->exprDisplay->setText(QString());
    ui->display->setText("错误：" + reason);
    expr.clear();
    current.clear();
    justEvaluated = false;
    errorState    = true;
}

// ==================== 键盘事件处理 ====================
// 思路：把按键映射到对应 QPushButton，调用 click()，
// 完全复用鼠标点击的那一套槽函数，避免逻辑重复（实验要求 4）
void Calculator::keyPressEvent(QKeyEvent *event)
{
    const int key = event->key();

    // 错误状态下仅允许 C / Del / Esc 清除
    if (errorState && key != Qt::Key_C && key != Qt::Key_Delete
                   && key != Qt::Key_Escape) {
        event->accept();
        return;
    }

    // 括号：Shift+9 / Shift+0 产生的字符事件，也兼容中文输入法的（）以外的英文括号
    if (event->text() == "(") { ui->btnLParen->click(); event->accept(); return; }
    if (event->text() == ")") { ui->btnRParen->click(); event->accept(); return; }

    switch (key) {
    case Qt::Key_0: ui->btn0->click(); break;
    case Qt::Key_1: ui->btn1->click(); break;
    case Qt::Key_2: ui->btn2->click(); break;
    case Qt::Key_3: ui->btn3->click(); break;
    case Qt::Key_4: ui->btn4->click(); break;
    case Qt::Key_5: ui->btn5->click(); break;
    case Qt::Key_6: ui->btn6->click(); break;
    case Qt::Key_7: ui->btn7->click(); break;
    case Qt::Key_8: ui->btn8->click(); break;
    case Qt::Key_9: ui->btn9->click(); break;

    case Qt::Key_Period:
    case Qt::Key_Comma:          ui->btnPoint->click();     break;

    case Qt::Key_Plus:           ui->btnAdd->click();       break;
    case Qt::Key_Minus:          ui->btnSub->click();       break;
    case Qt::Key_Asterisk:       ui->btnMul->click();       break;
    case Qt::Key_Slash:          event->accept();
                                 ui->btnDiv->click();       break;

    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Equal:          ui->btnEqual->click();     break;

    case Qt::Key_Backspace:      ui->btnBackspace->click(); break;
    case Qt::Key_Delete:
    case Qt::Key_Escape:
    case Qt::Key_C:              ui->btnClear->click();     break;

    default:                     QWidget::keyPressEvent(event); return;
    }
    event->accept();
}
