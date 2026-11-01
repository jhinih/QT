#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "codeeditor.h"
#include "finddialog.h"
#include "replacedialog.h"

#include <QAction>
#include <QCloseEvent>
#include <QColorDialog>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPalette>
#include <QStatusBar>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTime>
#include <QTimer>
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // ---------------- 编辑器：默认等宽字体 + 四周留白
    QFont editorFont(QStringLiteral("Consolas"), 12);
    editorFont.setStyleHint(QFont::Monospace);
    ui->editor->document()->setDefaultFont(editorFont);
    ui->editor->setFont(editorFont);
    ui->editor->document()->setDocumentMargin(8);   // 正文与编辑框边框之间的留白

    // ---------------- 状态栏右侧常驻信息（行、列、字符数）
    m_statusLabel = new QLabel(this);
    statusBar()->addPermanentWidget(m_statusLabel);

    // ---------------- 定时自动保存
    m_autoSaveTimer = new QTimer(this);
    m_autoSaveTimer->setInterval(60 * 1000);        // 每 60 秒检查一次
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::autoSave);

    // ---------------- 文件动作
    connect(ui->actionNew,    &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionOpen,   &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSave,   &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(ui->actionExit,   &QAction::triggered, this, &MainWindow::close);

    // ---------------- 编辑动作：直接复用 QPlainTextEdit 自带的实现
    connect(ui->actionUndo,      &QAction::triggered, ui->editor, &QPlainTextEdit::undo);
    connect(ui->actionRedo,      &QAction::triggered, ui->editor, &QPlainTextEdit::redo);
    connect(ui->actionCut,       &QAction::triggered, ui->editor, &QPlainTextEdit::cut);
    connect(ui->actionCopy,      &QAction::triggered, ui->editor, &QPlainTextEdit::copy);
    connect(ui->actionPaste,     &QAction::triggered, ui->editor, &QPlainTextEdit::paste);
    connect(ui->actionSelectAll, &QAction::triggered, ui->editor, &QPlainTextEdit::selectAll);

    // ---------------- 查找 / 替换 / 关于
    connect(ui->actionFind,    &QAction::triggered, this, &MainWindow::showFindDialog);
    connect(ui->actionReplace, &QAction::triggered, this, &MainWindow::showReplaceDialog);
    connect(ui->actionAbout,   &QAction::triggered, this, &MainWindow::showAbout);

    // ---------------- 格式
    connect(ui->actionFont,        &QAction::triggered, this, &MainWindow::chooseFont);
    connect(ui->actionFontSize,    &QAction::triggered, this, &MainWindow::chooseFontSize);
    connect(ui->actionFontColor,   &QAction::triggered, this, &MainWindow::chooseFontColor);
    connect(ui->actionBackground,  &QAction::triggered, this, &MainWindow::chooseBackgroundColor);

    // ---------------- 视图
    connect(ui->actionLineNumber, &QAction::toggled, this, &MainWindow::toggleLineNumber);
    connect(ui->actionWordWrap,   &QAction::toggled, this, &MainWindow::toggleWordWrap);
    connect(ui->actionAutoSave,   &QAction::toggled, this, &MainWindow::toggleAutoSave);
    connect(ui->actionToolBar,    &QAction::toggled, ui->toolBar,   &QToolBar::setVisible);
    connect(ui->actionStatusBar,  &QAction::toggled, ui->statusbar, &QStatusBar::setVisible);

    // ---------------- 编辑器状态 → 标题 / 状态栏 / 动作使能
    connect(ui->editor->document(), &QTextDocument::modificationChanged,
            this, &MainWindow::updateTitle);
    connect(ui->editor->document(), &QTextDocument::modificationChanged,
            this, &MainWindow::updateEditActions);
    connect(ui->editor, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::updateStatusBar);
    connect(ui->editor, &QPlainTextEdit::textChanged,
            this, &MainWindow::updateStatusBar);
    connect(ui->editor, &QPlainTextEdit::textChanged,
            this, &MainWindow::updateEditActions);
    connect(ui->editor, &QPlainTextEdit::copyAvailable,
            this, &MainWindow::updateEditActions);
    connect(ui->editor->document(), &QTextDocument::undoAvailable,
            this, &MainWindow::updateEditActions);
    connect(ui->editor->document(), &QTextDocument::redoAvailable,
            this, &MainWindow::updateEditActions);

    updateTitle();
    updateEditActions();
    updateStatusBar();
    ui->editor->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ============================================================ 文件操作
void MainWindow::newFile()
{
    if (!maybeSave())
        return;

    ui->editor->clear();
    ui->editor->document()->setModified(false);
    m_filePath.clear();

    updateTitle();
    updateEditActions();
    updateStatusBar();
    statusBar()->showMessage(tr("已新建空白文档"), 2000);
}

void MainWindow::openFile()
{
    if (!maybeSave())
        return;

    const QString startDir = m_filePath.isEmpty()
                                 ? QDir::homePath()
                                 : QFileInfo(m_filePath).absolutePath();
    const QString path = QFileDialog::getOpenFileName(
        this, tr("打开文件"), startDir,
        tr("文本文件 (*.txt *.md *.cpp *.h *.log *.ini *.json);;所有文件 (*.*)"));
    if (path.isEmpty())
        return;

    if (loadFromFile(path))
        statusBar()->showMessage(tr("已打开 %1").arg(QFileInfo(path).fileName()), 3000);
}

bool MainWindow::saveFile()
{
    if (m_filePath.isEmpty())          // 从未保存过：走"另存为"让它选位置
        return saveFileAs();
    return saveToFile(m_filePath);
}

bool MainWindow::saveFileAs()
{
    const QString suggested = m_filePath.isEmpty()
                                  ? QDir::homePath() + QStringLiteral("/未命名.txt")
                                  : m_filePath;
    const QString path = QFileDialog::getSaveFileName(
        this, tr("另存为"), suggested, tr("文本文件 (*.txt);;所有文件 (*.*)"));
    if (path.isEmpty())
        return false;
    return saveToFile(path);
}

bool MainWindow::maybeSave()
{
    if (!ui->editor->document()->isModified())
        return true;

    const auto ret = QMessageBox::warning(
        this, tr("未保存的修改"),
        tr("“%1”已被修改，是否保存？").arg(currentFileName()),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save)
        return saveFile();
    if (ret == QMessageBox::Cancel)
        return false;
    return true;                        // Discard：放弃修改
}

bool MainWindow::loadFromFile(const QString &path)
{
    const QFileInfo info(path);
    if (!info.exists() || !info.isFile()) {
        QMessageBox::warning(this, tr("打开失败"),
                             tr("文件不存在或不是普通文件：\n%1").arg(path));
        return false;
    }

    if (info.size() > 20 * 1024 * 1024) {
        const auto ret = QMessageBox::question(
            this, tr("文件过大"),
            tr("该文件有 %1 MB，可能不是文本文件，仍要打开吗？")
                .arg(info.size() / 1024.0 / 1024.0, 0, 'f', 1),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret != QMessageBox::Yes)
            return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("打开失败"),
                             tr("无法读取文件：\n%1\n原因：%2")
                                 .arg(path, file.errorString()));
        return false;
    }
    const QByteArray data = file.readAll();
    file.close();

    if (data.contains('\0')) {
        QMessageBox::warning(this, tr("打开失败"),
                             tr("这是一个二进制文件，文本编辑器无法打开。"));
        return false;
    }

    // 先按 UTF-8 解码；出现替换字符说明不是 UTF-8，再按本地编码（GBK）解一次
    QString text = QString::fromUtf8(data);
    if (text.contains(QChar::ReplacementCharacter))
        text = QString::fromLocal8Bit(data);

    ui->editor->setPlainText(text);
    ui->editor->document()->setModified(false);
    m_filePath = path;

    updateTitle();
    updateEditActions();
    updateStatusBar();
    ui->editor->setFocus();
    return true;
}

