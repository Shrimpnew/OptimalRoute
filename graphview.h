#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QTabWidget>
#include <QStackedWidget>
#include <QListWidget>
#include <QTreeWidget>
#include <QLabel>
#include <set>
#include <QComboBox>
#include <QStatusBar>
#include <QProgressBar>


/*** ui item functions rewrite start ***/
class Node;
class Edge;
class Path;

class NameLineEdit : public QLineEdit{
    Q_OBJECT
public:
    NameLineEdit(QWidget *parent = nullptr);
    void clear();
    void setPropertyValue(QString *newPropertyValue);
    void setNode(Node *node);
    Node *getNode() const;
    void setPath(Path *path);

protected:
    void focusOutEvent(QFocusEvent *event);

private:
    QString *property_value;
    Node *node;
    Path *path;
};



class PropertySpinBox : public QDoubleSpinBox{
    Q_OBJECT
public:
    PropertySpinBox(QWidget *parent = nullptr);
    void clear();
    void setPropertyValue(qreal *newPropertyValue);

protected:
    void focusOutEvent(QFocusEvent *event);

private:
    qreal *property_value;
};


class ClickLabel : public QLabel{
    Q_OBJECT
public:
    ClickLabel(QWidget *parent = nullptr, const Qt::WindowFlags &f = Qt::WindowFlags());
signals:
    void click_left();
    void click_right();

protected:
    void mousePressEvent(QMouseEvent *event);
};

class OutputItem : public QTreeWidgetItem{
public:
    QVector<QPair<Node *, Path*> > *route;
    Path *path;
    Node *node;
    OutputItem(int type = Type);
    ~OutputItem();
};

/*** ui item functions rewrite end ***/
/*** set global variables start ***/
class GraphView;
class GlobalVar{
public:
    static GraphView *graph_view;
    static QGraphicsScene *scene;
    static QVector<Node *> *cache_highlight_nodes;
    static QVector<Edge *> *cache_highlight_edges;
    static QTabWidget *console_tabs;
    static QComboBox *stategy_box;
    static QStackedWidget *property_stacks;
    static NameLineEdit *nodename_edit;
    static NameLineEdit *pathname_edit;
    static PropertySpinBox *price_box;
    static PropertySpinBox *time_box;
    static PropertySpinBox *speed_box;
    static QLineEdit *node_filter;
    static QListWidget *node_list;
    static QLineEdit *path_filter;
    static QTreeWidget *path_list;
    static QTreeWidget *output_list;
};
/*** set global variables end ***/
/*** scene item functions rewrite start ***/
class PathNode;

class Node : public QGraphicsItem, public QListWidgetItem{
public:
    static QSet<Node *> nodes;
    Node(const QPointF &pos = QPointF(0.0, 0.0));
    ~Node();
    static void resetup();
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
    static qreal Distance(Node *a, Node *b);
    void showProperty();
    void insertPath(PathNode *pathnode);
    void deletePath(PathNode *pathnode);
    int getPathCount() const;
    QString getName() const;
    QString *getNamePointer();
    void setName(const QString &newName);
    void setIs_highlight(bool newIs_highlight);
    std::multiset<PathNode *> *getPaths();

private:
    static int name_count;
    QString name;
    bool is_highlight;
    std::multiset<PathNode *>paths;
};


class Edge : public QGraphicsItem{
public:
    static QSet<Edge *> edges;
    Edge(Node *start_node, Node *end_node);
    ~Edge();
    static void resetup();
    QRectF boundingRect() const;
    QPointF counterWise90(const QPointF &pos, qreal length);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
    static Edge *getEdge(Node *start_node, Node *end_node);
    static Edge *findEdge(Node *start_node, Node *end_node);
    void setHighlight_path(Path *newHighlight_path);
    void insertPath(Path *path);
    bool hasPath(Path *path);
    void deletePath(Path *path);
    int getPathCount();

private:
    static QMap<QPair<Node *,Node *>, Edge*> map;
    Node *start_node;
    Node *end_node;
    Path *highlight_path;
    std::multiset<Path *>paths;
};

class Path;

class PathNode : public QTreeWidgetItem{
public:
    PathNode(Path *path, Node *node = nullptr, Edge *edge = nullptr);
    Node *node;
    Edge *edge;
    Path *path;
};

