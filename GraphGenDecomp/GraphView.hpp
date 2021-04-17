#include "../GraphGen/GraphGen.h"
#include "../GraphDecomp/GraphDecomp.h"
//#include "../GUI_facilities.h"

/*开发记录：
	2020 / 12 / 24 ~ 2020 / 12 / 25             小型结构
    2020 / 12 / 25 ~ 2020 / 12 / 26             展示算法
*/

// Snake Distribution

#ifndef pq
#define pq

// 0 号元素不存，最小化堆
template<class Type>
class priorityQueue_m : public queue<Type> {
private:
    int currentSize;                            // 队列长度
    Type* array;
    int maxSize;                                // 容量

    void doubleSpace() {
        Type* tmp = array;
        maxSize *= 2;
        array = new Type[maxSize];
        for (int i = 0; i <= currentSize; ++i)
            array[i] = tmp[i];
        delete[] tmp;
    }

    void buildHeap() {
        for (int i = currentSize / 2; i > 0; --i)
            percolateDown(i);
    }

    void percolateDown(int hole) {
        int child;
        Type tmp = array[hole];

        for (; hole * 2 <= currentSize; hole = child) {
            child = hole * 2;
            if (child != currentSize && array[child + 1] < array[child])    // 较小的一个
                ++child;
            if (array[child] < tmp)
                array[hole] = array[child];                             // 换掉
            else break;
        }
        array[hole] = tmp;
    }

public:

    priorityQueue_m(int capacity = 100) {
        array = new Type[capacity];
        maxSize = capacity;
        currentSize = 0;
    }

    priorityQueue_m(const Type item[], int size) :
        maxSize(size + 10), currentSize(size) {
        array = new Type[maxSize];
        for (int i = 0; i < size; ++i)
            array[i + 1] = item[i];
        buildHeap();
    }

    ~priorityQueue_m() { delete[] array; }

    bool isEmpty() const { return currentSize == 0; }

    void enQueue(const Type& x) {
        if (currentSize == maxSize - 1) doubleSpace();

        //向上过滤
        int hole = ++currentSize;
        for (; hole > 1 && x < array[hole / 2]; hole /= 2)
            array[hole] = array[hole / 2];
        array[hole] = x;
    }

    Type deQueue() {
        Type minItem;
        minItem = array[1];
        array[1] = array[currentSize--];
        percolateDown(1);
        return minItem;
    }

    Type getHead() const { return array[1]; }
};


#endif // !pq

struct nodeConn {
    int data;
    int connSize;
    double connWeight;

    nodeConn(int d = RESNODE, int c = -1, double w = 0) : data(d), connSize(c), connWeight(w) {}
    ~nodeConn() = default;

    bool operator<(nodeConn& r) const {
        // 比较以连接量优先，方便展示
        if (connSize < r.connSize) return true;
        return false;
    }
};

// 单图读取器
class SinglePrevReader : public Processor {
public:
    SinglePrevReader(string _mainDir = "\0", bool raw = true) : mainDir(_mainDir) {
		fstream mfs(mainDir, fstream::in);
        if (raw)
            readRawFile(mfs);
        else readFile(mfs, false);

        int nsize = adjListGraph.size() + isoNodes.size();
        nodeConn* nodeConnArr = new nodeConn[nsize];

        int i = 0;
        for (auto n : adjListGraph) {
            double nWeight = 0;
            for (auto e : n.second) {
                nWeight += e.weight;
                if (e.weight > maxEdgeWeight)
                    maxEdgeWeight = e.weight;
            }
            if (nWeight > maxNodeWeight)
                maxNodeWeight = nWeight;
            nodeConnArr[i++] = nodeConn(n.first, (int)n.second.size(), nWeight);
        }
        for (auto in : isoNodes)
            nodeConnArr[i++] = nodeConn(in, 0, 0.0);
        priorityQueue_m<nodeConn> pnq(nodeConnArr, nsize);
        while (!pnq.isEmpty())
            displayq.push(pnq.deQueue());

        delete[] nodeConnArr;
	}
	~SinglePrevReader() = default;

	queue<nodeConn> displayq;
    double maxNodeWeight = 0;
    double maxEdgeWeight = 0;
private:
    string mainDir;
};

// 多图读取器
class MultiPrevReader : public Processor {
public:
	MultiPrevReader(string _subDir, string _fil) {
		subDir = _subDir;
		SUFFIX = _fil;

		vector<string> opt_files;
		getFiles(subDir, ".txt", opt_files, SUFFIX);
		for (auto f : opt_files) {
            fileNo curFile = parseFileInt(f);
            SinglePrevReader spr(f, false);
            storedNodeq[curFile] = spr.displayq;
            if (spr.maxNodeWeight > maxNodeWeight)
                maxNodeWeight = spr.maxNodeWeight;
            if (spr.maxEdgeWeight > maxEdgeWeight)
                maxEdgeWeight = spr.maxEdgeWeight;

			fstream subfs(f, fstream::in);
			FileUnit subfu;
			subfu.readFile(subfs, false);
			storedFile[curFile] = subfu;
		}
	}
	~MultiPrevReader() = default;