bool MainWindow::saveToFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, tr("保存失败"),
                             tr("无法写入文件：\n%1\n原因：%2")
                                 .arg(path, file.errorString()));
        return false;
    }

    const QByteArray data = ui->editor->toPlainText().toUtf8();
    const qint64 written = file.write(data);
    file.close();

    if (written != data.size()) {
        QMessageBox::warning(this, tr("保存失败"),
                             tr("写入不完整，请检查磁盘空间或文件权限。"));
        return false;
    }

    m_filePath = path;
    ui->editor->document()->setModified(false);
    updateTitle();
    updateEditActions();
    statusBar()->showMessage(tr("已保存到 %1").arg(path), 3000);
    return true;
}

QString MainWindow::currentFileName() const
{
    return m_filePath.isEmpty() ? tr("未命名.txt") : QFileInfo(m_filePath).fileName();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        m_autoSaveTimer->stop();
        event->accept();
    } else {
        event->ignore();                // 用户取消：不关闭窗口
    }
}

// ============================================================ 查找 / 替换 / 关于
void MainWindow::showFindDialog()
{
    if (!m_findDialog)
        m_findDialog = new FindDialog(ui->editor, this);

    // 编辑器里有选中文字时，把它带进查找框，省一次输入
    const QTextCursor cursor = ui->editor->textCursor();
    if (cursor.hasSelection()) {
        const QString selected = cursor.selectedText();
        if (!selected.isEmpty() && !selected.contains(QChar::ParagraphSeparator))
            m_findDialog->setSearchText(selected);
    }
    m_findDialog->activate();
}

