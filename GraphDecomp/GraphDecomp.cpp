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
	
	Decomposer decomp(n, fs, subDir, dfs);
	fs.close();
}

void GraphDecomp::Optimize() {
	Optimizer op(n, subDir);
	op.Optimize();
}

bool GraphDecomp::Check() {
	Checker orignalChecker(mainDir);
	Checker decompChecker(subDir, DECOMPFIL);
	Checker optChecker(subDir, OPTFIL);
	return orignalChecker == decompChecker && decompChecker == optChecker;
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

// 有浪费，但大图仍然需要优化
void Decomposer::writeEdgeFile() {
	fstream subfs(getFileString(), fstream::out);
	int nodeLeft = n;
	while (!visitQueue.empty()) {
		int f = visitQueue.front();
		visitQueue.pop();

		for (auto i = adjListGraph[f].begin(); i != adjListGraph[f].end(); ++i) {
			if (i->start == RESNODE) continue;			// 空边跳过
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
			if (isEmptyNodeEdge(adjListGraph, base)) break;			// 孤立节点
			visitQueue.push(base);			// 基点
		}				
		writeEdgeFile();
	}
	
	writeNodeFile();
}

void Decomposer::allocateNodes() {
	// 贪心算法：小船分配
	// 选择最大的连接节点(还是权重？)，分配最小的末枝，减少交叉量
	// nodeSet 将会作为访问过节点的补集

	map<int, int> nodesEdgeLinked;
	for (auto ne : adjListGraph)
		nodesEdgeLinked.insert(make_pair(ne.first, ne.second.size()));
	
	int edgeLeft;
	while (nodesEdgeLinked.empty()) {
		edgeLeft = n;
		while (--edgeLeft) {
			// 选择连接节点中权重最高的
			
		}
	}

}

void Decomposer::DFS() {
	/*
	图分割之后生成的子图之间为什么会有重复节点？分割不就是把节点分到不同的图吗？
	怕有些同学加了很多节点进去。如果没有引进冗余节点，就没有问题。
	*/

	// 需要扫描两遍
	// 第一遍：分配节点
	// 第二遍：分配边

	allocateNodes();

}

void Decomposer::Kerninghan_Lin() {
	// TODO
}

Decomposer::~Decomposer() = default;

Optimizer::Optimizer(int _n, string _subDir) {
	n = _n;
	subDir = _subDir;
	SUFFIX = OPTFIL;

	// 遍历文件
	string fileExtension = ".txt";
	getFiles(_subDir, fileExtension, txt_files, DECOMPFIL);
}

string Optimizer::getOptFileName(string oriPath) {
	return SUFFIX + to_string(parseFileInt(oriPath));
}

void Optimizer::optimizeUnit(string ifn, string ofn) {
	// 对照表。
	// 出现过的就直接引用。

	fstream ifs(ifn, fstream::in);
	fstream ofs(ofn, fstream::out);

	int cnt = 0;

	// 但是需要单独存储为一个文件中。
	// 分布式节点存储。否则会在一个地方存储过多的存储过节点。
	// 可以先分配点。孤立节点依然存于最后一批文件中。

	while (!ifs.eof()) {
		string rl;
		ifs >> rl;
		if (rl == "") break;
		stringstream rs(rl);
		edge e;
		if (find(rl.begin(), rl.end(), DILIMETER) == rl.end()) {
			// 孤立节点的处理
			char ch;
			int node;
			if (rs >> ch >> node && ch == '<') {
				ofs << '<' << node << '>' << endl;
				++cnt;
				continue;
			}
		}
		else rs >> e;			// 边的处理

		//if (isEmptyEdge(e))
		//	continue;			// 读到节点了 继续

		auto sloc = storedNodes.find(e.start);
		auto eloc = storedNodes.find(e.end);
		auto tloc = storedNodes.end();

		if ((sloc != tloc && eloc != tloc && sloc->second != ifn && eloc->second != ifn) ||
			e.start == e.end)
			pendingEdges.push(e);			// 之后再分配
		else {

			// 处理始点
			if (sloc == tloc)
				storedNodes.insert(make_pair(e.start, ifn));
			else if (sloc->second != ifn) {
				e.start = -1;
				e.targetFile = getOptFileName(sloc->second);
				e.targetNode = sloc->first;
			}

			// 处理终点
			if (eloc == tloc)
				storedNodes.insert(make_pair(e.end, ifn));
			else if (eloc->second != ifn) {
				e.end = -1;
				e.targetFile = getOptFileName(eloc->second);
				e.targetNode = eloc->first;
			}

			ofs << e;
			++cnt;
		}

	}

	fileLineCnt[fileNum] = cnt;
	ifs.close();
	ofs.close();
}

void Optimizer::optimizeRemain() {
	// 剩余边再分配，处理起点和终点都不在之前图的情形
	// 需要比较复杂的分配策略（切割策略需要改进）

	vector<string> opt_txt_files;
	getFiles(subDir, ".txt", opt_txt_files, OPTFIL);

	while (!pendingEdges.empty()) {
		edge e = pendingEdges.front();
		pendingEdges.pop();

		auto sloc = storedNodes.find(e.start);
		auto eloc = storedNodes.find(e.end);

		if (sloc->second == eloc->second) {
			int endC = parseFileInt(eloc->second);
			fstream nofs(getFileString(endC), fstream::app);
			nofs << e;
			fileLineCnt[endC]++;
			nofs.close();
		}
		else {
			if (fileLineCnt[parseFileInt(storedNodes[e.start])] >= n) {
				// 放在结束点所在文件
				int endC = parseFileInt(eloc->second);
				fstream nofs(getFileString(endC), fstream::app);
				e.start = -1;
				e.targetFile = getOptFileName(sloc->second);
				e.targetNode = sloc->first;
				nofs << e;
				fileLineCnt[endC]++;
				nofs.close();
			}
			else {
				// 放在开始点所在文件
				int startC = parseFileInt(sloc->second);
				fstream nofs(getFileString(startC), fstream::app);
				e.end = -1;
				e.targetFile = getOptFileName(eloc->second);
				e.targetNode = eloc->first;
				nofs << e;
				fileLineCnt[startC]++;
				nofs.close();
			}
		}
	}

}

void Optimizer::Optimize() {
	fileNum = 1;
	storedNodes.clear();
	while (!pendingEdges.empty()) pendingEdges.pop();

	for (auto i = txt_files.begin(); i != txt_files.end(); ++i) {
		optimizeUnit(*i, getFileString());
		++fileNum;
	}

	optimizeRemain();

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

	// 算法正确性？
	// 考虑虚边 以及只能有一个虚节点的事情
	// 虚边是关键 存储虚边
	// 文件是一个广义节点
	// 访问过的节点将不会被重复访问
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
				int ending = e->end;
				if (ending != -1) {
					if (reachableNodes.find(ending) == reachableNodes.end())
						subVisitQueue.push(ending);
					reachableNodes.insert(ending);
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

	// 打印可达节点
	for (auto i = reachableNodes.begin(); i != reachableNodes.end(); ++i)
		cout << *i << endl;

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

	// 分布式 SPFA 算法
	// Shortest Path Faster Alogrithm
	// SPFA 在形式上和BFS非常类似，不同的是BFS中一个点出了队列就不可能重新进入队列，但是SPFA中一个点可能在出队列之后再次被放入队列，也就是一个点改进过其它的点之后，过了一段时间可能本身被改进，于是再次用来改进其它的点，这样反复迭代下去。

	distance[start] = 0;
	prev[start] = start;

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

			// 考虑指出边
			auto edgeList = subGraphs[cur.first].adjListGraph[tn];
			for (auto e = edgeList.begin(); e != edgeList.end(); ++e) {
				if (e->start == e->end || findLoop(e->start, e->end)) 
					continue;					// 环路不是最短路径
				int ending = e->end;
				if (ending == -1) {
					// 虚边
					visitFileMap[parseFileInt(e->targetFile)].push(e->targetNode);
					ending = e->targetNode;
				}
				if (distance.find(ending) == distance.end() ||
					e->weight <= distance[ending]) {
					distance[ending] = distance[tn] + e->weight;
					prev[ending] = tn;
					subVisitQueue.push(ending);
				}
			}
		}

		for (auto m = visitFileMap.begin(); m != visitFileMap.end(); ++m)
			visitFileQueue.push(*m);
	}

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