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

	double ev = decomp.Evaluate();
	double aw = decomp.GetAllWeights();

	cout << "评估：" << ev << '/' << aw << '=' << ev / aw << endl;
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
		&& decompChecker == optChecker
		;
}

void GraphDecomp::ReachablePoints(int node) {
	Finder nfd(subDir);
	nfd.ReachableNodes(node);
}

double GraphDecomp::ShortestPath(int start, int end) {
	Finder pfd(subDir);
	return pfd.ShortestPath(start, end);
}

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

double Decomposer::getCostValue(int i, int j) {
	if (costMat.find(i) == costMat.end() ||
		costMat[i].find(j) == costMat[i].end())
		return 0;
	return costMat[i][j];
}

void Decomposer::divide(set<int> S) {
	// 开始时直接按序号将连通节点分为两半
	set<int> A, B;
	set<int>::iterator it = S.begin();
	for(int size = S.size() * SPLIT_RATIO; size > 0; --size){
		A.insert(*it);
		++it;
	}
	for (; it != S.end(); ++it)
		B.insert(*it);
	
	optimizeParts(A, B);

	partitions.push(A);
	partitions.push(B);
}

void Decomposer::calcDiffMat(set<int>& A, set<int>& B) {
	// 计算外内差 D
	for (auto a : A) {
		// 计算外差距
		double E = 0;
		for (auto b : B)
			E += getCostValue(a, b);
		// 计算内差距
		double I = 0;
		for (auto ap : A)
			I += getCostValue(a, ap);
		diffCol[a] = E - I;
	}

	for (auto b : B) {
		double E = 0;
		for (auto a : A)
			E += getCostValue(b, a);
		double I = 0;
		for (auto bp : B)
			I += getCostValue(b, bp);
		diffCol[b] = E - I;
	}
}

pair<int, double> Decomposer::getMaxDinSet(set<int>& S) {
	int zmax = *S.begin();
	double Dzmax = diffCol[zmax];
	auto z = S.begin();
	++z;
	for (; z != S.end(); ++z)
		if (diffCol[*z] > Dzmax) {
			zmax = *z;
			Dzmax = diffCol[*z];
		}
	return make_pair(zmax, Dzmax);
}

void Decomposer::optimizeParts(set<int>& A, set<int>& B) {
	if (sol == ll) {
		calcDiffMat(A, B);
		set<int> Ap = A;
		set<int> Bp = B;
		vector<int> ak;
		vector<int> bk;
		vector<double> gain;
		int size = B.size();
		while (--size) {
			pair<int, double> amax = getMaxDinSet(Ap);
			pair<int, double> bmax = getMaxDinSet(Bp);
			ak.push_back(amax.first);
			bk.push_back(bmax.first);
			Ap.erase(amax.first);
			Ap.insert(bmax.first);
			Bp.erase(bmax.first);
			Bp.insert(amax.first);
			gain.push_back(amax.second + bmax.second - 2 * getCostValue(amax.first, bmax.first));
			calcDiffMat(Ap, Bp);
		}
		int k = 0;
		double gainmax = gain[0];
		for (int i = 1; i < gain.size(); ++i)
			if (gain[i] > gainmax) {
				k = i;
				gainmax = gain[i];
			}
		for (int l = 0; l <= k; ++l) {
			A.erase(ak[l]);
			B.insert(ak[l]);
			B.erase(bk[l]);
			A.insert(bk[l]);
		}
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
			m_size = partition.size() * SPLIT_RATIO;
		}
	}	

	// 孤立节点直接最后输出，分配
	allocateIsoNodes();

	// 输出分配情况作为验证文件
	OuputPartitions();

	// 连通子图输出
	outputSubAdjGraphs();
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

void Decomposer::outputSubAdjGraphs() const {
	queue<set<int>> partTmp = partitions;
	fileNo fileNum = 1;
	while (!partTmp.empty()) {
		set<int> s = partTmp.front();
		partTmp.pop();
		fstream subfs(getFileString(fileNum), fstream::out);
		fileNum++;
		for (auto n : s) {
			subfs << node(n);
			auto adjG = adjListGraph;
			for (auto e : adjG[n])
				subfs << e;
		}
		subfs.close();
	}
}

double Decomposer::Evaluate(){
	// 只需要考虑cost
	// 每两个子集进行比较
	double loss = 0;
	
	queue<set<int>> partTmp = partitions;
	set<set<int>> partitionSet;
	while (!partTmp.empty()) {
		partitionSet.insert(partTmp.front());
		partTmp.pop();
	}

	for (auto i = partitionSet.begin(); i != partitionSet.end(); ++i) {
		auto j = i;
		++j;
		for (; j != partitionSet.end(); ++j)
			for (auto ik : *i)
				for (auto jk : *j)
					loss += getCostValue(ik, jk);
	}

	return loss;
}

double Decomposer::GetAllWeights() {
	double total = 0;

	// 所有节点 = 孤立节点（无连接权重） + 连通节点
	set<int> connNodes;
	for (auto n : adjListGraph)
		connNodes.insert(n.first);

	for (auto i : connNodes)
		for (auto j : connNodes)
			total += adjMat[i].getConnWeight(j);

	return total;
}

Decomposer::~Decomposer() = default;

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
		FileUnit fu;
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

Finder::Finder(string _subDir) {
	subDir = _subDir;
	SUFFIX = OPTFIL;
	getFiles(subDir, ".txt", files, SUFFIX);
}

