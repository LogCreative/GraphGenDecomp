#include "GraphGen.h"

GraphGen::GraphGen(string _directory, nodeType _nt, edgeType _et, isoType _it):
	directory(_directory), nt(_nt), et(_et), it(_it), max_node(100)
{
	
}

void GraphGen::NewGraph(int x) {
	fstream fs(directory, fstream::out);
	int nodeLine = x * randomRatio();
	nodeSet.clear();
	genNode(nodeLine, fs);
	genEdge(x - nodeLine, fs);
	fs.close();
}

void GraphGen::AppendGraph(int y) {
	// in �� app �������
	fstream ifs(directory, fstream::in);
	if (!ifs) error("Cannot open file.");
	nodeSet.clear();
	ReadNode(ifs, nodeSet);
	max_node = *max_element(nodeSet.begin(), nodeSet.end());		// ��ȡ���Ԫ����Ϊ���ֵ
	ifs.close();

	fstream afs(directory, fstream::app);
	genEdge(y, afs);
	afs.close();
}

void GraphGen::genNode(int quantity, fstream& fs) {
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

void GraphGen::genEdge(int quantity, fstream& fs) {
	switch (it) {
	case Single: genSingleGraph(quantity, fs); break;
	case Multi: genMultiGraphs(quantity, fs); break;
	}
}

bool GraphGen::genLayer(int& quantity, fstream& fs, queue<int>& visitQueue) {
	int cur = visitQueue.front(); visitQueue.pop();	 // ��
	visitedNode.insert(cur);
	int childNum = 2 + randomRatio() * (double)(MAX_CHILD - 2);	 // ��������������
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

	// �����ͨ��ͼ�����а��㼶��չ
	int isograph = 1 + randomRatio() * (double)(MAX_ISOGRAPH - 1);		// ������һ����ͨ��ͼ

	vector<queue<int>> visitQueues(isograph, queue<int>{});
	
	// ��ÿһ����ͨ��ͼ����һ�����ڵ�
	for (int i = 0; i < isograph; ++i) 
		visitQueues[i].push(getRandomNode());

	while (quantity) {
		for (int i = 0; i < isograph; ++i) {
			if (visitQueues[i].empty()) continue;
			if (!genLayer(quantity, fs, visitQueues[i])) return;
		}
	}
}


//TODO:��һ��ʱ������Ϊα��������������
double GraphGen::randomRatio() {
	srand((unsigned)time(0)*(++gseed));		// �����������
	return 1.0 * rand() / RAND_MAX;			// ���� 0 ~ 1 ֮��������
}

//double GraphGen::randomRatio() {
//	// ʹ�� boost ������������
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
		else max_node = *std::max_element(nodeSet.begin(), nodeSet.end()); //�������
	}*/
	return res;
}

GraphGen::~GraphGen() {
	
}