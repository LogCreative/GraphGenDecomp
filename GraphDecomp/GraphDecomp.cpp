#include "GraphDecomp.h"

GraphDecomp::GraphDecomp(int _n, string _mainDir, string _subDir) :
	n(_n), mainDir(_mainDir), subDir(_subDir)
{
	ResetSubFolder();
}

void GraphDecomp::ResetSubFolder() {
	string command_rd = "rd /s /q \"" + subDir + '\"';
	system(command_rd.c_str());
	string command_md = "md \"" + subDir + '\"';
	system(command_md.c_str());
}

void GraphDecomp::Decomp() {
	fstream fs(mainDir, fstream::in);
	if (!fs) error("Cannot open main graph file!");
	
	Decomposer decomp(n, fs, subDir, bfs);
	fs.close();
}

double GraphDecomp::Evaluate() {
	Evaluator ev(n, subDir);
	return ev.Evaluate();
}

void GraphDecomp::Optimize() {
	Optimizer op(n, subDir);
	op.Optimize();
}

bool GraphDecomp::Check() {
	Checker orignalChecker(mainDir);
	Checker decompChecker(subDir, DECOMPFIL);
	Checker optChecker(subDir, OPTFIL);
	return orignalChecker == decompChecker 
		//&& decompChecker == optChecker
		;
}

void GraphDecomp::ReachablePoints(int node) {
	Finder nfd(subDir);
	nfd.ReachableNodes(node);
}

void GraphDecomp::ShortestPath(int start, int end) {
	Finder pfd(subDir);
	if (pfd.ShortestPath(start, end) == INF)
		error("These two nodes are not connected!");
}

GraphDecomp::~GraphDecomp() = default;

string Processor::getFileString(fileNo label) {
	return subDir + SUFFIX + to_string(label == -1 ? fileNum : label) + ".txt";
}

int Processor::getFiles(string fileFolderPath, string fileExtension, vector<string>& file, string nameFilter)
{
	// 摘自 https://blog.csdn.net/u014311125/article/details/93076784
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
		fileName = fileFolderPath + '\\' + fileInfo.name;
		if (fileInfo.attrib == _A_ARCH &&
			strstr(fileInfo.name, nameFilter.c_str()) != NULL)			// 添加查找文件的过滤器
		{
			file.push_back(fileName);
		}
	} while (_findnext(findResult, &fileInfo) == 0);

	_findclose(findResult);
}

string Processor::parseFileName(string filePath) {
	auto beg = filePath.find('\\');
	return filePath.substr(beg + 1, filePath.find_last_of('.') - beg - 1);
}

int Processor::parseInt(string str) {
	int n = 0;
	for (auto c : str) {
		if (c >= '0' && c <= '9')
			n = n * 10 + c - '0';
	}
	return n;
}

fileNo Processor::parseFileInt(string filePath) {
	return parseInt(parseFileName(filePath));
}

Decomposer::Decomposer(int _n, fstream& fs, string _subDir, DecompSol sol) {
	fileNum = 1;
	n = _n;
	subDir = _subDir;
	SUFFIX = DECOMPFIL;

	// 读取节点与边
	nodeSet.clear();
	adjListGraph.clear();
	readRawFile(fs);

	switch (sol)
	{
	case bfs:
		BFS();
		break;
	case dfs:
		DFS();
		break;
	case kl:
		break;
	default:
		break;
	}

}

void Decomposer::initialize() {
	nodeMap.clear();
	// 初始化邻接矩阵
	for (auto n : adjListGraph)
		for (auto e : n.second) {
			if (nodeMap.find(n.first) == nodeMap.end())
				nodeMap[n.first] = node(n.first);
			if (nodeMap[n.first].weightAdjCol.find(e.end) == nodeMap[n.first].weightAdjCol.end())
				nodeMap[n.first].weightAdjCol[e.end] = e.weight;
			else nodeMap[n.first].weightAdjCol[e.end] += e.weight;
		}
	// 计算总权重
	for (auto i = nodeMap.begin(); i != nodeMap.end(); ++i)
		i->second.calcTotalWeight();
}

int Decomposer::getMaxWeightNode() const {
	int maxNode = RESNODE;
	double maxWeight = RESNODE;
	for (auto n : nodeMap)
		if (n.second.totalWeight > maxWeight) {
			maxNode = n.second.data;
			maxWeight = n.second.totalWeight;
		}
	return maxNode;
}

int Decomposer::putN2N(int start, int end) {
	nodeMap[start].eraseConn(end);
	vector<vector<edge>::iterator> eraseEdges;	// 迟删除
	for (auto ie = adjListGraph[start].begin(); ie != adjListGraph[start].end(); ++ie)
		if (ie->end == end)
			eraseEdges.push_back(ie);
	for (auto ie : eraseEdges) {
		*subfs << *ie;
		EmptyEdge(*ie);
	}
	return eraseEdges.size();
}

