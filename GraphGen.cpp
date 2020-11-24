#include "GraphGen.h"

GraphGen::GraphGen(string _directory, nodeType _nt, edgeType _et):
	directory(_directory), nt(_nt), et(_et), max_node(100)
{
	
}

void GraphGen::newGraph(int x) {
	fstream fs(directory, fstream::out);
	int nodeLine = x * randomRatio();
	nodeSet.clear();
	GenNode(nodeLine, fs);
	GenEdge(x - nodeLine, fs);
	fs.close();
}

void GraphGen::appendGraph(int y) {
	// in 和 app 不能配合
	fstream ifs(directory, fstream::in);
	if (!ifs) error("Cannot open file.");
	nodeSet.clear();
	ReadNode(ifs, nodeSet);
	max_node = *max_element(nodeSet.begin(), nodeSet.end());		// 获取最大元素作为最大值
	ifs.close();

	fstream afs(directory, fstream::app);
	GenEdge(y, afs);
	afs.close();
}

void GraphGen::GenNode(int quantity, fstream& fs) {
	switch (nt) {
	case continuous:
		for (int i = 0; i < quantity; ++i) {
			nodeSet.insert(i);
			fs << '<' << i << ">\n";
		}
		max_node = quantity - 1;
		break;
	case discrete:
		int cur = 0;
		for (int i = 0; i < quantity; ++i) {
			cur += randomRatio() * MAX_INCREASEMENT;
			nodeSet.insert(cur);
			fs << '<' << cur << ">\n";
		}
		max_node = cur;
		break;
	}
}

void GraphGen::GenEdge(int quantity, fstream& fs) {
	queue<int> visitQueue;
	set<int> visitedNode;
	visitQueue.push(getRandomNode());
	while (!visitQueue.empty()) {
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
				if (--quantity == 0) return;
			}
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