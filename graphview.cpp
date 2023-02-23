#include "graphview.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>
#include <queue>
#include <QFile>
#include <QFileInfo>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QMessageBox>

#define debug1 printf("run1\n");fflush(stdout);
#define debug2 printf("run2\n");fflush(stdout);

#define MAX_SCALE 1
#define MIN_SCALE 0.02

#define NODE_RADII (is_highlight ? 2 * (3 + 3 * qLn(1 / GlobalVar::graph_view->getView_scale())) : 5)
#define NODE_WIDTH (is_highlight ? 1 * (3 + 3 * qLn(1 / GlobalVar::graph_view->getView_scale())) : 4)
#define NODE_COLOR (is_highlight ? Qt::red : Qt::black)
#define EDGE_WIDTH 5
#define HIGHLIGHT_EDGE_WIDTH 5 * (3 + 3 * qLn(1 / GlobalVar::graph_view->getView_scale()))


/*** ui item functions rewrite start ***/
NameLineEdit::NameLineEdit(QWidget *parent)
    : QLineEdit(parent),
      property_value(nullptr),
      node(nullptr),
      path(nullptr)
{

}

void NameLineEdit::clear()
{
    property_value = nullptr;
    node = nullptr;
    path = nullptr;
}

void NameLineEdit::setPropertyValue(QString *newPropertyValue)
{
    property_value = newPropertyValue;
}

void NameLineEdit::setNode(Node *node)
{
    this->node = node;
}

Node *NameLineEdit::getNode() const
{
    return node;
}

void NameLineEdit::setPath(Path *path)
{
    this->path = path;
}

void NameLineEdit::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    if(Node::nodes.contains(node)){
        node->setName(text());
    }
    if(Path::paths.contains(path)){
        path->setName(text());
    }
    QLineEdit::focusOutEvent(event);
}


PropertySpinBox::PropertySpinBox(QWidget *parent)
    : QDoubleSpinBox(parent),
      property_value(nullptr)
{

}

void PropertySpinBox::clear()
{
    property_value = nullptr;
}

void PropertySpinBox::setPropertyValue(qreal *newPropertyValue)
{
    property_value = newPropertyValue;
}

void PropertySpinBox::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    *property_value = value();
    QDoubleSpinBox::focusOutEvent(event);
}


ClickLabel::ClickLabel(QWidget *parent, const Qt::WindowFlags &f)
    : QLabel(parent, f)
{

}

void ClickLabel::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)emit click_left();
    else if(event->button() == Qt::RightButton)emit click_right();
}


OutputItem::OutputItem(int type)
    : QTreeWidgetItem(type),
      route(nullptr),
      path(nullptr),
      node(nullptr)
{

}

OutputItem::~OutputItem()
{
    if(route != nullptr){
        delete route;
    }
}


/*** ui item functions rewrite end ***/
/*** set global variables start ***/
GraphView *GlobalVar::graph_view = nullptr;
QGraphicsScene *GlobalVar::scene = nullptr;
QVector<Node *> *GlobalVar::cache_highlight_nodes = nullptr;
QVector<Edge *> *GlobalVar::cache_highlight_edges = nullptr;
QTabWidget *GlobalVar::console_tabs = nullptr;
QComboBox *GlobalVar::stategy_box = nullptr;
QStackedWidget *GlobalVar::property_stacks = nullptr;
NameLineEdit *GlobalVar::nodename_edit = nullptr;
NameLineEdit *GlobalVar::pathname_edit = nullptr;
PropertySpinBox *GlobalVar::price_box = nullptr;
PropertySpinBox *GlobalVar::time_box = nullptr;
PropertySpinBox *GlobalVar::speed_box = nullptr;
QLineEdit *GlobalVar::node_filter = nullptr;
QListWidget *GlobalVar::node_list = nullptr;
QLineEdit *GlobalVar::path_filter = nullptr;
QTreeWidget *GlobalVar::path_list = nullptr;
QTreeWidget *GlobalVar::output_list = nullptr;
/*** set global variables end ***/
/*** scene item functions rewrite start ***/

/**          Node              **/

QSet<Node *> Node::nodes = QSet<Node *>();
int Node::name_count = 0;

Node::Node(const QPointF &pos)
    : name(QString("站点 ").append(QString::number(++name_count))),
      is_highlight(true)
{
    setPos(pos);
    setZValue(2);
    setText(name);
    nodes.insert(this);
    GlobalVar::node_list->addItem(this);
    setHidden(!text().contains(GlobalVar::node_filter->text()));
    if(GlobalVar::graph_view->getEnableScene()){
        GlobalVar::scene->addItem(this);
    }
}

Node::~Node()
{
    nodes.remove(this);
    if(this->scene() == GlobalVar::scene){
        GlobalVar::scene->removeItem(this);
    }
}

void Node::resetup()
{
    foreach(Node *node, nodes) {
        delete node;
    }
    nodes.clear();
    name_count = 0;
}

QRectF Node::boundingRect() const
{
    qreal length = 2 * NODE_RADII + NODE_WIDTH;
    return QRectF(-length / 2 - 10, -length / 2 - 10, length + 10 * name.length(), length + 20);
}

QPainterPath Node::shape() const
{
    QPainterPath path;
    path.addEllipse(QPoint(0, 0), NODE_RADII + NODE_WIDTH / 2.0, NODE_RADII + NODE_WIDTH / 2.0);
    return path;
}

void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(QPen(NODE_COLOR, NODE_WIDTH));
    painter->setBrush(QBrush(Qt::white));
    painter->drawEllipse(QPoint(0, 0), NODE_RADII, NODE_RADII);
    qreal length = 2 * NODE_RADII + NODE_WIDTH;
    painter->drawText(QPoint(-length / 2 - 10, -length / 2 - 2), name);
}

qreal Node::Distance(Node *a, Node *b)
{
    if(a == nullptr || b == nullptr)return 0;
    QPointF delta = a->pos() - b->pos();
    qreal x = delta.x();
    qreal y = delta.y();
    return qSqrt(x * x + y * y);
}