void Decomposer::refreshFile() {
	if (edgeLeft <= 0) {
		subfs->close();
		fileNum++;
		edgeLeft = n;
		subfs = new fstream(getFileString(), fstream::out);
	}
}

void Decomposer::outputIsoNode() {
	if (nodeSet.empty()) subfs->close();		// 关文件！

	// 剩余孤立节点
	for (auto i : nodeSet) {
		*subfs << node(i);
		--edgeLeft;
		refreshFile();
	}
}

void Decomposer::BFS() {
	// 仿照DFS 重写BFS

	initialize();

	subfs = new fstream(getFileString(), fstream::out);
	edgeLeft = n;
	queue<int> visitQueue;
	while (!nodeMap.empty()) {
		visitQueue.push(getMaxWeightNode());
		while (!visitQueue.empty()) {
			int tn = visitQueue.front();
			visitQueue.pop();

			// BFS
			int nn;
			while ((nn = nodeMap[tn].getMaxLinkedNode()) != RESNODE) {
				edgeLeft -= putN2N(tn, nn);
				refreshFile();
				visitQueue.push(nn);
			}

			nodeMap.erase(tn);
			nodeSet.erase(tn);

		}
	}

	outputIsoNode();

}

void Decomposer::DFSUnit(int start) {
	int visiting = nodeMap[start].getMaxLinkedNode();
	if (visiting == RESNODE) {
		nodeMap.erase(start);
		nodeSet.erase(start);
		return; 
	}
	edgeLeft -= putN2N(start, visiting);
	refreshFile();
	DFSUnit(visiting);
}

void Decomposer::DFS() {
	/*
	图分割之后生成的子图之间为什么会有重复节点？分割不就是把节点分到不同的图吗？
	怕有些同学加了很多节点进去。如果没有引进冗余节点，就没有问题。
	*/

	// 需要扫描两遍

	// 第一遍：计算节点
	initialize();

	// 第二遍：贪心分配
	subfs = new fstream(getFileString(), fstream::out);
	edgeLeft = n;
	while (!nodeMap.empty())
		DFSUnit(getMaxWeightNode());

	outputIsoNode();
	
}

void Decomposer::Kerninghan_Lin() {
	// TODO
}

Decomposer::~Decomposer() = default;

Evaluator::Evaluator(int _n, string _subDir) {
	n = _n;
	subDir = _subDir;
	
	getFiles(subDir, ".txt", files, DECOMPFIL);
}

double Evaluator::Evaluate() {
	// 使用原来的优化器思路
	// 遍历文件：第一次出现节点的文件为存储文件（不论是起始还是终止节点）
	// 存储至已经存储的节点集合中
	// 通过优化每一个文件，确定每一条边是否跨越文件
	// 如果是，就增加边的损失量

	double edgeLoss = 0;

	return edgeLoss;
}

Evaluator::~Evaluator() = default;

Optimizer::Optimizer(int _n, string _subDir) {
	n = _n;
	subDir = _subDir;
	SUFFIX = OPTFIL;

	// 遍历文件
	getFiles(_subDir, ".txt", txt_files, DECOMPFIL);
}

string Optimizer::getOptFileName(string oriPath) {
	return SUFFIX + to_string(parseFileInt(oriPath));
}

void Optimizer::allocateNodes() {

}

void Optimizer::allocateEdges() {

}

void Optimizer::Optimize() {
	// 第一次出现节点为起始节点的文件为存储文件
	// 第一遍扫描：需要先分配节点存储位置
	// 用文件-节点映射表存储
	allocateNodes();
	
	// 第二遍扫描：分配边，最多只有边的终节点为虚节点
	// 如果分配后两点为同一个文件，为实边
	// 否则接在对于起始节点存储文件后面
	// 原则：起始节点不能成为虚节点
	allocateEdges();

	// 这种优化方法将可能突破原有的文件行数限制
	// 比如星状图。


}

Optimizer::~Optimizer() = default;

Checker::Checker(string _subDir, string _filter){
	if (_filter == "") {
		fstream fs(_subDir, fstream::in);
		readRawFile(fs);
		fs.close();
	}
	else {
		subDir = _subDir;
		string fileExtension = ".txt";
		vector<string> files;
		getFiles(subDir, fileExtension, files, _filter);
		for (auto f = files.begin(); f != files.end(); ++f) {
			fstream cf(*f, fstream::in);
			readFile(cf, true);
			cf.close();
		}
	}
}

template<typename K>
bool operator==(const set<K>& set1, const set<K>& set2) {
	if (set1.size() != set2.size())
		return false;

	for (auto itl = set1.begin(), itr = set2.begin(); itl != set1.end(); ++itl, ++itr) 
		if (*itl != *itr) 
			return false;

	return true;
}

bool CompareMap(const map<int, vector<GraphCommon::edge>> &map1, const map<int, vector<GraphCommon::edge>> &map2) {
	if (map1.size() != map2.size())
		return false;

	for (auto it1 : map1) {
		auto it2t = map2.find(it1.first);
		if (it2t == map2.end()) return false;
		else {
			auto it2 = *it2t;			// 需要转为局部变量进行比较
			sort(it1.second.begin(), it1.second.end());
			sort(it2.second.begin(), it2.second.end());
			if (!equal(it1.second.begin(), it1.second.end(), it2.second.begin()))
				return false;
		}
	}

	return true;
}

