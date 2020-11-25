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
	fs.seekg(0,fstream::beg);	// �����ļ�ͷ
	readEdge(fs, adjListGraph);
	
	// �������ڵ�洢��һ���ļ���
	
	// ���Էּ�����Ʋ�ͬ���㷨��
	// trade off����Ȩ���٣�����ڵ�ࣻ����������Ȩ�ض�

	fs.close();
}

void GraphDecomp::Optimize() {

}

bool GraphDecomp::Check() {
	return true;
}

void GraphDecomp::ReachablePoints(int node) {
	// ͼ�Ĵ洢���ڽӱ�
}

void GraphDecomp::ShortestPath(int start, int end) {
	// Dijkstra
}

GraphDecomp::~GraphDecomp() = default;