void Node::showProperty()
{
//    GlobalVar::console_tabs->setCurrentIndex(1);
    GlobalVar::property_stacks->setCurrentIndex(0);
    GlobalVar::nodename_edit->setText(name);
    GlobalVar::nodename_edit->setPropertyValue(&name);
    GlobalVar::nodename_edit->setNode(this);
}

void Node::insertPath(PathNode *pathnode)
{
    paths.insert(pathnode);
}

void Node::deletePath(PathNode *pathnode)
{
    paths.erase(paths.find(pathnode));
}

int Node::getPathCount() const
{
    return paths.size();
}

QString Node::getName() const
{
    return name;
}

QString *Node::getNamePointer()
{
    return &name;
}

void Node::setName(const QString &newName)
{
    name = newName;
    setText(name);
    setHidden(!text().contains(GlobalVar::node_filter->text()));
    for(PathNode *pathnode : paths){
        pathnode->setText(0, name);
        setHidden(!pathnode->text(0).contains(GlobalVar::path_filter->text()));
    }
    update();
    GlobalVar::graph_view->showStartEndNode();
}

void Node::setIs_highlight(bool newIs_highlight)
{
    is_highlight = newIs_highlight;
    if(is_highlight == true)GlobalVar::cache_highlight_nodes->push_back(this);
    update();
}

std::multiset<PathNode *> *Node::getPaths()
{
    return &paths;
}



/**              Edge              **/

QSet<Edge *> Edge::edges = QSet<Edge *> ();
QMap<QPair<Node *,Node *>, Edge *> Edge::map = QMap<QPair<Node *,Node *>, Edge *> ();

Edge::Edge(Node *start_node, Node *end_node)
    : start_node(nullptr),
      end_node(nullptr),
      highlight_path(nullptr),
      paths()
{
    this->start_node = start_node;
    this->end_node = end_node;
    setPos(start_node->pos());
    setZValue(0);
    edges.insert(this);
    QPair<Node *,Node *> pair1(start_node, end_node);
    QPair<Node *,Node *> pair2(end_node, start_node);
    map[pair1] = map[pair2] = this;
    if(GlobalVar::graph_view->getEnableScene()){
        GlobalVar::scene->addItem(this);
    }
}

Edge::~Edge()
{
    QPair<Node *, Node *> pair1(start_node, end_node);
    QPair<Node *, Node *> pair2(end_node, start_node);
    map.erase(map.constFind(pair1));
    map.erase(map.constFind(pair2));
    edges.remove(this);
    if(this->scene() == GlobalVar::scene){
        GlobalVar::scene->removeItem(this);
    }
}

void Edge::resetup()
{
    foreach(Edge *edge, edges) {
        delete edge;
    }
    edges.clear();
    map.clear();
}

QRectF Edge::boundingRect() const
{
    return QRectF(fmin(0.0, end_node->x() - start_node->x()) - HIGHLIGHT_EDGE_WIDTH / 2.0,
                  fmin(0.0, end_node->y() - start_node->y()) - HIGHLIGHT_EDGE_WIDTH / 2.0,
                  fmax(start_node->x(), end_node->x()) - fmin(start_node->x(), end_node->x()) + HIGHLIGHT_EDGE_WIDTH,
                  fmax(start_node->y(), end_node->y()) - fmin(start_node->y(), end_node->y()) + HIGHLIGHT_EDGE_WIDTH);
}

QPointF Edge::counterWise90(const QPointF &pos, qreal length)
{
    qreal x = pos.x();
    qreal y = pos.y();
    QPointF ans(-y, x);
    ans *= length;
    ans /= qSqrt(x * x + y * y);
    return ans;
}

void Edge::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if(highlight_path == nullptr){
        int total_path = paths.size();
        int count_path = 0;
        QPointF start_pos(0, 0);
        QPointF end_pos = end_node->pos() - start_node->pos();
        for(std::multiset<Path *>::iterator it = paths.begin(); it != paths.end(); it++){
            Path *path = *it;
            painter->setPen(QPen(path->getColor(), 1.0 * EDGE_WIDTH / total_path));
            qreal length = EDGE_WIDTH / 2.0 - (count_path + 0.5) * EDGE_WIDTH / total_path;
            QPointF delta = counterWise90(end_pos, length);
            painter->drawLine(start_pos + delta, end_pos + delta);
            count_path++;
        }
    }
    else{
        painter->setPen(QPen(highlight_path->getColor(), HIGHLIGHT_EDGE_WIDTH));
        qreal x = end_node->x() - start_node->x();
        qreal y = end_node->y() - start_node->y();
        qreal length = qSqrt(x * x + y * y);
        x = (x * HIGHLIGHT_EDGE_WIDTH) / (2 * length);
        y = (y * HIGHLIGHT_EDGE_WIDTH) / (2 * length);
//        painter->drawLine(0, 0, end_node->x() - start_node->x(), end_node->y() - start_node->y());
        painter->drawLine(x, y, end_node->x() - start_node->x() - x, end_node->y() - start_node->y() - y);
    }
    //  **todo: change pos with nodes
}

Edge *Edge::getEdge(Node *start_node, Node *end_node)
{
    QPair<Node *,Node *> pair(start_node, end_node);
    if(map.count(pair))return map[pair];
    return new Edge(start_node, end_node);
}

Edge *Edge::findEdge(Node *start_node, Node *end_node)
{
    QPair<Node *,Node *> pair(start_node, end_node);
    if(map.count(pair))return map[pair];
    return nullptr;
}

void Edge::setHighlight_path(Path *newHighlight_path)
{
    highlight_path = newHighlight_path;
    if(highlight_path != nullptr){
        setZValue(1);
        GlobalVar::cache_highlight_edges->push_back(this);
    }
    else{
        setZValue(0);
    }
    update();
}

void Edge::insertPath(Path *path)
{
    paths.insert(path);
}

bool Edge::hasPath(Path *path)
{
    return paths.find(path) != paths.end();
}

void Edge::deletePath(Path *path)
{
    paths.erase(paths.find(path));
}

int Edge::getPathCount()
{
    return paths.size();
}