bool operator==(Checker const &l, Checker const &r) {
	if (l.nodeSet == r.nodeSet 
		&& CompareMap(l.adjListGraph, r.adjListGraph)
		)
		return true;
	return false;
}

Checker::~Checker() = default;

Finder::Finder(string _subDir) {
	subDir = _subDir;
	SUFFIX = OPTFIL;
	getFiles(subDir, ".txt", files, SUFFIX);
}

string Finder::findStoredFile(int node) {
	for (auto f = files.begin(); f != files.end(); ++f) {
		fstream ff(*f, fstream::in);
		while (!ff.eof()) {
			string fl;
			ff >> fl;
			stringstream fss(fl);
			char ch1, ch2;
			int start, end;
			if (fss >> ch1 >> start >> ch2 >> end) {
				if (start == node || end == node) {
					ff.close();
					return *f;
				}
			}
		}
		ff.close();
	}
	return "";
}

void Finder::loadSubgraph(fileNo fn) {
	fstream rf(getFileString(fn), fstream::in);
	FileUnit fu;
	fu.readFile(rf);
	subGraphs[fn] = fu;
}

void Finder::searchReachableNodes(int node) {
	string initialFile = findStoredFile(node);	// 搜索开始基点
	if (initialFile == "") error("No such node is found!");
	//reachableNodes.insert(node);				// 起始节点从一开始不是可达点
	visitFileQueue.push(make_pair(parseFileInt(initialFile), queue<int>({ node })));

	// 算法正确性？- 需要保证只有终结点可能是虚节点
	// 考虑虚边 以及只能有一个虚节点的事情
	// 虚边是关键 存储虚边
	// 文件是一个广义节点
	// 访问过的节点将不会被重复访问!
	// 访问过的文件的图将会被存储 不会被重新加载

	while (!visitFileQueue.empty()) {
		auto cur = visitFileQueue.front();
		visitFileQueue.pop();

		if (subGraphs.find(cur.first) == subGraphs.end())
			loadSubgraph(cur.first);

		queue<int> subVisitQueue = cur.second;
		map<fileNo, queue<int>> visitFileMap;
		
		while (!subVisitQueue.empty()) {
			int tn = subVisitQueue.front();
			subVisitQueue.pop();

			// BFS 搜索，考虑指出边
			auto edgeList = subGraphs[cur.first].adjListGraph[tn];
			for (auto e = edgeList.begin(); e != edgeList.end(); ++e) {
				if (isEmptyEdge(*e)) continue;
				int ending = e->end;
				if (ending != -1) {
					if (subGraphs[cur.first].visitedNode.find(ending) == subGraphs[cur.first].visitedNode.end()) {
						subVisitQueue.push(ending);
						subGraphs[cur.first].visitedNode.insert(ending);
						reachableNodes.insert(ending);
					}
				}
				else {
					// 虚边
					visitFileMap[parseFileInt(e->targetFile)].push(e->targetNode);
					reachableNodes.insert(e->targetNode);
				}
			}
			
		}

		for (auto m = visitFileMap.begin(); m != visitFileMap.end(); ++m)
			visitFileQueue.push(*m);
	}
}

void Finder::ReachableNodes(int node) {
	searchReachableNodes(node);
	// findReachableNodes(node);

	// 打印可达节点

	cout << "可达节点数目：" << reachableNodes.size() << endl;
	for (auto i = reachableNodes.begin(); i != reachableNodes.end(); ++i)
		cout << *i << ' ';
	cout << endl;
}

bool Finder::findLoop(int cur, int target) {
	if (cur == target) return true;
	if (prev[cur] == cur) return false;			// 追溯到起始点
	return findLoop(prev[cur], target);
}

double Finder::findShortestPath(int start, int end) {
	map<int, double> distance;

	string initialFile = findStoredFile(start);	// 搜索开始基点
	if (initialFile == "") error("No such node is found!");
	visitFileQueue.push(make_pair(parseFileInt(initialFile), queue<int>({ start })));

	// 分布式 Dijkstra 算法
	// 优化器可以保证能够找到起始点的所有指向终点。

	distance[start] = 0;
	prev[start] = start;

	// TODO

	if (distance.find(end) == distance.end())
		return INF;
	return distance[end];
}

void Finder::prtPath(int cur, int target, int finish) {
	if (cur != target) prtPath(prev[cur], target, finish);
	cout << cur << (cur == finish ? "\n" : "->");
}

double Finder::ShortestPath(int start, int end) {
	double res = findShortestPath(start, end);
	if (res != INF) { 
		prtPath(end, start, end);
		cout << "路径长度：" << res << endl; 
	}
	return res;
}

Finder::~Finder() = default;