class Path : public QTreeWidgetItem{
public:
    static QSet<Path *> paths;
    Path(const QColor &color = QColor(rand() % 256, rand() % 256, rand() % 256));
    ~Path();
    static void resetup();
    void clear();
    void addNode(const QPointF &pos, const QString &name = QString());
    void showProperty();
    void setHighlight();
    QString getName() const;
    QString *getNamePointer();
    void setName(const QString &newName);
    qreal getPrice() const;
    qreal *getPricePointer();
    void setPrice(qreal newPrice);
    qreal getTime() const;
    qreal *getTimePointer();
    void setTime(qreal newTime);
    qreal getSpeed() const;
    qreal *getSpeedPointer();
    void setSpeed(qreal newSpeed);
    void setColor(const QColor &color);
    QColor getColor() const;
    QVector<PathNode *> *getPathnodes();

private:
    static int name_count;
    QString name;
    qreal price;
    qreal time;
    qreal speed;
    QColor color;
    QVector<PathNode *> pathnodes;
};
/*** scene item functions rewrite end ***/


/*** main view start ***/
class GraphView : public QGraphicsView
{
    Q_OBJECT
public:
    enum Mode{ Select, AddPath };

    explicit GraphView(QWidget *parent = nullptr);
    ~GraphView();
    void clear();
    void test();
    QPointF posToPix(const QPointF &pos);
    QPointF pixToPos(const QPointF &pos);
    void openFile(const QString &file_path);
    void saveFile(const QString &file_path = QString());
    void queryFile(const QString &file_path);
    void setEnableScene(bool flag);
    bool getEnableScene();
    void setMode(Mode mode);
    qreal getView_scale() const;
    void setOffset(const QPointF &pos);
    void clearHighlight();
    void setHighlightNode(Node *node);
    void setHighlightPath(Path *path);
    void setHighlightRoute(QVector<QPair<Node *, Path *> > *route);
    void setViewNode(Node *node);
    void setViewPath(Path *path);
    void setViewRoute(QVector<QPair<Node *, Path *> > *route);
    void setViewAll();
    void deletePath(Path *path);
    void showStartEndNode();
    void setStartNode(Node *node);
    void setEndNode(Node *node);
    void swapStartEndNode();
    void clearStartNode();
    void clearEndNode();
    void setFile_path(const QString &newFile_path);
    void clearFile_path();
    bool getHave_file_path() const;

public slots:
    void showListItem(QListWidgetItem *item);
    void nodeFilter(const QString &str);
    void showTreeItem(QTreeWidgetItem *item);
    void pathFilter(const QString &str);
    void showOutputItem(QTreeWidgetItem *item);
    void setStartNode();
    void setEndNode();
    void queryRoute();

signals:
    void startNodeChanged(const QString &string);
    void endNodeChanged(const QString &string);

protected:
    void prt(const QPointF &pos);
    void setDefaultCursor();
    void changeScale(qreal new_scale, const QPointF &pos);
    void cleanProperty();
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    QGraphicsScene scene;
    bool enable_scene;
    Mode mode;
    qreal view_scale;
    int angle_delta;
    QPointF offset;
    QPointF cache_position;
    Path *cache_path;
    QVector<Node *> cache_highlight_nodes;
    QVector<Edge *> cache_highlight_edges;
    Node *start_node;
    Node *end_node;
    bool have_file_path;
    QString file_path;
};
/*** main view end ***/

/*** algorithm start ***/
class GraphAlgorithm{
public:
    GraphAlgorithm();
    QVector<QVector<QPair<Node *, Path *> > *> solve(Node *start_node, Node *end_node, int opt, int size);

protected:
    void setup(int tot_node);
    void dijkstra(int S);
    void dijkstra_base(int S);
    void findPaths(int S, int T, int size, QVector<QVector<QPair<Node *, Path *> > *> *ans);

private:
    int tot_node;
    std::map<Node *, int> map;
    std::vector<Node *> id_node;
    std::vector<Path *> id_path;
    std::vector<std::vector<std::pair<int, double> > > g;
    std::vector<std::vector<int> > g_r;
    std::vector<double> dis;
    std::vector<std::pair<int, int> > tr;
    std::vector<std::vector<double> > g2;
    std::vector<bool> vis;
};
/*** algorithm end ***/

#endif // GRAPHVIEW_H
