#include "GraphGen.h"

GraphGen::GraphGen(string _directory, nodeType _nt, edgeType _et/*, randomType _rt*/):
	directory(_directory), nt(_nt), et(_et)/*, rt(_rt)*/
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
	fstream fs(directory, fstream::in | fstream::app);
	if (!fs) error("Cannot open file.");
	nodeSet.clear();
	ReadNode(fs, nodeSet);

	fs.close();
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
		// 0 ~ 300000 100 Ϊ�����Χ
		int cur = 0;
		for (int i = 0; i < quantity; ++i) {
			cur += randomRatio() * 100;
			nodeSet.insert(cur);
			fs << '<' << cur << ">\n";
		}
		max_node = cur;
		break;
	}
}

void GraphGen::GenEdge(int quantity, fstream& fs) {
	queue<int> visitQueue;
	switch (et) {
	case Tree:
		visitQueue.push(getRandomNode());
		while (!visitQueue.empty()) {
			int cur = visitQueue.front(); visitQueue.pop();	 // ��
			int childNum = 1 + randomRatio() * (MAX_CHILD - 1);
			while(--childNum) {
				int childNode = getRandomNode();
				visitQueue.push(childNode);
				edge e(cur, childNode, randomRatio() * max_node);
				fs << e.output() << endl;
				if (--quantity == 0) return;
			}
		}
		break;
	case Graph:
			
		break;
	default:
		break;
	}
	
}

double GraphGen::randomRatio() {
	srand((unsigned)time(0)*(++gseed));		// TODO:����������ӣ���һ��ʱ������Ϊα�����
	return 1.0 * rand() / RAND_MAX;			// ���� 0 ~ 1 ֮��������
}

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