/**             Path               **/

PathNode::PathNode(Path *path, Node *node, Edge *edge)
    : node(node),
      edge(edge),
      path(path)
{
    node->insertPath(this);
    setText(0, node->getName());
    path->addChild(this);
    setHidden(!text(0).contains(GlobalVar::path_filter->text()));
}

QSet<Path *> Path::paths = QSet<Path *>();
int Path::name_count = 0;

Path::Path(const QColor &color)
    : name(QString("线路 ").append(QString::number(++name_count))),
      price(1),
      time(10),
      speed(1),
      color(Qt::black),
      pathnodes()
{
    this->color = color;
    setText(0, name);
    GlobalVar::path_list->addTopLevelItem(this);
    setHidden(!text(0).contains(GlobalVar::path_filter->text()));
    paths.insert(this);
}

Path::~Path()
{
    clear();
    paths.remove(this);
}

void Path::resetup()
{
    foreach(Path *path, paths) {
        delete path;
    }
    paths.clear();
    name_count = 0;
}

void Path::clear()
{
    for(PathNode *pathnode : pathnodes){
        Edge *edge = pathnode->edge;
        if(Edge::edges.contains(edge)){
            edge->deletePath(this);
            if(edge->getPathCount() == 0){
                delete edge;
            }
        }
    }
    for(PathNode *pathnode : pathnodes){
        Node *node = pathnode->node;
        if(Node::nodes.contains(node)){
            node->deletePath(pathnode);
            if(node->getPathCount() == 0){
                delete node;
            }
        }
    }
    for(PathNode *pathnode : pathnodes)delete pathnode;
}

void Path::addNode(const QPointF &pos, const QString &name)
{
    Node *node = dynamic_cast<Node *>(GlobalVar::scene->itemAt(pos, QTransform()));
    if(!pathnodes.empty() && node == pathnodes.back()->node)return;
    if(node == nullptr)node = new Node(pos);
    if(!name.isEmpty())node->setName(name);
    node->setIs_highlight(true);
    node->update();
    Edge *edge = nullptr;
    if(!pathnodes.empty()){
        edge = Edge::getEdge(pathnodes.back()->node, node);
        edge->setHighlight_path(this);
        edge->update();
        edge->insertPath(this);
    }
    pathnodes.push_back(new PathNode(this, node, edge));
}

void Path::showProperty()
{
//    GlobalVar::console_tabs->setCurrentIndex(1);
    GlobalVar::property_stacks->setCurrentIndex(1);
    GlobalVar::pathname_edit->setText(name);
    GlobalVar::pathname_edit->setPropertyValue(&name);
    GlobalVar::pathname_edit->setPath(this);
    GlobalVar::price_box->setValue(price);
    GlobalVar::price_box->setPropertyValue(&price);
    GlobalVar::time_box->setValue(time);
    GlobalVar::time_box->setPropertyValue(&time);
    GlobalVar::speed_box->setValue(speed);
    GlobalVar::speed_box->setPropertyValue(&speed);
}

void Path::setHighlight()
{
    for(PathNode *pathnode : pathnodes){
        Node *node = pathnode->node;
        if(Node::nodes.contains(node))node->setIs_highlight(true);
        Edge *edge = pathnode->edge;
        if(Edge::edges.contains(edge))edge->setHighlight_path(this);
    }
}

QString Path::getName() const
{
    return name;
}

QString *Path::getNamePointer()
{
    return &name;
}

void Path::setName(const QString &newName)
{
    name = newName;
    setText(0, name);
    setHidden(!text(0).contains(GlobalVar::path_filter->text()));
}

qreal Path::getPrice() const
{
    return price;
}

qreal *Path::getPricePointer()
{
    return &price;
}

void Path::setPrice(qreal newPrice)
{
    price = newPrice;
}

qreal Path::getTime() const
{
    return time;
}

qreal *Path::getTimePointer()
{
    return &time;
}

void Path::setTime(qreal newTime)
{
    time = newTime;
}

qreal Path::getSpeed() const
{
    return speed;
}

qreal *Path::getSpeedPointer()
{
    return &speed;
}

void Path::setSpeed(qreal newSpeed)
{
    speed = newSpeed;
}

void Path::setColor(const QColor &color)
{
    this->color = color;
}

QColor Path::getColor() const
{
    return color;
}

QVector<PathNode *> *Path::getPathnodes()
{
    return &pathnodes;
}


/*** scene item functions rewrite end ***/


/*** main view start ***/

GraphView::GraphView(QWidget *parent)
    : QGraphicsView{parent},
      scene(),
      enable_scene(true),
      mode(),
      view_scale(1.0),
      angle_delta(0.0),
      offset(0.0, 0.0),
      cache_position(),
      cache_path(nullptr),
      cache_highlight_nodes(),
      cache_highlight_edges(),
      start_node(nullptr),
      end_node(nullptr),
      have_file_path(false),
      file_path()
{
    GlobalVar::scene = &scene;
    GlobalVar::cache_highlight_nodes = &cache_highlight_nodes;
    GlobalVar::cache_highlight_edges = &cache_highlight_edges;
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    scene.setSceneRect(INT_MIN / 2, INT_MIN / 2, INT_MAX, INT_MAX);
//    scene.setItemIndexMethod(QGraphicsScene::NoIndex);
//    setRenderHint(QPainter::Antialiasing, false);
//    setDragMode(QGraphicsView::RubberBandDrag);
//    setOptimizationFlags(QGraphicsView::DontSavePainterState);
//    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
//    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    this->setScene(&scene);
    centerOn(0, 0);
}

GraphView::~GraphView()
{
    Path::resetup();
    Edge::resetup();
    Node::resetup();
}

