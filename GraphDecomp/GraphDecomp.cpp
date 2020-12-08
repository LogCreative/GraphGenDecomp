#include "GraphDecomp.h"

GraphDecomp::GraphDecomp(string _mainDir, string _subDir):
	mainDir(_mainDir), subDir(_subDir)
{

}

void GraphDecomp::Decomp(int n) {
	fstream fs(mainDir, fstream::in);
	if (!fs) error("Cannot open main graph file!");
	
	Decomposer decomp(n, fs, subDir);
	decomp.BFS();

	fs.close();
}

void GraphDecomp::Optimize() {
	Optimizer op(subDir);
	op.Optimize();
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
	// int leastGraphNum = ceil((double)nodeSet.size() / n);

	// BFS();
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

string Decomposer::getFileString() {
	return subDir + FILENAME + to_string(fileNum) + ".txt";
}

// ���˷ѣ�����ͼ��Ȼ��Ҫ�Ż�
void Decomposer::writeEdgeFile() {
	fstream subfs(getFileString(), fstream::out);
	int nodeLeft = n;
	while (!visitQueue.empty()) {
		int f = visitQueue.front();
		visitQueue.pop();

		for (auto i = adjListGraph[f].begin(); i != adjListGraph[f].end(); ++i) {
			if (i->start == 0) continue;			// �ձ�����
			if(adjListGraph.find(i->end)!=adjListGraph.end()) 
				visitQueue.push(i->end);			// ��ָ����ӽڵ�ȫ������
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
	if (nodeLeft != 0)								// ���ļ�û��д��
		++fileNum;
}

void Decomposer::writeNodeFile() {
	int nodeLeft = 0;
	fstream* subfs = NULL;
	for (auto i = adjListGraph.begin(); i != adjListGraph.end(); ++i) {
		if (nodeLeft == 0) {
			if (subfs) subfs->close();
			subfs = new fstream(getFileString(), fstream::out);
			++fileNum;
			nodeLeft = n;
		}
		*subfs << '<' << i->first << '>' << endl;
		--nodeLeft;
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
			int maxNum = adjListGraph[base].size();
			if (isEmptyEdge(adjListGraph, base)) break;			// �����ڵ�
			visitQueue.push(base);			// ����
		}				
		writeEdgeFile();
	}
	
	writeNodeFile();
}

void Decomposer::Kerninghan_Lin() {
	// TODO
}

Decomposer::~Decomposer() = default;

Optimizer::Optimizer(string _subDir) {
	// �����ļ�
	string fileExtension = ".txt";
	get_files(_subDir, fileExtension, txt_files);
}

// ժ�� https://blog.csdn.net/u014311125/article/details/93076784
int Optimizer::get_files(string fileFolderPath, string fileExtension, vector<string>& file)
{
	string fileFolder = fileFolderPath + "\\*" + fileExtension;
	string fileName;
	struct _finddata_t fileInfo;
	long long findResult = _findfirst(fileFolder.c_str(), &fileInfo);
	if (findResult == -1)
	{
		_findclose(findResult);
		return 0;
	}
	bool flag = 0;

	do
	{
		fileName = fileFolderPath + "\\" + fileInfo.name;
		if (fileInfo.attrib == _A_ARCH)
		{
			file.push_back(fileName);
		}
	} while (_findnext(findResult, &fileInfo) == 0);

	_findclose(findResult);
}

void Optimizer::Optimize() {
	// ��ƣ�
	stringstream ss("<1,-1,<G1.2,2.5>");
	edge e;
	ss >> e;
}

Optimizer::~Optimizer() = default;