void MainWindow::showReplaceDialog()
{
    if (!m_replaceDialog)
        m_replaceDialog = new ReplaceDialog(ui->editor, this);

    const QTextCursor cursor = ui->editor->textCursor();
    if (cursor.hasSelection()) {
        const QString selected = cursor.selectedText();
        if (!selected.isEmpty() && !selected.contains(QChar::ParagraphSeparator))
            m_replaceDialog->setSearchText(selected);
    }
    m_replaceDialog->activate();
}

void MainWindow::showAbout()
{
    QMessageBox::about(
        this, tr("关于 简单文本编辑器"),
        tr("<h3>简单文本编辑器 v1.0</h3>"
           "<p>Qt Widgets 多窗口文本编辑器实验程序。</p>"
           "<p>功能：新建/打开/保存/另存为、撤销与重做、剪切/复制/粘贴/全选、"
           "查找与替换、字体与颜色设置、行号显示、自动换行、定时自动保存。</p>"
           "<p>基于 Qt %1 构建。</p>")
            .arg(QString::fromLatin1(qVersion())));
}

// ============================================================ 格式设置
void MainWindow::chooseFont()
{
    bool ok = false;
    const QFont font = QFontDialog::getFont(&ok, ui->editor->font(), this, tr("选择字体"));
    if (!ok)
        return;

    ui->editor->setFont(font);
    ui->editor->document()->setDefaultFont(font);
    // 字号变化会改变行号栏宽度，重新应用一次行号可见状态以刷新边距
    ui->editor->setLineNumberVisible(ui->actionLineNumber->isChecked());
    statusBar()->showMessage(tr("字体：%1 %2pt").arg(font.family()).arg(font.pointSize()), 3000);
}

void MainWindow::chooseFontSize()
{
    QFont font = ui->editor->font();
    const int current = font.pointSize() > 0 ? font.pointSize() : 12;

    bool ok = false;
    const int size = QInputDialog::getInt(this, tr("字体大小"),
                                          tr("请输入字号（6 ~ 72）："),
                                          current, 6, 72, 1, &ok);
    if (!ok)
        return;

    font.setPointSize(size);
    ui->editor->setFont(font);
    ui->editor->document()->setDefaultFont(font);
    ui->editor->setLineNumberVisible(ui->actionLineNumber->isChecked());
    statusBar()->showMessage(tr("字号：%1 pt").arg(size), 3000);
}