fileNo Finder::findStoredFile(int node) {
	for (auto f = files.begin(); f != files.end(); ++f) {
		fileNo cur = parseFileInt(*f);
		if (subGraphs.find(cur) == subGraphs.end())
			loadSubgraph(cur);			// 子图尚未被加载
		
		for (auto n : subGraphs[cur].adjListGraph)
			if (n.first == node)
				return cur;				// 正数代表连通边存储
		for (auto n : subGraphs[cur].isoNodes)
			if (n == node)
				return -cur;			// 负数代表是孤立节点存储
	}
	return 0;							// 没有被存储
}

void Finder::loadSubgraph(fileNo fn) {
	fstream rf(getFileString(fn), fstream::in);
	FileUnit fu;
	fu.readFile(rf, false);				// 保留虚边
	subGraphs[fn] = fu;
}

void Finder::prtReachableNodes() const {
	// 打印可达节点
	cout << "可达节点数目：" << reachableNodes.size() << endl;
	for (auto i = reachableNodes.begin(); i != reachableNodes.end(); ++i)
		cout << *i << ' ';
	cout << endl;
}

void Finder::ReachableNodes(int beg) {
	// BFS 寻找可达节点
	queue<pair<fileNo, queue<int>>> visitFileQ;		// 文件访问队列
	fileNo init = findStoredFile(beg);
	if (init <= 0) {
		if (init == 0)								// 没有被存储 
			error("Node " + to_string(beg) + " is not stored!");
		prtReachableNodes();
		return;										// 或者是孤立节点
	}
	visitFileQ.push(make_pair(init, queue<int>({ beg })));
	bool flag = true;								// 自身开始不作为可达节点

	while (!visitFileQ.empty()) {
		auto fq = visitFileQ.front();
		visitFileQ.pop();

		if (subGraphs.find(fq.first) == subGraphs.end())
			loadSubgraph(fq.first);

		map<fileNo, queue<int>> visitFileMap;

		queue<int> subVisitq = fq.second;

		while (!subVisitq.empty()) {
			int tn = subVisitq.front();
			subVisitq.pop();
			if (flag) flag = false;
			else reachableNodes.insert(tn);

			for (auto e : subGraphs[fq.first].adjListGraph[tn]) {
				fileNo tarNo = parseInt(e.targetFile);
				if (e.end != -1) { // 实边
					if (subGraphs[fq.first].nodeVisited.find(e.end) == subGraphs[fq.first].nodeVisited.end())
						subVisitq.push(e.end);
				}
				else { // 虚边
					if (subGraphs.find(tarNo) == subGraphs.end() ||
						(subGraphs[tarNo].nodeVisited.find(e.targetNode) == subGraphs[tarNo].nodeVisited.end()))
						visitFileMap[tarNo].push(e.targetNode);

				}
			}

			// 该节点访问完毕
			subGraphs[fq.first].nodeVisited.insert(tn);
		}

		// 该轮文件访问完毕
		for (auto m : visitFileMap)
			visitFileQ.push(m);
	}

	prtReachableNodes();
}

void Finder::prtPath(int cur, int target, int finish) {
	if (cur != target) prtPath(prev[cur], target, finish);
	cout << cur << (cur == finish ? "\n" : "->");
}

double Finder::ShortestPath(int start, int end) {
	// dijkstra
	fileNo fin = findStoredFile(end);

	if (fin == 0){
		error("Node " + to_string(end) + " is not stored!");
		return INF;
	}

	distance[start] = 0;
	prev[start] = start;

	// 分布式 SPFA 算法
	// Shortest Path Faster Alogrithm
	// SPFA 在形式上和BFS非常类似，不同的是BFS中一个点出了队列就不可能重新进入队列，但是SPFA中一个点可能在出队列之后再次被放入队列，也就是一个点改进过其它的点之后，过了一段时间可能本身被改进，于是再次用来改进其它的点，这样反复迭代下去。

	queue<pair<fileNo, queue<int>>> visitFileQ;		// 文件访问队列
	fileNo init = findStoredFile(start);
	if (init <= 0) {
		if (init == 0)								// 没有被存储 
			error("Node " + to_string(start) + " is not stored!");
		return INF;									// 或者是孤立节点
	}
	if (findStoredFile(end) == 0)
		error("Node " + to_string(end) + " is not stored!"); // 没有被存储 

	visitFileQ.push(make_pair(init, queue<int>({ start })));

	while (!visitFileQ.empty()) {
		auto fq = visitFileQ.front();
		visitFileQ.pop();

		if (subGraphs.find(fq.first) == subGraphs.end())
			loadSubgraph(fq.first);

		map<fileNo, queue<int>> visitFileMap;

		queue<int> subVisitq = fq.second;

		while (!subVisitq.empty()) {
			int tn = subVisitq.front();
			subVisitq.pop();

			for (auto e : subGraphs[fq.first].adjListGraph[tn]) {
				int toNode = RESNODE;
				fileNo tarNo = parseInt(e.targetFile);
				toNode = e.end == -1 ? e.targetNode : e.end;
				if (toNode != RESNODE &&
					(distance.find(toNode) == distance.end() ||
						distance[tn] + e.weight < distance[toNode])) {
					distance[toNode] = distance[tn] + e.weight;
					prev[toNode] = tn;
					if (e.end != -1) subVisitq.push(e.end);
					else visitFileMap[tarNo].push(e.targetNode);
				}
			}
		}

		// 该轮文件访问完毕
		for (auto m : visitFileMap)
			visitFileQ.push(m);
	}

	if (distance.find(end) == distance.end()) {
		error("Two nodes are not connected!");
		return INF;
	}

	prtPath(end, start, end);

	return distance[end];
}

Finder::~Finder() = default;