void GraphView::clear()
{
    QProgressDialog dialog("清空进度", "取消", 0, 100, this);
    dialog.show();
    dialog.setValue(10);
    QCoreApplication::processEvents();
    Path::resetup();
    dialog.setValue(30);
    QCoreApplication::processEvents();
    Edge::resetup();
    dialog.setValue(60);
    QCoreApplication::processEvents();
    Node::resetup();
    dialog.setValue(90);
    QCoreApplication::processEvents();
    scene.clear();
    mode = Select;
    setOffset(QPointF(0, 0));
    changeScale(1 / view_scale, QPointF(0, 0));
    angle_delta = 0;
    cache_path = nullptr;
    cache_highlight_nodes.clear();
    cache_highlight_edges.clear();
    start_node = nullptr;
    end_node = nullptr;
    have_file_path = false;
    GlobalVar::console_tabs->setCurrentIndex(0);
    GlobalVar::stategy_box->setCurrentIndex(0);
    GlobalVar::property_stacks->setCurrentIndex(2);
    GlobalVar::nodename_edit->clear();
    GlobalVar::pathname_edit->clear();
    GlobalVar::price_box->clear();
    GlobalVar::time_box->clear();
    GlobalVar::speed_box->clear();
    GlobalVar::node_list->clear();
    GlobalVar::path_list->clear();
    GlobalVar::output_list->clear();
    showStartEndNode();
    setEnableScene(true);
    dialog.setValue(100);
    QCoreApplication::processEvents();
}

void GraphView::test()
{
//    for(int i=1;i<=358;i++){
//        printf("addpath: %d\n",i);
//        fflush(stdout);
//        Path *path = new Path();
//        Node *lastnode = nullptr;
//        for(int j=1;j<=20;j++){
//            printf("addnode: %d\n",(i-1)*20+j);
//            QPointF pos(rand(), rand());
//            Node *node = dynamic_cast<Node *>(GlobalVar::scene->itemAt(pos, QTransform()));
//            if(node == nullptr)node = new Node(pos);
//            Node *node = new Node(pos);
//            if(lastnode != nullptr){
//                Edge *edge = Edge::getEdge(lastnode, node);
//                edge->insertPath(path);
//            }
//            lastnode = node;
//        }
//    }
}

QPointF GraphView::posToPix(const QPointF &pos)
{
    return QPointF(pos.x() * 50000, -pos.y() * 50000);
}

QPointF GraphView::pixToPos(const QPointF &pos)
{
    return QPointF(pos.x() / 50000, -pos.y() / 50000);
}

void GraphView::openFile(const QString &file_path)
{
    clear();
    QFile file(file_path);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QMessageBox::critical(this, "错误", "文件错误。");
        return;
    }
    have_file_path = true;
    this->file_path = file_path;
    QStringList lines;
    while(!file.atEnd()){
        lines.push_back(file.readLine().trimmed());
    }
    if(lines.size() < 1000){
        setEnableScene(true);
    }
    else if(lines.size() < 100000){
        setEnableScene(false);
        QMessageBox::warning(this, "提示", "文件过大，将禁用视窗，仍可使用对象管理器或批量查询。");
    }
    else{
        setEnableScene(true);
        QMessageBox::critical(this, "错误", "文件过大。");
        return;
    }
    QMap<QPair<qreal, qreal>, Node *>map;
    QProgressDialog dialog("打开进度", "取消", 0, lines.size(), this);
    dialog.show();
    for(int i = 0, size = lines.size(); i < size; i++){
        dialog.setValue(i);
        QCoreApplication::processEvents();
        if(dialog.wasCanceled()){
            clear();
            break;
        }
        QString str = lines[i];
        QStringList list = str.split("：", Qt::SkipEmptyParts);
        if(list.size() < 2)continue;
        QString name = list[0];
        list = list[1].split("。", Qt::SkipEmptyParts);
        if(list.size() < 4)continue;

        bool flag = true;
        QStringList path_list = list[1].split("元", Qt::SkipEmptyParts);
        if(path_list.size() != 1)continue;
        qreal price = path_list[0].toDouble(&flag);
        if(!flag || price < 0 || price > 100)continue;

        flag = true;
        path_list = list[2].split("分钟", Qt::SkipEmptyParts);
        if(path_list.size() != 1)continue;
        qreal time = path_list[0].toDouble(&flag);
        if(!flag || time < 0 || time > 100)continue;

        flag = true;
        path_list = list[3].split("/分钟", Qt::SkipEmptyParts);
        qreal speed = path_list[0].toDouble(&flag);
        if(!flag || speed < 0 || speed > 100)continue;

        QVector<QPair<QString, QPointF> > pathnodes;
        list = list[0].split("；", Qt::SkipEmptyParts);
        for(QString s : list){
            QStringList node_list = s.split("(", Qt::SkipEmptyParts);
            if(node_list.size() != 2)continue;
            QString node_name = node_list[0];
            node_list = node_list[1].split(")", Qt::SkipEmptyParts);
            if(node_list.size() != 1)continue;
            node_list = node_list[0].split(",", Qt::SkipEmptyParts);
            if(node_list.size() != 2)continue;
            flag = true;
            qreal node_x = node_list[0].toDouble(&flag);
            if(!flag || node_x < INT_MIN / 2 || node_x > INT_MAX / 2)continue;
            flag = true;
            qreal node_y = node_list[1].toDouble(&flag);
            if(!flag || node_y < INT_MIN / 2 || node_y > INT_MAX / 2)continue;
            pathnodes.push_back(QPair<QString, QPointF> (node_name, posToPix(QPointF(node_x, node_y))));
        }
        if(pathnodes.empty()){
            continue;
        }
        Path *path = new Path();
        path->setName(name);
        path->setPrice(price);
        path->setTime(time);
        path->setSpeed(speed);
        Node *last_node = nullptr;
        for(QPair<QString, QPointF> p : pathnodes){
            Node *node = nullptr;
            QPair<qreal, qreal> pos(p.second.x(), p.second.y());
            if(map.contains(pos))node = map[pos];
            else map[pos] = node = new Node(p.second);
            node->setName(p.first);
            node->setIs_highlight(true);
            Edge *edge = nullptr;
            if(node == last_node)continue;
            if(last_node != nullptr){
                edge = Edge::getEdge(last_node, node);
                edge->insertPath(path);
                edge->setHighlight_path(path);
            }
            path->getPathnodes()->push_back(new PathNode(path, node, edge));
            last_node = node;
//            path->addNode(p.second, p.first);
        }
    }
