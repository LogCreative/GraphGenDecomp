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

void GraphDecomp::Decomp(DecompSol sol) {
	fstream fs(mainDir, fstream::in);
	if (!fs) error("Cannot open main graph file!");
	
	Decomposer decomp(n, fs, subDir, sol);
	fs.close();
}

/*
double GraphDecomp::Evaluate() {
	Evaluator ev(n, subDir);
	return ev.Evaluate();
}
*/

void GraphDecomp::Optimize() {
	Optimizer op(n, subDir);
	op.Optimize();
}

bool GraphDecomp::Check() {
	Checker orignalChecker(mainDir);
	Checker decompChecker(subDir, DECOMPFIL);
	Checker optChecker(subDir, OPTFIL);
	return orignalChecker == decompChecker 
		&& decompChecker == optChecker
		;
}

/*
void GraphDecomp::ReachablePoints(int node) {
	Finder nfd(subDir);
	nfd.ReachableNodes(node);
}

void GraphDecomp::ShortestPath(int start, int end) {
	Finder pfd(subDir);
	if (pfd.ShortestPath(start, end) == INF)
		error("These two nodes are not connected!");
}
*/
GraphDecomp::~GraphDecomp() = default;

string Processor::getFileString(fileNo label) const{
	return subDir + SUFFIX + to_string(label == -1 ? fileNum : label) + ".txt";
}

int Processor::getFiles(string fileFolderPath, string fileExtension, vector<string>& file, string nameFilter)
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

void Processor::refreshFile() {
	subfs->close();
	fileNum++;
	subfs = new fstream(getFileString(), fstream::out);
}

Decomposer::Decomposer(int _n, fstream& fs, string _subDir, DecompSol _sol) {
	fileNum = 1;
	n = _n;
	subDir = _subDir;
	SUFFIX = DECOMPFIL;
	sol = _sol;

	// 读取节点与边
	isoNodes.clear();
	adjListGraph.clear();
	readRawFile(fs);

	Kerninghan_Lin();
}

void Decomposer::initialAdjMat() {
	adjMat.clear();
	for (auto n : adjListGraph) {
		nodeStruct ns(n.first);
		for (auto e : adjListGraph[n.first])
			ns.inputEdge(e);
		adjMat[n.first] = ns;
	}
}

void Decomposer::initialCostMat() {
	costMat.clear();
	for (auto i : adjMat) 
		for (auto j : adjMat) 
			costMat[i.first][j.first] = 
			costMat[j.first][i.first] = 
				adjMat[i.first].getConnWeight(j.first)
				+ adjMat[j.first].getConnWeight(i.first);
}

void Decomposer::divide(set<int> S) {
	// 开始时直接按序号将连通节点分为两半
	set<int> A, B;
	set<int>::iterator it = S.begin();
	for(int size = (S.size() + 1) / 2; size > 0; --size){
		A.insert(*it);
		++it;
	}
	for (; it != S.end(); ++it)
		B.insert(*it);
	
	optimizeParts(A, B);

	partitions.push(A);
	partitions.push(B);
}

void Decomposer::optimizeParts(set<int> &A, set<int> &B) {
	if (sol != rough) {
		// 计算外内差 D

	}
	switch (sol)
	{
	case rough:
		return;				// 粗略分配将不会进行优化
	case ll:

		break;
	case kl:
		
		break;
	default:
		break;
	}
}

void Decomposer::Kerninghan_Lin() {

	// 初始化邻接矩阵
	initialAdjMat();

	// 初始化损失矩阵
	initialCostMat();

	// 分配连通节点
	set<int> connNodes;
	for (auto n : adjListGraph)
		connNodes.insert(n.first);
	
	// 每一步都是局部最优
	// 最终可能是全局最优
	partitions.push(connNodes);
	int m_size = connNodes.size();	// 下一步的切割大小
	while (m_size > n) {
		queue<set<int>> partitionSubq;
		while (!partitions.empty()) {
			partitionSubq.push(partitions.front());
			partitions.pop();
		}
		while (!partitionSubq.empty()) {
			set<int> partition = partitionSubq.front();
			partitionSubq.pop();
			divide(partition);		// 会进队另外两半
			m_size = (partition.size() + 1) / 2;
		}
	}	

	// 孤立节点直接最后输出，分配
	allocateIsoNodes();

	// 输出分配情况作为验证文件
	OuputPartitions();

	// 连通子图输出
	outputSubAdjGraphs();
}

