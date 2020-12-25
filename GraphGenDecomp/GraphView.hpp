#include "../GraphGen/GraphGen.h"
#include "../GraphDecomp/GraphDecomp.h"
#include "../GUI_facilities.h"

/*开发记录：
	2020 / 12 / 24 ~					接口
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


// 主器图读取器
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
		for (auto n : adjListGraph)
			nodeConnArr[i++] = nodeConn(n.first, (int)n.second.size());
        for (auto in : isoNodes)
            nodeConnArr[i++] = nodeConn(in, 0);
        priorityQueue_m<nodeConn> pnq(nodeConnArr, nsize);
        while (!pnq.isEmpty())
            displayq.push(pnq.deQueue().data);

        delete[] nodeConnArr;
	}
	~SinglePrevReader() = default;

	queue<int> displayq;
private:
	string mainDir;

	struct nodeConn {
		int data;
		int connSize;

		nodeConn(int d = RESNODE, int c = -1) : data(d), connSize(c) {}
		~nodeConn() = default;

		bool operator<(nodeConn &r) const {
			if (connSize < r.connSize) return true;
			return false;
		}
	};
};

// 子图读取器
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

			fstream subfs(f, fstream::in);
			FileUnit subfu;
			subfu.readFile(subfs, false);
			storedFile[curFile] = subfu;
		}
	}
	~MultiPrevReader() = default;

    map<fileNo, queue<int>> storedNodeq;
	map<fileNo, FileUnit> storedFile;
};

class GraphView : public Fl_Widget {
public:
	GraphView(int x, int y, int w, int h, const char* l = NULL) :Fl_Widget(x, y, w, h, l) {

	}

	void RefreshView(string _dir, string _fil = "\0") {
        nodeCoord.clear();
        edgeSave.clear();
		if (_fil == "\0") {
            SinglePrevReader mpr(_dir, true);
            updateCoord(mpr.displayq);
            edgeSave[0] = mpr.adjListGraph;
		}
		else {
			MultiPrevReader opr(_dir, _fil);
            for (auto f : opr.storedNodeq)
                updateCoord(f.second, f.first);
		}
        redraw();
	}

    bool label = true;                                              // 是否标注
private:
    map<fileNo, map<int, pair<float, float>>> nodeCoord;	        // 存储节点坐标
    map < fileNo, map<int, vector<GraphCommon::edge>>> edgeSave;    // 存储文件边集

    pair<int, int> getNodeCanvasCoord(pair<float, float>& ratioCoord, int* fborder) {
        return make_pair(x() + ratioCoord.first * (fborder[0] - fborder[2]) + fborder[2],
            y() + ratioCoord.second * (fborder[1] - fborder[3]) + fborder[3]);
    }

    // 画出画布
    void drawFrame() {
        double fsize = ceil(sqrt(nodeCoord.size()));
        int fpos[2] = { 1,1 };          // 以右下为基准点
        int fborder[4];
        for (auto fg : nodeCoord) {
            //定义画布边界
            fborder[0] = w() * fpos[0] / fsize;           // 右边界
            fborder[1] = h() * fpos[1] / fsize;           // 下边界
            fborder[2] = w() * (fpos[0] - 1) / fsize;     // 左边界
            fborder[3] = h() * (fpos[1] - 1) / fsize;     // 上边界
            
            //画出画布下的节点
            for (auto n = fg.second.begin(); n != fg.second.end(); ++n) {
                fl_color(Color::white);
                auto ncoord = getNodeCanvasCoord(n->second, fborder);
                fl_pie(ncoord.first, ncoord.second, 5, 5, 0, 360);
                //标号
                if (label)
                    fl_draw(to_string(n->first).c_str(), ncoord.first, ncoord.second - 5);
            }

            // 画出画布下的边
            for (auto n = edgeSave[fg.first].begin(); n != edgeSave[fg.first].end(); ++n) {

            }

            //更新下一个画布位置
            if (fpos[0] + 1 > (int)fsize) {
                fpos[1]++;
                fpos[0] = 1;
            }
            else fpos[0]++;
        }
    }

	void draw() {
		fl_rectf(x(), y(), w(), h(), Color::black);
        drawFrame();
	}

    // 蛇形矩阵 1021
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
            dir[0] = 1;                 // 开始前的移动方向：右
            dir[1] = 0;
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

    void updateCoord(queue<int> &pnq, fileNo fn = 0) {
        SnakeArray sa(ceil(sqrt(pnq.size())));
        while (!pnq.empty()) {
            int cur = pnq.front();
            pnq.pop();
            nodeCoord[fn][cur] = sa.getNextPos();
        }
    }
};