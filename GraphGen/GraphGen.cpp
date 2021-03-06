#include "GraphGen.h"

int MAX_INCREASEMENT = 100;
int MAX_ISOGRAPH = 5;
int MAX_CHILD = 5;

GraphGen::GraphGen(string _directory, nodeType _nt, edgeType _et, isoType _it):
	directory(_directory), nt(_nt), et(_et), it(_it), max_node(100)
{
	
}

void GraphGen::NewGraph(int x, double prop) {
	fstream fs(directory, fstream::out);
	int nodeLine = x * randomRatio();		// 不论是否被指定，都需要这么多的节点数。
	nodeSet.clear();
	if (prop < 0) { 
		genNode(nodeLine, fs, -1);
		genEdge(x - nodeLine, fs);
	}
	else {
		genNode(nodeLine, fs, x * prop);
		genEdge(x - (int)x * prop, fs);
	}
	fs.close();
}

void GraphGen::AppendGraph(int y) {
	// in 和 app 不能配合
	fstream ifs(directory, fstream::in);
	if (!ifs) error("Cannot open file.");
	nodeSet.clear();
	ReadNode(ifs);
	max_node = *max_element(nodeSet.begin(), nodeSet.end());		// 获取最大元素作为最大值
	ifs.close();

	fstream afs(directory, fstream::app);
	genEdge(y, afs);
	afs.close();
}

void GraphGen::genNode(int quantity, fstream& fs, int oline) {
	switch (nt) {
	case continuous:
		for (int i = 0; i < quantity; ++i) {
			nodeSet.insert(i);
			if (oline == -1 || oline > 0) {
				fs << '<' << i << ">\n";
				if (oline != -1) --oline;
			}
		}
		max_node = quantity - 1;
		break;
	case discrete:
		int cur = 0;
		for (int i = 0; i < quantity; ++i) {
			cur += randomRatio() * MAX_INCREASEMENT;
			nodeSet.insert(cur);
			if (oline == -1 || oline > 0) {
				fs << '<' << i << ">\n";
				if (oline != -1) --oline;
			}
		}
		max_node = cur;
		break;
	}
}

void GraphGen::genEdge(int quantity, fstream& fs) {
	switch (it) {
	case Single: genSingleGraph(quantity, fs); break;
	case Multi: genMultiGraphs(quantity, fs); break;
	}
}

bool GraphGen::genLayer(int& quantity, fstream& fs, queue<int>& visitQueue) {
	int cur = visitQueue.front(); visitQueue.pop();	 // 根
	visitedNode.insert(cur);
	int childNum = 2 + randomRatio() * (double)(MAX_CHILD - 2);	 // 至少有两个孩子
	while (--childNum) {
		int childNode = getRandomNode();
		if (et == Graph ||
			et == Tree && visitedNode.find(childNode) == visitedNode.end()) {
			visitQueue.push(childNode);
			edge e(cur, childNode, randomRatio() * max_node / 10);
			fs << e;
			if (--quantity == 0) return false;
		}
	}
	return true;
}

void GraphGen::genSingleGraph(int quantity, fstream& fs) {
	visitedNode.clear();
	queue<int> visitQueue;
	visitQueue.push(getRandomNode());
	while (!visitQueue.empty()) 
		if(!genLayer(quantity, fs, visitQueue)) return ;	
}

void GraphGen::genMultiGraphs(int quantity, fstream& fs) {
	visitedNode.clear();

	// 多个联通子图，并行按层级扩展
	int isograph = 1 + randomRatio() * (double)(MAX_ISOGRAPH - 1);		// 至少有一个连通子图

	vector<queue<int>> visitQueues(isograph, queue<int>{});
	
	// 对每一个联通子图推入一个根节点
	for (int i = 0; i < isograph; ++i) 
		visitQueues[i].push(getRandomNode());

	while (quantity) {
		for (int i = 0; i < isograph; ++i) {
			if (visitQueues[i].empty()) continue;
			if (!genLayer(quantity, fs, visitQueues[i])) return;
		}
	}
}


//TODO:在一段时间内仍为伪随机数，不够随机
double GraphGen::randomRatio() {
	srand((unsigned)time(0)*(++gseed));		// 产生随机种子
	return 1.0 * rand() / RAND_MAX;			// 产生 0 ~ 1 之间的随机数
}

//double GraphGen::randomRatio() {
//	// 使用 boost 库产生真随机数
//	boost::mt19937 rng(time(0));
//	boost::uniform_01<boost::mt19937&> u01(rng);
//	return u01();
//}

int GraphGen::getRandomNode() {
	int res = randomRatio() * max_node;
	auto iter = nodeSet.lower_bound(res);
	if(iter!=nodeSet.end()) res = *iter;
	/*if(iter!=nodeSet.end()) nodeSet.erase(iter);
	if (res == max_node) { 
		if (nt == continuous) --max_node;
		else max_node = *std::max_element(nodeSet.begin(), nodeSet.end()); //不清除了
	}*/
	return res;
}

GraphGen::~GraphGen() {
	
}

// TODO: 边生成边边生成点 可以直接只生成边
// 随机化程度更高一些
// 点可以直接连续生成
// 可以参考老师的样例