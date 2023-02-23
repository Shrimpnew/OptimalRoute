#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMouseEvent>
#include <QFileDialog>
#include <QStandardPaths>
#include <QGraphicsDropShadowEffect>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      node_menu(),
      path_menu(),
      file_menu(),
      tool_menu(),
      run_menu()
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    QMainWindow::setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    QMainWindow::setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    ui->menuWidget->setWindow(this->window());
    delete ui->topDockWidget->titleBarWidget();
    ui->topDockWidget->setTitleBarWidget(new QWidget());
    ui->objectManager->setCurrentIndex(0);
    ui->consoleTabs->setCurrentIndex(0);
    ui->propertyStackedWidget->setCurrentIndex(2);
    GlobalVar::graph_view = ui->graphView;
    GlobalVar::console_tabs = ui->consoleTabs;
    GlobalVar::stategy_box = ui->strategyComboBox;
    GlobalVar::property_stacks = ui->propertyStackedWidget;
    GlobalVar::nodename_edit = ui->nodeNameLineEdit;
    GlobalVar::pathname_edit = ui->pathNameLineEdit;
    GlobalVar::price_box = ui->pathPriceDoubleSpinBox;
    GlobalVar::time_box = ui->pathTimeDoubleSpinBox;
    GlobalVar::speed_box = ui->pathSpeedDoubleSpinBox;
    GlobalVar::node_filter = ui->nodeFilter;
    GlobalVar::node_list = ui->nodeList;
    GlobalVar::path_filter = ui->pathFilter;
    GlobalVar::path_list = ui->pathList;
    GlobalVar::output_list = ui->outputWidget;
    connect(ui->nodeFilter, &QLineEdit::textChanged, ui->graphView, &GraphView::nodeFilter);
    connect(ui->nodeList, &QListWidget::itemClicked, ui->graphView, &GraphView::showListItem);
    connect(ui->pathFilter, &QLineEdit::textChanged, ui->graphView, &GraphView::pathFilter);
    connect(ui->pathList, &QTreeWidget::itemClicked, ui->graphView, &GraphView::showTreeItem);
    connect(ui->outputWidget, &QTreeWidget::itemClicked, ui->graphView, &GraphView::showOutputItem);
    connect(ui->swapNodeLabel, &ClickLabel::click_left, ui->graphView, &GraphView::swapStartEndNode);
    connect(ui->graphView, &GraphView::startNodeChanged, ui->startNode, &QLabel::setText);
    connect(ui->graphView, &GraphView::endNodeChanged, ui->endNode, &QLabel::setText);
    connect(ui->setStartNodeButton, SIGNAL(clicked(bool)), ui->graphView, SLOT(setStartNode()));
    connect(ui->setEndNodeButton, SIGNAL(clicked(bool)), ui->graphView, SLOT(setEndNode()));
    connect(ui->queryButton, &QPushButton::clicked, ui->graphView, &GraphView::queryRoute);
    node_menu.addAction(ui->actionSetStartNode);
    node_menu.addAction(ui->actionSetEndNode);
    path_menu.addAction(ui->actionDeletePath);
    file_menu.addAction(ui->actionNewFile);
    file_menu.addAction(ui->actionOpenFile);
    file_menu.addAction(ui->actionSaveFile);
    file_menu.addAction(ui->actionSaveFileAs);
    file_menu.setWindowFlags(file_menu.windowFlags()  | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    file_menu.setAttribute(Qt::WA_TranslucentBackground);
    file_menu.setStyleSheet("QMenu{"
                            "background-color: rgb(255, 255, 255);"
                            "border:1px solid rgb(176, 218, 255);"
                            "border-radius:8px;"
                            "}"
                            "QMenu::item{"
                            "background-color:transparent;"
                            "height:25px;"
                            "min-width:100px;"
                            "padding-left:10px;"
                            "font-size:15px;"
                            "border-radius:8px;"
                            "}"
                            "QMenu::item:selected{"
                            "background-color: rgb(176, 218, 255);"
                            "}");
    ui->fileButton->setMenu(&file_menu);
    tool_menu.addAction(ui->action_openObjectManager);
    tool_menu.addAction(ui->action_openConsole);
    tool_menu.addAction(ui->action_openOutput);
    tool_menu.setWindowFlags(file_menu.windowFlags()  | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    tool_menu.setAttribute(Qt::WA_TranslucentBackground);
    tool_menu.setStyleSheet("QMenu{"
                            "background-color: rgb(255, 255, 255);"
                            "border:1px solid rgb(176, 218, 255);"
                            "border-radius:8px;"
                            "}"
                            "QMenu::item{"
                            "background-color:transparent;"
                            "height:25px;"
                            "min-width:100px;"
                            "padding-left:10px;"
                            "font-size:15px;"
                            "border-radius:8px;"
                            "}"
                            "QMenu::item:selected{"
                            "background-color: rgb(176, 218, 255);"
                            "}");
    ui->toolButton->setMenu(&tool_menu);
    run_menu.addAction(ui->action_batchQuery);
    run_menu.setWindowFlags(file_menu.windowFlags()  | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    run_menu.setAttribute(Qt::WA_TranslucentBackground);
    run_menu.setStyleSheet("QMenu{"
                            "background-color: rgb(255, 255, 255);"
                            "border:1px solid rgb(176, 218, 255);"
                            "border-radius:8px;"
                            "}"
                            "QMenu::item{"
                            "background-color:transparent;"
                            "height:25px;"
                            "min-width:100px;"
                            "padding-left:10px;"
                            "font-size:15px;"
                            "border-radius:8px;"
                            "}"
                            "QMenu::item:selected{"
                            "background-color: rgb(176, 218, 255);"
                            "}");
    ui->runButton->setMenu(&run_menu);
    ui->outputWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->nodeWidget->hide();
    ui->pathWidget->hide();

    ui->graphView->test();
}

MainWindow::~MainWindow()
{
    delete ui;
}

Node *MainWindow::getSelectedNode()
{
    if(ui->objectManager->currentIndex() == 0){
        QList<QListWidgetItem *> list = ui->nodeList->selectedItems();
        if(list.empty())return nullptr;
        return dynamic_cast<Node *> (list.front());
    }
    else{
        QList<QTreeWidgetItem *> list = ui->pathList->selectedItems();
        if(list.empty())return nullptr;
        PathNode *pathnode = dynamic_cast<PathNode *> (list.front());
        if(pathnode == nullptr)return nullptr;
        return pathnode->node;
    }
}


void MainWindow::on_actionSelect_triggered()
{
    ui->graphView->setMode(GraphView::Select);
}


void MainWindow::on_actionAddPath_triggered()
{
    ui->graphView->setMode(GraphView::AddPath);
}


void MainWindow::on_selectButton_clicked()
{
    ui->actionSelect->trigger();
}


void MainWindow::on_addButton_clicked()
{
    ui->actionAddPath->trigger();
}

void MainWindow::on_nodeList_customContextMenuRequested(const QPoint &pos)
{
    if(ui->nodeList->itemAt(pos) != nullptr){
        node_menu.exec(QCursor::pos());
    }
}

void MainWindow::on_pathList_customContextMenuRequested(const QPoint &pos)
{
    PathNode *pathnode = dynamic_cast<PathNode *> (ui->pathList->itemAt(pos));
    if(pathnode != nullptr)node_menu.exec(QCursor::pos());
    else{
        Path *path = dynamic_cast<Path *> (ui->pathList->itemAt(pos));
        if(path != nullptr)path_menu.exec(QCursor::pos());
    }
}


void MainWindow::on_actionSetStartNode_triggered()
{
    ui->graphView->setMode(GraphView::Select);
    ui->graphView->setStartNode(getSelectedNode());
}


void MainWindow::on_actionSetEndNode_triggered()
{
    ui->graphView->setMode(GraphView::Select);
    ui->graphView->setEndNode(getSelectedNode());
}


void MainWindow::on_actionDeletePath_triggered()
{
    ui->graphView->setMode(GraphView::Select);
    ui->outputWidget->clear();
    QList<QTreeWidgetItem *> list = ui->pathList->selectedItems();
    if(!list.empty()){
        Path *path = dynamic_cast<Path *> (list.front());
        if(path != nullptr)ui->graphView->deletePath(path);
    }
}


void MainWindow::on_actionNewFile_triggered()
{
    ui->graphView->clear();
}


void MainWindow::on_actionOpenFile_triggered()
{
    QString file_path =  QFileDialog::getOpenFileName(this, tr("Open File"), QStandardPaths::standardLocations(QStandardPaths::DesktopLocation)[0], tr("Text files (*.txt)"));
    if(file_path != ""){
        int index = ui->objectManager->currentIndex();
        if(index == 0){
            ui->nodeList->hide();
            ui->nodeWidget->show();
        }
        else{
            ui->pathList->hide();
            ui->pathWidget->show();
        }
        ui->graphView->openFile(file_path);
        if(index == 0){
            ui->nodeWidget->hide();
            ui->nodeList->show();
        }
        else{
            ui->pathWidget->hide();
            ui->pathList->show();
        }
    }
}


void MainWindow::on_actionSaveFile_triggered()
{
    if(ui->graphView->getHave_file_path() == false){
        on_actionSaveFileAs_triggered();
        return;
    }
    ui->graphView->saveFile();
}


void MainWindow::on_actionSaveFileAs_triggered()
{
    QString file_path = QFileDialog::getSaveFileName(this, tr("Save File"), QStandardPaths::standardLocations(QStandardPaths::DesktopLocation)[0] + "/untitled.txt", tr("Text files (*.txt)"));
    ui->graphView->saveFile(file_path);
}


void MainWindow::on_action_openObjectManager_triggered()
{
    ui->leftWidget->show();
}


void MainWindow::on_action_openConsole_triggered()
{
    ui->bottomWidget->show();
}


void MainWindow::on_action_openOutput_triggered()
{
    ui->rightWidget->show();
}


void MainWindow::on_action_batchQuery_triggered()
{
    QString file_path =  QFileDialog::getOpenFileName(this, tr("Open File"), QStandardPaths::standardLocations(QStandardPaths::DesktopLocation)[0], tr("Text files (*.txt)"));
    if(file_path != ""){
        ui->graphView->queryFile(file_path);
    }
}


void MainWindow::on_closeButton_clicked()
{
    this->window()->close();
}


void MainWindow::on_maximumButton_clicked()
{
    this->window()->isMaximized() ? this->window()->showNormal() : this->window()->showMaximized();
}


void MainWindow::on_minimumButton_clicked()
{
    this->window()->showMinimized();
}


TitleWidget::TitleWidget(QWidget *parent)
    : QWidget(parent),
      window(nullptr),
      offset(0, 0)
{

}

void TitleWidget::setWindow(QWidget *newWindow)
{
    window = newWindow;
}

void TitleWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        offset = window->frameGeometry().topLeft() - event->globalPos();
    }
}

void TitleWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton){
        window->move(offset + event->globalPos());
    }
}