    double maxNodeWeight = 0;
    double maxEdgeWeight = 0;
    map<fileNo, queue<nodeConn>> storedNodeq;
	map<fileNo, FileUnit> storedFile;
};

class GraphView : public Fl_Widget {
public:
	GraphView(int x, int y, int w, int h, const char* l = NULL) :Fl_Widget(x, y, w, h, l) {

	}

    // 使用新数据刷新视图
	void RefreshView(string _dir, bool dark_mode, string _fil = "\0") {
        initPara(dark_mode);

		if (_fil == "\0") {
            SinglePrevReader mpr(_dir, true);
            updateCoord(mpr.displayq);
            edgeSave[0] = mpr.adjListGraph;
            maxNodeWeight = mpr.maxNodeWeight;
            maxEdgeWeight = mpr.maxEdgeWeight;
		}
		else {
			MultiPrevReader opr(_dir, _fil);
            for (auto f : opr.storedNodeq)
                updateCoord(f.second, f.first);
            for (auto f : opr.storedFile)
                edgeSave[f.first] = f.second.adjListGraph;
            maxNodeWeight = opr.maxNodeWeight;
            maxEdgeWeight = opr.maxEdgeWeight;
		}
        redraw();
	}

    void Dark_Mode(bool dark_mode) {
        if (dark_mode) {
            bg = Color::black;
            fg = Color::white;
            hg = Color::yellow;
        }
        else {
            bg = Color::white;
            fg = Color::black;
            hg = Color::red;
        }
    }
    
    int label = 2;                                                  // 节点标注状态
    set<GraphCommon::node> nodeR;       // 可达点标记 
    queue<GraphCommon::node> pathR;     // 最短路径
private:
    map<fileNo, map<int, pair<float, float>>> nodeCoord;	        // 存储节点坐标
    map < fileNo, map<int, vector<GraphCommon::edge>>> edgeSave;    // 存储文件边集
    map<fileNo, map<int, double>> nodeWeight;                       // 存储节点权重
    double maxNodeWeight = 0;           // 最大节点权重
    double maxEdgeWeight = 0;           // 最大边权重
    map<fileNo, int[4]> fborder;        // 画布边界   
    bool dim = false;                   // 暗下来
    Fl_Color bg = NULL;
    Fl_Color fg = NULL;
    Fl_Color hg = NULL;

    // 刷新初始参数
    void initPara(bool dark_mode) {
        nodeCoord.clear();
        edgeSave.clear();
        nodeWeight.clear();
        maxNodeWeight = 0;
        maxEdgeWeight = 0;
        fborder.clear();
        Dark_Mode(dark_mode);
    }

    // 蛇形矩阵 OJ-1021
    struct SnakeArray {
        int size;                       // 一维大小
        double step;                    // 每一步的步长
        int bound[4];	                // 记录四边界
        int pos[2];                     // 当前位置
        int dir[2];                     // 上一次的移动方向向量
        bool first = true;              // 第一个

        SnakeArray(int n) :size(n), step(1.0 / (n + 1)) {
            bound[0] = n;               // 右边界
            bound[1] = n;               // 下边界
            bound[2] = -1;              // 左边界
            bound[3] = 0;               // 上边界
            pos[0] = 0;                 // 开始的x坐标
            pos[1] = 0;                 // 开始的y坐标
            dir[0] = 1;                 // 移动方向向量x坐标
            dir[1] = 0;                 // 移动方向向量y坐标
        }

        void dirNext() {
            if (dir[0] == 1) {
                if (pos[0] + dir[0] >= bound[0]) {
                    bound[0]--;
                    dir[0] = 0; dir[1] = 1;
                }
            }
            else if (dir[1] == 1) {
                if (pos[1] + dir[1] >= bound[1]) {
                    bound[1]--;
                    dir[1] = 0; dir[0] = -1;
                }
            }
            else if (dir[0] == -1) {
                if (pos[0] + dir[0] <= bound[2]) {
                    bound[2]++;
                    dir[0] = 0; dir[1] = -1;
                }
            }
            else if (dir[1] == -1) {
                if (pos[1] + dir[1] <= bound[3]) {
                    bound[3]++;
                    dir[1] = 0; dir[0] = 1;
                }
            }
        }

        inline pair<float, float> convertPos() {
            return make_pair((pos[0] + 1) * step, (pos[1] + 1) * step);
        }

        pair<float, float> getNextPos() {
            if (first) {
                first = false;
                return convertPos();
            }
            dirNext();
            pos[0] += dir[0];
            pos[1] += dir[1];
            return convertPos();
        }
    };