//    printf("path size: %d\n",Path::paths.size());
//    printf("node size: %d\n",Node::nodes.size());
//    printf("edge size: %d\n",Edge::edges.size());
//    fflush(stdout);
    file.close();
    clearHighlight();
    setViewAll();
    viewport()->update();
}

void GraphView::saveFile(const QString &file_path)
{
    QFile file;
    if(file_path.isEmpty()){
        if(!have_file_path){
            QMessageBox::critical(this, "错误", "保存路径错误。");
            return;
        }
        file.setFileName(this->file_path);
    }
    else{
        file.setFileName(file_path);
        have_file_path = true;
        this->file_path = file_path;
    }
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox::critical(this, "错误", "文件错误。");
        return;
    }
    QProgressDialog dialog("保存进度", "取消", 0, Path::paths.size(), this);
    dialog.show();
    dialog.setValue(0);
    QCoreApplication::processEvents();
    int progress = 0;
    foreach(Path *path, Path::paths){
        dialog.setValue(++progress);
        QCoreApplication::processEvents();
        if(dialog.wasCanceled()){
            break;
        }
        QString str = path->getName();
        str.append("：");
        bool first_node = true;
        for(PathNode *pathnode : *path->getPathnodes()){
            if(first_node)first_node = false;
            else str.append("；");
            str.append(pathnode->node->getName());
            QPointF pos = pixToPos(pathnode->node->pos());
            str.append("(");
            str.append(QString::number(pos.x()));
            str.append(",");
            str.append(QString::number(pos.y()));
            str.append(")");
        }
        str.append("。");
        str.append(QString::number(path->getPrice()));
        str.append("元。");
        str.append(QString::number(path->getTime()));
        str.append("分钟。");
        str.append(QString::number(path->getSpeed()));
        str.append("/分钟。\n");
        file.write(str.toStdString().c_str());
    }
    file.close();
}

void GraphView::queryFile(const QString &file_path)
{
    QFile rfile(file_path);
    if(!rfile.open(QIODevice::ReadOnly | QIODevice::Text)){
        QMessageBox::critical(this, "错误", "读入文件错误");
        return;
    }
    QString save_path = file_path;
    QFile wfile(save_path.replace(".txt", "_routes_output.txt"));
    if(!wfile.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox::critical(this, "错误", "写入文件错误");
        return;
    }
    QStringList lines;
    while(!rfile.atEnd()){
        lines.push_back(rfile.readLine().trimmed());
    }
    QProgressDialog dialog("路径计算进度", "取消", 0, lines.size(), this);
    dialog.show();
    for(int i = 0, size = lines.size(); i < size; i++){
        dialog.setValue(i);
        QCoreApplication::processEvents();
        if(dialog.wasCanceled()){
            break;
        }
        QString str = lines[i];
        wfile.write((str + '\n').toStdString().c_str());
        QStringList list = str.split(" ", Qt::SkipEmptyParts);
        bool flag = false;
        int opt = list[0].toInt(&flag);
        if(!flag)continue;
        Node *start_node = nullptr;
        Node *end_node = nullptr;
        foreach(Node *node, Node::nodes){
            if(node->getName() == list[1]){
                start_node = node;
            }
            if(node->getName() == list[2]){
                end_node = node;
            }
        }
        GraphAlgorithm model;
        QVector<QVector<QPair<Node *, Path *> > *> ans_routes = model.solve(start_node, end_node, opt, 1);
        if(ans_routes.empty())continue;
        str = "";
//        qreal totDis = 0;
        qreal totTime = 0;
        qreal totPrice = 0;
//        int totChange = 0;
        Path *last_path = nullptr;
        Node *last_node = nullptr;
        bool first_path = true;
        bool first_node = true;
        for(QPair<Node *, Path *> p : *ans_routes[0]){
            Node *node = p.first;
            Path *path = p.second;
            if(node != nullptr && path != nullptr){
                if(path != last_path){
//                    totChange++;
                    totPrice += path->getPrice();
                    if(opt != 1){
                        totTime += path->getTime();
                    }
                    if(!first_path){
                        str += "；";
                    }
                    first_path = false;
                    first_node = true;
                    str += "换乘" + path->getName() + "：";
                }
                if(last_node != nullptr && node != last_node){
                    qreal dis = Node::Distance(last_node, node);
//                    totDis += dis;
                    totTime += dis / path->getSpeed();
                }
                if(!first_node){
                    str += "，";
                }
                first_node = false;
                str += node->getName();
            }
            last_node = node;
            last_path = path;
        }
        if(opt == 0){
            str += "。共花费" + QString::number(totPrice) + "元。\n";
        }
        else if(opt == 1){
            str += "。共花费" + QString::number(totTime) + "时间。\n";

        }
        else if(opt == 2){
            str += "。共花费" + QString::number(totTime) + "时间。\n";
        }
        wfile.write(str.toStdString().c_str());
    }
    rfile.close();
    wfile.close();
}

void GraphView::setEnableScene(bool flag)
{
    if(flag){
        enable_scene = true;
        this->setEnabled(true);
        setScene(&scene);
    }
    else{
        enable_scene = false;
        this->setEnabled(false);
        setScene(nullptr);
    }
}

bool GraphView::getEnableScene()
{
    return enable_scene;
}

void GraphView::setDefaultCursor()
{
    if(mode == Select){
        this->setCursor(Qt::ArrowCursor);
    }
    else{
        this->setCursor(Qt::CrossCursor);
    }
}

void GraphView::setMode(Mode mode){
    this->mode = mode;
    setDefaultCursor();
    cleanProperty();
    clearHighlight();
    cache_path = nullptr;
    if(mode == AddPath)GlobalVar::output_list->clear();
}

qreal GraphView::getView_scale() const
{
    return view_scale;
}

void GraphView::setOffset(const QPointF &pos)
{
    offset = pos;
    centerOn(offset);
}

