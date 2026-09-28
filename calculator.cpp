#include "calculator.h"
#include "ui_calculator.h"

#include <QKeyEvent>
#include <QPushButton>
#include <cmath>
#include <limits>

Calculator::Calculator(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Calculator)
{
    ui->setupUi(this);

    // ---- 计算状态初始化 ----
    firstOperand       = 0.0;
    pendingOperator    = QString();     // 无等待执行的操作符
    waitingForOperand  = true;          // 下一个数字开始新操作数
    justCalculated     = false;         // 尚未按过等号
    errorState         = false;         // 不处于除零等错误状态

    connectButtons();
}

Calculator::~Calculator()
{
    delete ui;
}

// 统一连接：数字键共用 digitClicked，通过 sender() 区分
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

    connect(ui->btnPoint,     &QPushButton::clicked, this, &Calculator::pointClicked);
    connect(ui->btnAdd,       &QPushButton::clicked, this, &Calculator::operatorClicked);
    connect(ui->btnSub,       &QPushButton::clicked, this, &Calculator::operatorClicked);
    connect(ui->btnMul,       &QPushButton::clicked, this, &Calculator::operatorClicked);
    connect(ui->btnDiv,       &QPushButton::clicked, this, &Calculator::operatorClicked);
    connect(ui->btnEqual,     &QPushButton::clicked, this, &Calculator::equalClicked);
    connect(ui->btnBackspace, &QPushButton::clicked, this, &Calculator::backspaceClicked);
    connect(ui->btnClear,     &QPushButton::clicked, this, &Calculator::clearClicked);
}

// ==================== 按键事件处理 ====================

// 数字键 0~9：共用一个槽
void Calculator::digitClicked()
{
    if (errorState) return;                 // 错误状态下必须先按 C 清除
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    const QString digit = btn->text();

    if (waitingForOperand) {                // 刚按过操作符/等号 → 开始新操作数
        ui->display->setText(digit);
        waitingForOperand = false;
    } else {
        const QString cur = ui->display->text();
        if (cur == "0")                     // 避免 "00"、"007" 这类前导 0
            ui->display->setText(digit);
        else
            ui->display->setText(cur + digit);
    }
}

// 小数点：通过 contains('.') 防止同一操作数出现连续小数点
void Calculator::pointClicked()
{
    if (errorState) return;

    if (waitingForOperand) {                // 操作符后直接按 "." → 补成 "0."
        ui->display->setText("0.");
        waitingForOperand = false;
        return;
    }
    if (!ui->display->text().contains('.'))
        ui->display->setText(ui->display->text() + '.');
}

// 操作符 + - × ÷：双操作数四则运算的核心状态管理
void Calculator::operatorClicked()
{
    if (errorState) return;
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    const QString op = btn->text();

    if (!waitingForOperand) {
        // 正在输入第二个操作数：先结算上一步运算
        if (pendingOperator.isEmpty()) {
            firstOperand = ui->display->text().toDouble();
        } else {
            const double result = calculate(ui->display->text().toDouble(),
                                            pendingOperator);
            if (std::isnan(result)) {       // 上一步除数为 0
                abortOperation();
                return;
            }
            firstOperand = result;
            ui->display->setText(QString::number(result, 'g', 15));
        }
    } else if (pendingOperator.isEmpty()) {
        // 刚按完等号或刚清除：把当前显示值（上次结果）当作第一个操作数
        firstOperand = ui->display->text().toDouble();
    }
    // 其余情况（连续按操作符）→ 只替换操作符，不重复结算

    pendingOperator    = op;
    waitingForOperand  = true;
    justCalculated     = false;
}

// 等号：用当前操作数结算 pendingOperator，结果回显并允许继续运算
void Calculator::equalClicked()
{
    if (errorState || pendingOperator.isEmpty())
        return;                             // 没有完整表达式时等号无效

    const double result = calculate(ui->display->text().toDouble(),
                                    pendingOperator);
    if (std::isnan(result)) {
        abortOperation();
        return;
    }
    ui->display->setText(QString::number(result, 'g', 15));
    firstOperand      = result;             // 结果保留，便于连续运算
    pendingOperator   = QString();
    waitingForOperand = true;
    justCalculated    = true;               // 标记"计算完成"，后续输入重新开始
}

// 退格：仅对"正在输入中的操作数"有效
void Calculator::backspaceClicked()
{
    if (errorState || waitingForOperand)    // 错误状态 / 结果状态下退格无效
        return;

    QString text = ui->display->text();
    text.chop(1);
    if (text.isEmpty() || text == "-") {
        ui->display->setText("0");
        waitingForOperand = true;
    } else {
        ui->display->setText(text);
    }
}

// 清除：回到初始状态
void Calculator::clearClicked()
{
    firstOperand       = 0.0;
    pendingOperator    = QString();
    waitingForOperand  = true;
    justCalculated     = false;
    errorState         = false;
    ui->display->setText("0");
}

// ==================== 内部辅助 ====================

// 执行一步二元运算；除数为 0 时返回 NaN 由调用方转入错误状态
double Calculator::calculate(double rightOperand, const QString &op) const
{
    if (op == "+") return firstOperand + rightOperand;
    if (op == "-") return firstOperand - rightOperand;
    if (op == "×") return firstOperand * rightOperand;
    if (op == "÷") {
        if (rightOperand == 0.0)
            return std::numeric_limits<double>::quiet_NaN();   // 除零 → 错误
        return firstOperand / rightOperand;
    }
    return 0.0;
}

// 进入错误状态：显示提示并锁定，直到按 C 清除
void Calculator::abortOperation()
{
    ui->display->setText("错误：除数为 0");
    pendingOperator    = QString();
    waitingForOperand  = true;
    justCalculated     = false;
    errorState         = true;
}