    void updateCoord(queue<nodeConn>& pnq, fileNo fn = 0) {
        SnakeArray sa(ceil(sqrt(pnq.size())));
        while (!pnq.empty()) {
            nodeConn cur = pnq.front();
            pnq.pop();
            nodeCoord[fn][cur.data] = sa.getNextPos();
            nodeWeight[fn][cur.data] = cur.connWeight;
        }
    }

    // 获取节点真正坐标
    pair<int, int> getNodeCanvasCoord(pair<float, float>& ratioCoord, int* fborder) {
        return make_pair(x() + ratioCoord.first * (fborder[0] - fborder[2]) + fborder[2],
            y() + ratioCoord.second * (fborder[1] - fborder[3]) + fborder[3]);
    }

    // 更新画布边界
    void refreshFborder() {
        double fsize = ceil(sqrt(nodeCoord.size()));
        int fpos[2] = { 1,1 };          // 以右下为基准点

        for (auto f : nodeCoord) {
            int fbordertmp[4] = {
                w() * fpos[0] / fsize,           // 右边界
                h() * fpos[1] / fsize,           // 下边界
                w() * (fpos[0] - 1) / fsize,     // 左边界
                h() * (fpos[1] - 1) / fsize      // 上边界
            };

            for (int i = 0; i < 4; ++i)
                fborder[f.first][i] = fbordertmp[i];

            // 更新下一个画布位置
            if (fpos[0] + 1 > (int)fsize) {
                fpos[1]++;
                fpos[0] = 1;
            }
            else fpos[0]++;
        }
    }

    void drawNode(fileNo file, int node) {
        
            // 按照连接权重涂色
            float nWeight = (maxNodeWeight == 0 ? 1.0 : nodeWeight[file][node] / maxNodeWeight);
            if (dim) nWeight *= 0.5;
            fl_color(fl_color_average(fg, bg, nWeight));

            auto ncoord = getNodeCanvasCoord(nodeCoord[file][node], fborder[file]);
            fl_pie(ncoord.first - 3, ncoord.second - 3, 6, 6, 0, 360);
            //标号
            if (label == 2) {
                if (dim) fl_color(fl_color_average(fg, bg, 0.5));
                else fl_color(fg);
                fl_draw(to_string(node).c_str(), ncoord.first, ncoord.second - 5);
            }
        
    }

    void drawNodes() {
        for (auto fg : nodeCoord)
            for (auto n = fg.second.begin(); n != fg.second.end(); ++n)
                drawNode(fg.first, n->first);
    }
    
    void drawEdge(GraphCommon::edge e, fileNo startF, bool stress = false) {
        // 按照权重涂色
        float eWeight = (maxEdgeWeight == 0 ? 1.0 : e.weight / maxEdgeWeight);
        if (dim) eWeight *= 0.5;
        if (stress) eWeight = 1.0;

        pair<int, int> beg = getNodeCanvasCoord(nodeCoord[startF][e.start], fborder[startF]);

        // end 可能为虚节点
        pair<int, int> fin;
        if (e.end == -1) {
            fileNo tarFile = parseInt(e.targetFile);
            fl_color(fl_color_average(hg, bg, eWeight));
            fin = getNodeCanvasCoord(nodeCoord[tarFile][e.targetNode], fborder[tarFile]);
        }
        else {
            fl_color(fl_color_average(fg, bg, eWeight));
            fin = getNodeCanvasCoord(nodeCoord[startF][e.end], fborder[startF]);
        }

        // 划线
        fl_line(beg.first, beg.second, fin.first, fin.second);
    }

    void drawEdge(GraphCommon::node start, GraphCommon::node end, bool stress = false) {
        //find the edge
        GraphCommon::edge de;
        for (auto e : edgeSave[start.file][start.data])
            if (e.end == end.data || e.targetNode == end.data) {
                de = e;
                break;
            }
        drawEdge(de, start.file, stress);
    }

    void drawEdge() {
        for (auto fg : nodeCoord) {
            // 画出画布下的边
            for (auto n = edgeSave[fg.first].begin(); n != edgeSave[fg.first].end(); ++n)
                for (auto e : n->second)
                    drawEdge(e, fg.first);

        }
    }

    void drawRNodes() {
        for (auto n = nodeR.begin(); n != nodeR.end(); ++n)
            drawNode(n->file, n->data);
    }

    void drawPath() {
        auto pathRtmp = pathR;
        auto prev = pathRtmp.front();
        pathRtmp.pop();

        while (!pathRtmp.empty()) {
            auto next = pathRtmp.front();
            pathRtmp.pop();

            drawEdge(prev, next, true);
            drawNode(prev.file, prev.data);
            prev = next;
        }

        drawNode(prev.file, prev.data);
    }

	void draw() {
		fl_rectf(x(), y(), w(), h(), bg);
        refreshFborder();
        if (!nodeR.empty() || !pathR.empty()) dim = true;
        drawEdge();
        if (label > 0)
            drawNodes();
        dim = false;
        if (!nodeR.empty())
            drawRNodes();
        if (!pathR.empty())
            drawPath();
	}

};