void GraphView::clearHighlight()
{
    for(Node *node : cache_highlight_nodes){
        if(Node::nodes.contains(node)){
            node->setIs_highlight(false);
        }
    }
    cache_highlight_nodes.clear();
    for(Edge *edge : cache_highlight_edges){
        if(Edge::edges.contains(edge)){
            edge->setHighlight_path(nullptr);
        }
    }
    cache_highlight_edges.clear();
}

void GraphView::setHighlightNode(Node *node)
{
    clearHighlight();
    if(node != nullptr)node->setIs_highlight(true);
}

void GraphView::setHighlightPath(Path *path)
{
    clearHighlight();
    path->setHighlight();
}

void GraphView::setHighlightRoute(QVector<QPair<Node *, Path *> > *route)
{
    clearHighlight();
    Node *last_node = nullptr;
    for(QPair<Node *, Path *> p : *route){
        Node *node = p.first;
        Path *path = p.second;
        if(Node::nodes.contains(node))node->setIs_highlight(true);
        if(!Node::nodes.contains(last_node) || !Node::nodes.contains(node) || !Path::paths.contains(path)){
            last_node = node;
            continue;
        }
        Edge *edge = Edge::findEdge(last_node, node);
        if(edge == nullptr || !edge->hasPath(path)){
            last_node = node;
            continue;
        }
        edge->setHighlight_path(path);
        last_node = node;
    }
}

void GraphView::setViewNode(Node *node)
{
    if(!Node::nodes.contains(node))return;
    setOffset(node->pos());
}

void GraphView::setViewPath(Path *path)
{
    if(!Path::paths.contains(path) || path->getPathnodes()->empty())return;
    qreal minx = 1e18;
    qreal miny = 1e18;
    qreal maxx = -1e18;
    qreal maxy = -1e18;
    QVector<PathNode *> *pathnodes = path->getPathnodes();
    for(PathNode *pathnode : *pathnodes){
        Node *node = pathnode->node;
        if(Node::nodes.contains(node)){
            minx = fmin(minx, node->x());
            miny = fmin(miny, node->y());
            maxx = fmax(maxx, node->x());
            maxy = fmax(maxy, node->y());
        }
    }
    minx -= 50;
    miny -= 50;
    maxx += 50;
    maxy += 50;
    QPointF pos((minx + maxx) / 2, (miny + maxy) / 2);
    qreal scale_x = viewport()->width() / (maxx - minx);
    qreal scale_y = viewport()->height() / (maxy - miny);
    changeScale(fmin(scale_x, scale_y) / view_scale, pos);
    setOffset(pos);
}

void GraphView::setViewRoute(QVector<QPair<Node *, Path *> > *route)
{
    if(route->empty())return;
    qreal minx = 1e18;
    qreal miny = 1e18;
    qreal maxx = -1e18;
    qreal maxy = -1e18;
    for(QPair<Node *, Path *> p : *route){
        Node *node = p.first;
        if(Node::nodes.contains(node)){
            minx = fmin(minx, node->x());
            miny = fmin(miny, node->y());
            maxx = fmax(maxx, node->x());
            maxy = fmax(maxy, node->y());
        }
    }
    minx -= 50;
    miny -= 50;
    maxx += 50;
    maxy += 50;
    QPointF pos((minx + maxx) / 2, (miny + maxy) / 2);
    qreal scale_x = viewport()->width() / (maxx - minx);
    qreal scale_y = viewport()->height() / (maxy - miny);
    changeScale(fmin(scale_x, scale_y) / view_scale, pos);
    setOffset(pos);
}

void GraphView::setViewAll()
{
    if(Node::nodes.empty())return;
    qreal minx = 1e18;
    qreal miny = 1e18;
    qreal maxx = -1e18;
    qreal maxy = -1e18;
    foreach(Node *node, Node::nodes){
        minx = fmin(minx, node->x());
        miny = fmin(miny, node->y());
        maxx = fmax(maxx, node->x());
        maxy = fmax(maxy, node->y());
    }
    minx -= 50;
    miny -= 50;
    maxx += 50;
    maxy += 50;
    QPointF pos((minx + maxx) / 2, (miny + maxy) / 2);
    qreal scale_x = viewport()->width() / (maxx - minx);
    qreal scale_y = viewport()->height() / (maxy - miny);
    changeScale(fmin(scale_x, scale_y) / view_scale, pos);
    setOffset(pos);
}

void GraphView::deletePath(Path *path)
{
    if(Path::paths.contains(path)){
        clearHighlight();
        delete path;
        viewport()->update();
        showStartEndNode();
    }
}

void GraphView::showStartEndNode()
{
    emit startNodeChanged(Node::nodes.contains(start_node) ? start_node->getName() : "未选择");
    emit endNodeChanged(Node::nodes.contains(end_node) ? end_node->getName() : "未选择");
//    GlobalVar::console_tabs->setCurrentIndex(0);
}

void GraphView::setStartNode()
{
    start_node = GlobalVar::nodename_edit->getNode();
    showStartEndNode();
}

void GraphView::setStartNode(Node *node)
{
    start_node = node;
    showStartEndNode();
}

void GraphView::setEndNode()
{
    end_node = GlobalVar::nodename_edit->getNode();
    showStartEndNode();
}

