#include "GraphDecomp.h"

GraphDecomp::GraphDecomp(string _mainDir, string _subDir):
	mainDir(_mainDir), subDir(_subDir)
{

}

void GraphDecomp::Decomp(int n) {
	fstream fs(mainDir, fstream::in);
	if (!fs) error("Cannot open main graph file!");
	
	Decomposer decomp(n, fs, subDir);

	fs.close();
}

void GraphDecomp::Optimize() {

}

bool GraphDecomp::Check() {
	return true;
}

void GraphDecomp::ReachablePoints(int node) {
	// ͼ�Ĵ洢���ڽӱ�
	/* //naive method
	vector<edge> connectedEdges = adjListGraph[node];
	set<int> reachablePoints;
	for (auto e : connectedEdges) reachablePoints.insert(e.end);
	for (auto n : reachablePoints) cout << n << endl;
	*/
}

void GraphDecomp::ShortestPath(int start, int end) {
	// Dijkstra

}

GraphDecomp::~GraphDecomp() = default;

Decomposer::Decomposer(int _n, fstream &fs, string _subDir): 
	n(_n), fileNum(1), subDir(_subDir) {
	// ��ȡ�ڵ����
	nodeSet.clear();
	readNode(fs, nodeSet);
	adjListGraph.clear();
	fs.clear(); // ������ļ��Ѿ���ȡ����βʱ��fstream�Ķ���Ὣ�ڲ���eof state��λ����ʱʹ�� seekg() �������ܽ���״̬ȥ������Ҫʹ�� clear() ������
	fs.seekg(0, fstream::beg);	// �����ļ�ͷ
	readEdge(fs, adjListGraph);

	// �������ڵ�洢��һ���ļ���

	// ���Էּ�����Ʋ�ͬ���㷨��
	// trade off����Ȩ���٣�����ڵ�ࣻ����������Ȩ�ض�
	int leastGraphNum = ceil((double)nodeSet.size() / n);

	BFS();
	// ͼ��ѧ

	// Kerninghan_Lin(n);

}

int Decomposer::maxlinked_node() {
	auto res = adjListGraph.begin();
	for (auto i = adjListGraph.begin(); i != adjListGraph.end(); ++i)
		if (i->second.size() > res->second.size())
			res = i;
	return res->first;
}

void Decomposer::writeFile() {
	string subdirectory = subDir + FILENAME + to_string(fileNum) + ".txt";
	fstream subfs(subdirectory, fstream::out);
	int nodeLeft = n;
	while (!visitQueue.empty()) {
		int f = visitQueue.front();
		visitQueue.pop();

		for (auto i = adjListGraph[f].begin(); i != adjListGraph[f].end(); ++i) {
			if(adjListGraph.find(i->end)!=adjListGraph.end()) 
				visitQueue.push(i->end);				// ��ָ����ӽڵ�ȫ������
			subfs << *i;							// ����ñ�
			if (--nodeLeft == 0) {
				subfs.close();
				++fileNum;
				return;
			}
		}

		// �ýڵ������ϣ��˳�
		adjListGraph.erase(f);
	}
}

void Decomposer::BFS() {
	// �����ڽӱ�洢�����ó��洢������ʼ�����ʼ��
	// ��������ӽڵ���Ϊ����
	while (!visitQueue.empty()) visitQueue.pop();
	
	// ���һ����ͼ�������ӱ�
	// ÿ���ļ�����һ����ͼ���������ӽ���

	// 0 ���ļ��洢���ǹ����ڵ�

	while (!adjListGraph.empty()) {
		if (visitQueue.empty()) { 
			int base = maxlinked_node();
			if (adjListGraph[base].size() == 0) break;			// �����ڵ�
			visitQueue.push(base);			// ����
		}				
		writeFile();
	}
	
	string subdirectory = subDir + FILENAME + "0.txt";
	fstream subfs(subdirectory, fstream::out);
	for (auto i = adjListGraph.begin(); i != adjListGraph.end(); ++i) 
		subfs << '<' << i->first << '>' << endl;
	subfs.close();
}

void Decomposer::Kerninghan_Lin() {

}

Decomposer::~Decomposer() = default;