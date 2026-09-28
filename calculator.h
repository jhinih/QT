#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Calculator; }
QT_END_NAMESPACE

class QKeyEvent;

// 带键盘事件的计算器：四则运算 + 小数点 + 退格 + 清除
// 鼠标点击与键盘按键复用同一套槽函数（实验要求 4）
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
    void equalClicked();        // 等号 =
    void backspaceClicked();    // 退格 ⌫
    void clearClicked();        // 清除 C

private:
    void connectButtons();      // 统一连接按钮与槽
    double calculate(double rightOperand, const QString &op) const; // 一步二元运算
    void abortOperation();      // 除零等错误：进入错误状态并锁定

    Ui::Calculator *ui;

    // ---- 计算状态管理（实验步骤三）----
    double  firstOperand        = 0.0;     // 第一个操作数（等号后保存结果）
    QString pendingOperator;               // 等待执行的操作符，"" 表示无
    bool    waitingForOperand   = true;    // true：下一个数字开始新操作数
    bool    justCalculated      = false;   // 刚按过等号，用于"结果后继续输入"
    bool    errorState          = false;   // 除零等错误状态，按 C 解除
};

#endif // CALCULATOR_H