void GraphView::queryRoute()
{
    setMode(Select);
    if(!Node::nodes.contains(start_node) || !Node::nodes.contains(end_node) || start_node == end_node)return;
    GraphAlgorithm model;
    int strategy_id = GlobalVar::stategy_box->currentIndex();
    QVector<QVector<QPair<Node *, Path *> > *> ans_routes
            = model.solve(start_node, end_node, GlobalVar::stategy_box->currentIndex(), 5);
    int route_count = 0;
    GlobalVar::output_list->clear();
    for(QVector<QPair<Node *, Path *> > *route : ans_routes){
        qreal totDis = 0;
        qreal totTime = 0;
        qreal totPrice = 0;
        int totChange = 0;
        Path *last_path = nullptr;
        Node *last_node = nullptr;
        for(QPair<Node *, Path *> p : *route){
            Node *node = p.first;
            Path *path = p.second;
            if(node != nullptr && path != nullptr){
                if(path != last_path){
                    totChange++;
                    totPrice += path->getPrice();
                    if(strategy_id != 1){
                        totTime += path->getTime();
                    }
                }
                if(last_node != nullptr && node != last_node){
                    qreal dis = Node::Distance(last_node, node);
                    totDis += dis;
                    totTime += dis / path->getSpeed();
                }
            }
            last_node = node;
            last_path = path;
        }
        OutputItem *route_item = new OutputItem();
        route_item->route = route;
        route_item->setText(0, QString("方案 ").append(QString::number(++route_count))
                            .append("(总距离:").append(QString::number(totDis))
                            .append("单位,总时间:").append(QString::number(totTime))
                            .append("分钟,总价:").append(QString::number(totPrice))
                            .append("元,换乘次数:").append(QString::number(totChange))
                            .append("次)"));
        GlobalVar::output_list->addTopLevelItem(route_item);
        last_path = nullptr;
        OutputItem *last_path_item = nullptr;
        for(QPair<Node *, Path *> p : *route){
            Node *node = p.first;
            Path *path = p.second;
            if(node == nullptr || path == nullptr)continue;
            if(path != last_path){
                last_path = path;
                last_path_item = new OutputItem();
                last_path_item->path = path;
                last_path_item->setText(0, path->getName());
                route_item->addChild(last_path_item);
            }
            OutputItem *node_item = new OutputItem();
            node_item->node = node;
            node_item->setText(0, node->getName());
            last_path_item->addChild(node_item);
        }
    }
}

void GraphView::setEndNode(Node *node)
{
    end_node = node;
    showStartEndNode();
}

void GraphView::swapStartEndNode()
{
    qSwap(start_node, end_node);
    showStartEndNode();
}

void GraphView::clearStartNode()
{
    start_node = nullptr;
    emit startNodeChanged("未选择");
}

void GraphView::clearEndNode()
{
    end_node = nullptr;
    emit endNodeChanged("未选择");
}

void GraphView::setFile_path(const QString &newFile_path)
{
    have_file_path = true;
    file_path = newFile_path;
}

void GraphView::clearFile_path()
{
    have_file_path = false;
}

bool GraphView::getHave_file_path() const
{
    return have_file_path;
}


void GraphView::showListItem(QListWidgetItem *item)
{
    Node *node = dynamic_cast<Node *>(item);
    if(node != nullptr){
        setHighlightNode(node);
        setViewNode(node);
        node->showProperty();
    }
}

void GraphView::nodeFilter(const QString &str)
{
    foreach(Node *node, Node::nodes) {
        node->setHidden(!node->text().contains(str));
    }
}

void GraphView::showTreeItem(QTreeWidgetItem *item)
{
    Path *path = dynamic_cast<Path *>(item);
    if(path != nullptr){
        setHighlightPath(path);
        setViewPath(path);
        path->showProperty();
    }
    else{
        PathNode *pathnode = dynamic_cast<PathNode *>(item);
        if(pathnode != nullptr){
            Node *node = pathnode->node;
            setViewNode(node);
            setHighlightPath(pathnode->path);
            node->showProperty();
        }
    }
}

void GraphView::pathFilter(const QString &str)
{
    foreach(Path *path, Path::paths){
        path->setHidden(true);
    }
    for(QTreeWidgetItemIterator it(GlobalVar::path_list); *it; it++){
        if((*it)->text(0).contains(str)){
            (*it)->setHidden(false);
            if((*it)->parent() != nullptr){
                (*it)->parent()->setHidden(false);
            }
        }
        else{
            (*it)->setHidden(true);
        }
    }
}

void GraphView::showOutputItem(QTreeWidgetItem *item)
{
    QTreeWidgetItem *parent = item;
    while(parent->parent() != nullptr)parent = parent->parent();
    OutputItem *x = dynamic_cast<OutputItem *> (item);
    OutputItem *y = dynamic_cast<OutputItem *> (parent);
    if(y->route != nullptr){
        setHighlightRoute(y->route);
    }
    if(x->route != nullptr){
        setViewRoute(x->route);
    }
    else if(Path::paths.contains(x->path)){
        setViewPath(x->path);
    }
    else if(Node::nodes.contains(x->node)){
        setViewNode(x->node);
    }
}

void GraphView::prt(const QPointF &pos)
{
    printf("%lf %lf\n",pos.x(),pos.y());
    fflush(stdout);
}

void GraphView::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        QPoint pos = event->pos();
        QPointF scene_pos = this->mapToScene(pos);
        if(mode == Select){
            Node *node = dynamic_cast<Node *>(scene.itemAt(scene_pos, QTransform()));
            if(node != nullptr){
                setHighlightNode(node);
                node->showProperty();
            }
            else{
                clearHighlight();
                cleanProperty();
            }
        }
        else if(mode == AddPath){
            if(cache_path == nullptr){
                cache_path = new Path();
            }
            cache_path->addNode(scene_pos);
            cache_path->showProperty();
//            GlobalVar::console_tabs->setCurrentIndex(1);
        }
    }
    else if(event->button() == Qt::RightButton){
        this->setCursor(Qt::ClosedHandCursor);
        cache_position = offset + event->position() / view_scale;
    }
}

void GraphView::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton){
        setDefaultCursor();
    }
}

void GraphView::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::RightButton){
        setOffset(cache_position - event->position() / view_scale);
    }
}

void GraphView::changeScale(qreal new_scale, const QPointF &pos)
{
    QPointF scene_position = this->mapToScene(pos.x(), pos.y());
    QPointF center_position = QPointF(this->viewport()->width() / 2, this->viewport()->height() / 2);
    new_scale = fmin(new_scale, MAX_SCALE / view_scale);
    new_scale = fmax(new_scale, MIN_SCALE / view_scale);
    view_scale *= new_scale;
//    qDebug()<<"scale="<<view_scale;
    scale(new_scale, new_scale);
    setOffset(scene_position + (center_position - pos) / view_scale);
}

void GraphView::cleanProperty()
{
    GlobalVar::property_stacks->setCurrentIndex(2);
}

