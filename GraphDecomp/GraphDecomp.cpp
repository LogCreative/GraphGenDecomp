#include "GraphDecomp.h"

GraphDecomp::GraphDecomp(int _n, string _mainDir, string _subDir):
	n(_n), mainDir(_mainDir), subDir(_subDir)
{
	
}

void GraphDecomp::Decomp() {
	fstream fs(mainDir, fstream::in);
	if (!fs) error("Cannot open main graph file!");
	
	Decomposer decomp(n, fs, subDir);
	decomp.BFS();

	fs.close();
}

void GraphDecomp::Optimize() {
	Optimizer op(n, subDir);
	op.Optimize();
}

bool GraphDecomp::Check() {
	Checker decompChecker(subDir, DECOMPFIL);
	Checker optChecker(subDir, OPTFIL);
	return decompChecker == optChecker;
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

string Processor::getFileString(int label) {
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

Decomposer::Decomposer(int _n, fstream &fs, string _subDir) {
	fileNum = 1;
	n = _n;
	subDir = _subDir;
	SUFFIX = DECOMPFIL;

	// 读取节点与边
	nodeSet.clear();
	adjListGraph.clear();
	readFile(fs);

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
			if (isEmptyNodeEdge(adjListGraph, base)) break;			// 孤立节点
			visitQueue.push(base);			// 基点
		}				
		writeEdgeFile();
	}
	
	writeNodeFile();
}

void Decomposer::DFS() {

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

string Optimizer::parseFileName(string filePath) {
	auto beg = filePath.find('\\');
	return filePath.substr(beg + 1, filePath.find_last_of('.') - beg - 1);
}

int Optimizer::parseInt(string str) {
	int n = 0;
	for (auto c : str) {
		if (c >= '0' && c <= '9')
			n = n * 10 + c - '0';
	}
	return n;
}

int Optimizer::parseFileInt(string filePath) {
	return parseInt(parseFileName(filePath));
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

		if (sloc != tloc && eloc != tloc || 
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

		if (e.start == e.end) {		// 自环
			// 虚边
			if (fileLineCnt[parseFileInt(storedNodes[e.start])] >= n) {
				// 放在结束点所在文件
				int endC = parseFileInt(eloc->second);
				fstream nofs(getFileString(endC), fstream::app);
				e.end = -1;
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
				e.start = -1;
				e.targetFile = getOptFileName(eloc->second);
				e.targetNode = eloc->first;
				nofs << e;
				fileLineCnt[startC]++;
				nofs.close();
			}
		}
		else {
			// 考虑新的策略
			int endC = parseFileInt(eloc->second);
			fstream nofs(getFileString(endC), fstream::app);
			nofs << e;
			fileLineCnt[endC]++;
			nofs.close();
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

Checker::Checker(string _subDir, string _filter) {
	subDir = _subDir;
	string fileExtension = ".txt";
	vector<string> files;
	getFiles(subDir, fileExtension, files, _filter);
	for (auto f = files.begin(); f != files.end(); ++f) {
		fstream cf(*f, fstream::in);
		readFile(cf);
		cf.close();
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

