#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Calculator; }
QT_END_NAMESPACE

class QKeyEvent;

// 带键盘事件的计算器（v2：追加括号与上下双显示）
// 上行 exprDisplay 显示完整表达式（含括号），下行 display 显示当前输入/结果
// 支持 + − × ÷ 与括号 ()，键盘与鼠标复用同一套槽函数
class Calculator : public QWidget
{
    Q_OBJECT

public:
    explicit Calculator(QWidget *parent = nullptr);
    ~Calculator();

protected:
    void keyPressEvent(QKeyEvent *event) override;   // 键盘事件入口

private slots:
    void digitClicked();        // 数字键 0~9（共用，sender() 区分）
    void pointClicked();        // 小数点
    void operatorClicked();     // 操作符 + - × ÷
    void lParenClicked();       // 左括号 (
    void rParenClicked();       // 右括号 )
    void equalClicked();        // 等号 =（整表达式求值）
    void backspaceClicked();    // 退格 ⌫
    void clearClicked();        // 清除 C

private:
    void connectButtons();      // 统一连接按钮与槽
    void updateDisplays();      // 上行 = expr + current，下行 = 当前操作数
    void enterError(const QString &reason);   // 进入错误状态并锁定
    double evaluate(const QString &src, bool *ok, QString *reason) const; // 调度场算法求值

    Ui::Calculator *ui;

    // ---- 表达式状态（v2）----
    QString expr;                   // 已确认的表达式（操作符、括号、已提交的操作数）
    QString current;                // 当前正在输入的操作数（可为空）
    bool    justEvaluated = false;  // 刚按过等号，用于"结果后继续输入"
    bool    errorState    = false;  // 除零/括号不匹配等错误状态，按 C 解除
};

#endif // CALCULATOR_H