void GraphView::wheelEvent(QWheelEvent *event)
{
    angle_delta += event->angleDelta().y();
    for(;angle_delta >= 120; angle_delta -= 120){
        changeScale(1.1, event->position());
    }
    for(;angle_delta <= -120; angle_delta += 120){
        changeScale(1 / 1.1, event->position());
    }
}

/*** main view end ***/

/*** algorithm start ***/
GraphAlgorithm::GraphAlgorithm()
    : tot_node(0),
      map(),
      id_node(),
      id_path(),
      g(),
      g_r(),
      dis(),
      tr(),
      g2(),
      vis()
{

}

QVector<QVector<QPair<Node *, Path *> > *> GraphAlgorithm::solve(Node *start_node, Node *end_node, int opt, int size)
{
    QVector<QVector<QPair<Node *, Path *> > *> ans_routes;
    int cnt = Node::nodes.size();
    foreach(Path *path, Path::paths){
        cnt += 4 * path->getPathnodes()->size();
    }
    setup(cnt);
    cnt = 0;
    foreach(Node *node, Node::nodes){
        map[node] = cnt;
        cnt++;
    }
    if(!map.count(start_node) || !map.count(end_node))return ans_routes;
    foreach(Path *path, Path::paths){
        QVector<PathNode *> *pathnodes = path->getPathnodes();
        Node *last_node = nullptr;
        for(QVector<PathNode *>::iterator it = pathnodes->begin(); it != pathnodes->end(); it++){
            Node *node = (*it)->node;
            if(it != pathnodes->begin()){
                if(opt == 0 || opt == 2){
                    g[cnt + 2].push_back(std::pair<int, double>(cnt, 0));
                    g[cnt + 1].push_back(std::pair<int, double>(cnt + 3, 0));
                }
                double w = 0;
                if(opt == 1 || opt == 2)w = Node::Distance(last_node, node) / path->getSpeed();
                g[cnt].push_back(std::pair<int, double>(cnt - 2, w));
                g[cnt - 1].push_back(std::pair<int, double>(cnt + 1, w));
            }
            id_node[cnt] = id_node[cnt + 1] = id_node[cnt + 2] = id_node[cnt + 3] = node;
            id_path[cnt] = id_path[cnt + 1] = id_path[cnt + 2] = id_path[cnt + 3] = path;
            if(map.count(node)){
                int v = map[node];
                g[cnt + 1].push_back(std::pair<int, double>(v, 0));
                g[cnt + 2].push_back(std::pair<int, double>(v, 0));
                double w = 0;
                if(opt == 0)w = path->getPrice();
                else if(opt == 1)w = 0;
                else if(opt == 2)w = path->getTime();
                g[v].push_back(std::pair<int, double>(cnt, w));
                g[v].push_back(std::pair<int, double>(cnt + 3, w));
            }
            last_node = node;
            cnt += 4;
        }
    }
    dijkstra(map[start_node]);
//    printf("disT = %lf\n",dis[map[end_node]]);
//    fflush(stdout);
    findPaths(map[start_node], map[end_node], size, &ans_routes);
    return ans_routes;
}

void GraphAlgorithm::setup(int tot_node)
{
    this->tot_node = tot_node;
    map.clear();
    id_node = std::vector<Node *> (tot_node, nullptr);
    id_path = std::vector<Path *> (tot_node, nullptr);
    g = std::vector<std::vector<std::pair<int, double> > > (tot_node);
    g_r = std::vector<std::vector<int> > (tot_node);
    dis = std::vector<double> (tot_node, 1e18);
    tr.clear();
}

void GraphAlgorithm::dijkstra(int S)
{
    std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int> >, std::greater<std::pair<double, int> > > q;
    dis[S] = 0;
    q.push(std::make_pair(0, S));
    while(!q.empty()){
        std::pair<double, int> p = q.top();
        q.pop();
        double d = p.first;
        int u=p.second;
        if(std::fabs(d - dis[u]) > 1e-6){
            continue;
        }
        for(std::pair<int, double> e : g[u]){
            int v = e.first;
            double w = e.second;
            if(dis[v] > dis[u] + w){
                dis[v] = dis[u] + w;
                q.push(std::make_pair(dis[v], v));
            }
        }
    }
}

void GraphAlgorithm::dijkstra_base(int S)
{
    dis[S] = 0;
    for(int i = 0; i < tot_node; i++){
        int t = -1;
        for(int j = 0; j < tot_node; j++){
            if(!vis[j] && (t==-1 || dis[t] > dis[j])){
                t = j;
            }
        }
        vis[t] = true;
        for(int j = 0; j < tot_node; j++){
            dis[j] = fmin(dis[j], dis[t] + g2[t][j]);
        }
    }
}

void GraphAlgorithm::findPaths(int S, int T, int size, QVector<QVector<QPair<Node *, Path *> > *> *ans)
{
    for(int u = 0; u < tot_node; u++){
        for(std::pair<int, double> p : g[u]){
            int v = p.first;
            double w = p.second;
            if(fabs(dis[v] - dis[u] - w) < 1e-6){
                g_r[v].push_back(u);
            }
        }
    }
    std::queue<int> q;
    q.push(T);
    tr.push_back(std::pair<int, int>(T, -1));
    int now = 0;
    while(!q.empty()){
        int u = q.front();
        q.pop();
        if(u == S){
            QVector<QPair<Node*, Path *> > *res = new QVector<QPair<Node*, Path *> >();
            for(int tmp = now; tmp >= 0; tmp = tr[tmp].second){
                int id = tr[tmp].first;
//                printf("id = %d\n",id);
//                fflush(stdout);
                if(id_node[id] != nullptr && id_path[id] != nullptr){
                    QPair<Node *, Path *> p(id_node[id], id_path[id]);
                    if(res->empty() || p != res->back()){
                        res->push_back(p);
                    }
                }
            }
            ans->push_back(res);
        }
        else{
            for(int v : g_r[u]){
                if(int(q.size()) < size){
                    q.push(v);
                    tr.push_back(std::pair<int, int>(v, now));
                }
            }
        }
        now++;
    }
}
/*** algorithm end ***/
