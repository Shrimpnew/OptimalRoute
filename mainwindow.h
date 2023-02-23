#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QProgressBar>
#include <QGraphicsDropShadowEffect>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Node;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    Node *getSelectedNode();

private slots:
    void on_actionSelect_triggered();

    void on_actionAddPath_triggered();

    void on_nodeList_customContextMenuRequested(const QPoint &pos);

    void on_pathList_customContextMenuRequested(const QPoint &pos);

    void on_actionSetStartNode_triggered();

    void on_actionSetEndNode_triggered();

    void on_actionDeletePath_triggered();

    void on_actionNewFile_triggered();

    void on_actionOpenFile_triggered();

    void on_actionSaveFile_triggered();

    void on_actionSaveFileAs_triggered();

    void on_action_openObjectManager_triggered();

    void on_action_openConsole_triggered();

    void on_action_openOutput_triggered();

    void on_action_batchQuery_triggered();

    void on_selectButton_clicked();

    void on_addButton_clicked();

    void on_closeButton_clicked();

    void on_maximumButton_clicked();

    void on_minimumButton_clicked();

private:
    Ui::MainWindow *ui;
    QMenu node_menu;
    QMenu path_menu;
    QMenu file_menu;
    QMenu tool_menu;
    QMenu run_menu;
};

class TitleWidget : public QWidget{
public:
    TitleWidget(QWidget *parent = nullptr);
    void setWindow(QWidget *newWindow);
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
private:
    QWidget *window;
    QPoint offset;
};

#endif // MAINWINDOW_H