void MainWindow::chooseFontColor()
{
    QColor current = ui->editor->currentCharFormat().foreground().color();
    if (!current.isValid())
        current = ui->editor->palette().color(QPalette::Text);

    const QColor color = QColorDialog::getColor(current, this, tr("选择字体颜色"));
    if (!color.isValid())
        return;

    // mergeCurrentCharFormat：有选中就改选中文字，没选中就改接下来输入的文字，
    // 两种情况用同一个接口，状态不会分叉
    QTextCharFormat format;
    format.setForeground(color);
    ui->editor->mergeCurrentCharFormat(format);

    statusBar()->showMessage(tr("字体颜色：%1").arg(color.name()), 3000);
}

void MainWindow::chooseBackgroundColor()
{
    QPalette palette = ui->editor->palette();
    const QColor color = QColorDialog::getColor(palette.color(QPalette::Base), this,
                                                tr("选择背景颜色"));
    if (!color.isValid())
        return;

    // 深色背景配白字、浅色背景配黑字，避免"看不见字"
    const bool dark = color.lightness() < 128;
    palette.setColor(QPalette::Base, color);
    palette.setColor(QPalette::Text, dark ? QColor(Qt::white) : QColor(Qt::black));
    ui->editor->setPalette(palette);

    statusBar()->showMessage(tr("背景颜色：%1").arg(color.name()), 3000);
}

// ============================================================ 视图
void MainWindow::toggleLineNumber(bool on)
{
    ui->editor->setLineNumberVisible(on);
    statusBar()->showMessage(on ? tr("已显示行号") : tr("已隐藏行号"), 2000);
}

void MainWindow::toggleWordWrap(bool on)
{
    ui->editor->setLineWrapMode(on ? QPlainTextEdit::WidgetWidth
                                   : QPlainTextEdit::NoWrap);
    statusBar()->showMessage(on ? tr("已开启自动换行") : tr("已关闭自动换行"), 2000);
}

void MainWindow::toggleAutoSave(bool on)
{
    if (on) {
        m_autoSaveTimer->start();
        statusBar()->showMessage(tr("已开启定时自动保存（每 60 秒）"), 3000);
    } else {
        m_autoSaveTimer->stop();
        statusBar()->showMessage(tr("已关闭定时自动保存"), 3000);
    }
}

void MainWindow::autoSave()
{
    if (m_filePath.isEmpty()) {
        // 未命名文档没有保存路径，自动保存无从下手，交给用户手动保存
        statusBar()->showMessage(tr("自动保存已跳过：文件尚未保存过一次"), 3000);
        return;
    }
    if (!ui->editor->document()->isModified())
        return;                                  // 没有改动就不用写盘

    if (saveToFile(m_filePath)) {
        statusBar()->showMessage(
            tr("已自动保存 %1").arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss"))),
            3000);
    }
}

// ============================================================ 界面状态刷新
void MainWindow::updateTitle()
{
    // [*] 是 QMainWindow 的修改标记占位符，配合 setWindowModified 显示
    setWindowTitle(tr("%1[*] - 简单文本编辑器").arg(currentFileName()));
    setWindowModified(ui->editor->document()->isModified());
}

void MainWindow::updateStatusBar()
{
    const QTextCursor cursor = ui->editor->textCursor();
    const int line = cursor.blockNumber() + 1;
    const int column = cursor.positionInBlock() + 1;
    const int chars = qMax(0, ui->editor->document()->characterCount() - 1);
    m_statusLabel->setText(tr("行 %1，列 %2    共 %3 字符    UTF-8")
                               .arg(line).arg(column).arg(chars));
}

void MainWindow::updateEditActions()
{
    const QTextDocument *doc = ui->editor->document();
    const bool hasSelection = ui->editor->textCursor().hasSelection();

    ui->actionUndo->setEnabled(doc->isUndoAvailable());
    ui->actionRedo->setEnabled(doc->isRedoAvailable());
    ui->actionCut->setEnabled(hasSelection);
    ui->actionCopy->setEnabled(hasSelection);
    ui->actionPaste->setEnabled(ui->editor->canPaste());
    ui->actionFind->setEnabled(!ui->editor->toPlainText().isEmpty());
    ui->actionReplace->setEnabled(!ui->editor->toPlainText().isEmpty());
}
