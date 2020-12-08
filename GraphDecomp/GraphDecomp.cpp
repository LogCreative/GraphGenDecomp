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
	// 图的存储：邻接表
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
	// 读取节点与边
	nodeSet.clear();
	readNode(fs, nodeSet);
	adjListGraph.clear();
	fs.clear(); // 如果在文件已经读取到结尾时，fstream的对象会将内部的eof state置位，这时使用 seekg() 函数不能将该状态去除，需要使用 clear() 方法。
	fs.seekg(0, fstream::beg);	// 返回文件头
	readEdge(fs, adjListGraph);

	// 将孤立节点存储到一个文件中

	// 可以分级，设计不同的算法，
	// trade off：边权重少，但虚节点多；连续，但边权重多
	// int leastGraphNum = ceil((double)nodeSet.size() / n);

	// BFS();
	// 图谱学

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

// 有浪费，但大图仍然需要优化
void Decomposer::writeEdgeFile() {
	fstream subfs(getFileString(), fstream::out);
	int nodeLeft = n;
	while (!visitQueue.empty()) {
		int f = visitQueue.front();
		visitQueue.pop();

		for (auto i = adjListGraph[f].begin(); i != adjListGraph[f].end(); ++i) {
			if (i->start == 0) continue;			// 空边跳过
			if(adjListGraph.find(i->end)!=adjListGraph.end()) 
				visitQueue.push(i->end);			// 其指向的子节点全部进队
			subfs << *i;							// 输出该边
			if (--nodeLeft == 0) {
				subfs.close();
				++fileNum;
				return;
			}
		}

		// 该节点输出完毕，退出
		adjListGraph.erase(f);
	}
	if (nodeLeft != 0)								// 该文件没有写满
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
	// 由于邻接表存储方法擅长存储的是起始点和起始边
	// 以最大连接节点作为基点
	while (!visitQueue.empty()) visitQueue.pop();
	
	// 输出一个子图所有连接边
	// 每个文件最多存一个子图，减少连接交叉

	// 0 号文件存储的是孤立节点

	while (!adjListGraph.empty()) {
		if (visitQueue.empty()) { 
			int base = maxlinked_node();
			int maxNum = adjListGraph[base].size();
			if (isEmptyEdge(adjListGraph, base)) break;			// 孤立节点
			visitQueue.push(base);			// 基点
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
	// 遍历文件
	string fileExtension = ".txt";
	get_files(_subDir, fileExtension, txt_files);
}

// 摘自 https://blog.csdn.net/u014311125/article/details/93076784
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
	// 设计？
	stringstream ss("<1,-1,<G1.2,2.5>");
	edge e;
	ss >> e;
}

Optimizer::~Optimizer() = default;