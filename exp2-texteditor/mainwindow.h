#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QLabel;
class QTimer;
class QCloseEvent;
class FindDialog;
class ReplaceDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 文件
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();

    // 对话框
    void showFindDialog();
    void showReplaceDialog();
    void showAbout();

    // 格式
    void chooseFont();
    void chooseFontSize();
    void chooseFontColor();
    void chooseBackgroundColor();

    // 视图
    void toggleLineNumber(bool on);
    void toggleWordWrap(bool on);
    void toggleAutoSave(bool on);
    void autoSave();

    // 界面状态
    void updateStatusBar();
    void updateEditActions();
    void updateTitle();

private:
    bool maybeSave();                            // 有未保存修改时询问
    bool loadFromFile(const QString &path);
    bool saveToFile(const QString &path);
    QString currentFileName() const;

    Ui::MainWindow *ui;
    QLabel *m_statusLabel = nullptr;
    QTimer *m_autoSaveTimer = nullptr;
    QString m_filePath;                          // 空表示尚未保存过（未命名）
    FindDialog *m_findDialog = nullptr;
    ReplaceDialog *m_replaceDialog = nullptr;
};

#endif // MAINWINDOW_H