void Decomposer::OuputPartitions() const {
	queue<set<int>> outputPartq = partitions;
	fstream partfs(subDir + "\\partitions.txt", fstream::out);
	while (!outputPartq.empty()) {
		set<int> s = outputPartq.front();
		outputPartq.pop();
		for (auto n : s)
			partfs << n << ' ';
		partfs << endl;
	}
	partfs.close();
}

void Decomposer::outputSubAdjGraphs() {
	while (!partitions.empty()) {
		set<int> s = partitions.front();
		partitions.pop();
		fstream subfs(getFileString(), fstream::out);
		fileNum++;
		for (auto n : s) {
			subfs << node(n);
			for (auto e : adjListGraph[n])
				subfs << e;
		}
		subfs.close();
	}
}

void Decomposer::allocateIsoNodes() {
	set<int> isoSet;
	for (auto in : isoNodes) {
		isoSet.insert(in);
		if (isoSet.size() == n) {
			partitions.push(isoSet);
			isoSet.clear();
		}
	}
	if (!isoSet.empty())
		partitions.push(isoSet);
}

Decomposer::~Decomposer() = default;

// Evaluator

Optimizer::Optimizer(int _n, string _subDir) {
	n = _n;
	subDir = _subDir;
	SUFFIX = OPTFIL;

	// 遍历文件
	getFiles(_subDir, ".txt", files, DECOMPFIL);
}

void Optimizer::getNodesAllocation() {
	for (auto f = files.begin(); f != files.end(); ++f) {
		fstream subfs(*f, fstream::in);
		fileNo cur = parseFileInt(*f);
		fileUnit fu;
		fu.readNode(subfs, true);
		set<int> *nodeSet = &fu.isoNodes;
		for (auto n : *nodeSet)
			nodeFileMap[n] = cur;
		subfs.close();
	}
}

void Optimizer::allocateEdges() {
	map<fileNo, set<int>> isoNodes;
	map<fileNo, vector<edge>> writingFileMap;
	set<int> startNodes;
	for (auto f = files.begin(); f != files.end(); ++f) {
		fstream ifs(*f, fstream::in);
		fileNo curFile = parseFileInt(*f);

		while (!ifs.eof()) {
			string rl;
			getline(ifs, rl);
			if (rl == "") break;
			stringstream rs(rl);

			if (find(rl.begin(), rl.end(), DILIMETER) == rl.end()) {
				node n;
				rs >> n;
				isoNodes[curFile].insert(n.data);
			} else {
				edge e;
				rs >> e;
				auto sloc = nodeFileMap.find(e.start);
				auto eloc = nodeFileMap.find(e.end);
				auto tail = nodeFileMap.end();

				isoNodes[curFile].erase(e.start);
				// end 不擦了 可能会存在叶子节点！存在彼处

				if (sloc->second != eloc->second) {
					e.targetFile = SUFFIX + to_string(eloc->second);
					e.targetNode = e.end;
					e.end = -1;
				} 		// 虚边
				if (writingFileMap.find(sloc->second) == writingFileMap.end())
					writingFileMap[sloc->second] = vector<edge>({ e });
				else writingFileMap[sloc->second].push_back(e);
			}

		}
		ifs.close();
	}

	// 输出边文件
	for (auto fn : writingFileMap) {
		fstream ofs(getFileString(fn.first), fstream::out);
		for (auto e : fn.second)
			ofs << e;
		ofs.close();
	}

	// 输出独立节点
	for (auto in : isoNodes) {
		fstream ofs(getFileString(in.first), fstream::app);
		for (auto n : in.second)
			ofs << node(n);
		ofs.close();
	}

}

void Optimizer::Optimize() {
	getNodesAllocation();
	allocateEdges();
}

Optimizer::~Optimizer() = default;


Checker::Checker(string _subDir, string _filter) {
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

bool CompareMap(const map<int, vector<GraphCommon::edge>>& map1, const map<int, vector<GraphCommon::edge>>& map2) {
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

bool operator==(Checker const& l, Checker const& r) {
	if (l.isoNodes == r.isoNodes &&
		CompareMap(l.adjListGraph, r.adjListGraph)
		)
		return true;
	return false;
}

Checker::~Checker() = default;

