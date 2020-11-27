#include "GraphDecomp.h"

GraphDecomp::GraphDecomp(string _mainDir, string _subDir):
	mainDir(_mainDir), subDir(_subDir)
{

}

void GraphDecomp::Decomp(int n) {
	fstream fs(mainDir, fstream::in);
	if (!fs) error("Cannot open main graph file!");
	
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

	bfs(n);
	// ͼ��ѧ

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

void GraphDecomp::bfs(int n) {

}

GraphDecomp::~GraphDecomp() = default;