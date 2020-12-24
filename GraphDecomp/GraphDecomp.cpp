#include "GraphDecomp.h"

string DECOMPFIL = "A";
string OPTFIL = "O";
char R_PREFIX = 'P';
char R_DILIMETER = ' ';
char DILIMETER = ',';
int RESNODE = -2;

double ev = 0;
double aw = 0;

GraphDecomp::GraphDecomp(int _n, string _mainDir, string _subDir) :
	n(_n), mainDir(_mainDir), subDir(_subDir)
{
	//ResetSubFolder();
}

void GraphDecomp::ResetSubFolder() {
	string command_rd = "rd /s /q \"" + subDir + '\"';
	system(command_rd.c_str());
	string command_md = "md \"" + subDir + '\"';
	system(command_md.c_str());
}

string GraphDecomp::Decomp(DecompSol sol) {
	fstream fs(mainDir, fstream::in);
	if (!fs) error("Cannot open main graph file!");
	
	Decomposer decomp(n, fs, subDir, sol);
	fs.close();

	ev = decomp.Evaluate();
	aw = decomp.GetAllWeights();
	
	return decomp.OuputPartitions();
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

pair<bool, string> GraphDecomp::Evaluate(bool _raw) {
	Evaluator ev(mainDir, subDir, _raw);
	ev.EvaluateWeights();
	return make_pair(ev.Check(), ev.OuputPartitions());
}

string GraphDecomp::ReachablePoints(int node) {
	Finder nfd(subDir);
	return nfd.ReachableNodes(node);
}

string GraphDecomp::ShortestPath(int start, int end) {
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
			strstr(fileInfo.name, nameFilter.c_str()) != NULL)			// ��Ӳ����ļ��Ĺ�����
		{
			file.push_back(fileName);
		}
	} while (_findnext(findResult, &fileInfo) == 0);

	_findclose(findResult);
	return 0;
}

string Processor::parseFileName(string filePath) {
	auto beg = filePath.find_last_of('\\');
	return filePath.substr(beg + 1, filePath.find_last_of('.') - beg - 1);
}

int parseInt(string str) {
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

	// ��ȡ�ڵ����
	isoNodes.clear();
	adjListGraph.clear();
	readRawFile(fs);

	Kerninghan_Lin();
}

void ValueProcessor::initialAdjMat() {
	adjMat.clear();
	for (auto n : adjListGraph) {
		nodeStruct ns(n.first);
		for (auto e : adjListGraph[n.first])
			ns.inputEdge(e);
		adjMat[n.first] = ns;
	}
}

void ValueProcessor::initialCostMat() {
	costMat.clear();
	for (auto i : adjMat)
		for (auto j : adjMat)
			if (i.first != j.first)
				costMat[i.first][j.first] =
				costMat[j.first][i.first] =
				adjMat[i.first].adjMatCol[j.first] + adjMat[j.first].adjMatCol[i.first];
			else
				costMat[j.first][i.first] = adjMat[i.first].adjMatCol[j.first];			// ���ֻ��һ��
}

void Decomposer::divide(set<int> S) {
	// ��ʼʱֱ�Ӱ���Ž���ͨ�ڵ��Ϊ����
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
	// �������ڲ� D
	for (auto a : A) {
		// ��������
		double E = 0;
		for (auto b : B)
			E += costMat[a][b];
		// �����ڲ��
		double I = 0;
		for (auto ap : A)
			I += costMat[a][ap];
		diffCol[a] = E - I;
	}

	for (auto b : B) {
		double E = 0;
		for (auto a : A)
			E += costMat[b][a];
		double I = 0;
		for (auto bp : B)
			I += costMat[b][bp];
		diffCol[b] = E - I;
	}
}

int Decomposer::getMaxDinSet(set<int>& S) {
	int zmax = *S.begin();
	double Dzmax = diffCol[zmax];
	auto z = S.begin();
	++z;
	for (; z != S.end(); ++z)
		if (diffCol[*z] > Dzmax) {
			zmax = *z;
			Dzmax = diffCol[*z];
		}
	return zmax;
}

pair<int, int> Decomposer::getMaxGainPair(set<int>& A, set<int>& B) {
	pair<int, int> selecPair;
	double gainmax = - INFINITY;
	for (auto a : A) {
		for (auto b : B) {
			double gaintmp = diffCol[a] + diffCol[b] - 2 * costMat[a][b];
			if (gaintmp > gainmax) {
				selecPair = make_pair(a, b);
				gainmax = gaintmp;
			}
		}
	}
	return selecPair;
}

void Decomposer::optimizeParts(set<int>& A, set<int>& B) {
	if (sol != rough) {
		double G = INF;
		do {
			calcDiffMat(A, B);
			set<int> Ap = A;
			set<int> Bp = B;
			vector<int> ak;
			vector<int> bk;
			vector<double> gain;
			int size = B.size();
			if (sol == onepass) size *= 0.25;
			while (--size) {
				int amax;
				int bmax;
				double gainLocal;
				if (sol == onepass || sol == ll) {
					amax = getMaxDinSet(Ap);
					bmax = getMaxDinSet(Bp);
				}
				else if (sol == kl) {
					pair<int, int> selecPair = getMaxGainPair(Ap, Bp);
					amax = selecPair.first;
					bmax = selecPair.second;
				}
				gainLocal = diffCol[amax] + diffCol[bmax] - 2 * costMat[amax][bmax];
				ak.push_back(amax);
				bk.push_back(bmax);
				Ap.erase(amax);
				Ap.insert(bmax);
				Bp.erase(bmax);
				Bp.insert(amax);
				gain.push_back(gainLocal);
				calcDiffMat(Ap, Bp);
			}
			int k = 0;
			double G = gain[0];
			double Gtmp = gain[0];
			for (int i = 1; i < gain.size(); ++i) {
				if (Gtmp + gain[i] > G) {
					k = i;
					G = Gtmp + gain[i];
				}
				Gtmp += gain[i];
			}
			for (int l = 0; l <= k; ++l) {
				A.erase(ak[l]);
				B.insert(ak[l]);
				B.erase(bk[l]);
				A.insert(bk[l]);
			}
			if (sol == onepass)
				break;
		} while (G > 0);
	}
}

int Decomposer::getMaxConnWeightNode() {
	int maxNode = RESNODE;
	double maxWeight = RESNODE;
	for (auto n : connNodes)
		if (adjMat[n].totalWeight > maxWeight) {
			maxNode = n;
			maxWeight = adjMat[n].totalWeight;
		}
	return maxNode;
}

void Decomposer::insertPartTmp(int input) {
	if (connNodes.find(input) != connNodes.end()) {
		connNodes.erase(input);
		partTmp.insert(input);
		if (partTmp.size() == n) {
			partitions.push(partTmp);
			partTmp.clear();
		}
	}
}

void Decomposer::BFS() {

	queue<int> visitQueue;

	while (!connNodes.empty()) {
		visitQueue.push(getMaxConnWeightNode());
		while (!visitQueue.empty()) {
			int tn = visitQueue.front();
			visitQueue.pop();
			insertPartTmp(tn);

			int nn;
			while ((nn = adjMat[tn].getMaxLinkedNode()) != RESNODE) {
				insertPartTmp(nn);
				if (connNodes.find(nn) != connNodes.end())
					visitQueue.push(nn);
				adjMat[tn].removeN2N(nn);
			}
		}
	}

	if (!partTmp.empty())
		partitions.push(partTmp);

	allocateIsoNodes();
	outputSubAdjGraphs();
}

void Decomposer::Kerninghan_Lin() {

	// ��ʼ���ڽӾ���
	initialAdjMat();

	// ��ʼ����ʧ����
 	initialCostMat();

	// ������ͨ�ڵ�
	for (auto n : adjListGraph)
		connNodes.insert(n.first);

	if (sol == bfs) { BFS();  return; }
	
	// ÿһ�����Ǿֲ�����
	// ���տ�����ȫ������
	partitions.push(connNodes);
	int m_size = connNodes.size();	// ��һ�����и��С
	while (m_size > n) {
		queue<set<int>> partitionSubq;
		while (!partitions.empty()) {
			partitionSubq.push(partitions.front());
			partitions.pop();
		}
		while (!partitionSubq.empty()) {
			set<int> partition = partitionSubq.front();
			partitionSubq.pop();
			divide(partition);		// �������������
			m_size = partition.size() * SPLIT_RATIO;
		}
	}	

	// �����ڵ�ֱ��������������
	allocateIsoNodes();

	// ������������Ϊ��֤�ļ�
	// OuputPartitions();

	// ��ͨ��ͼ���
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

string ValueProcessor::OuputPartitions() const {
	stringstream ss;
	queue<set<int>> outputPartq = partitions;
	fstream partfs(subDir + "\\partitions.txt", fstream::out);
	while (!outputPartq.empty()) {
		set<int> s = outputPartq.front();
		outputPartq.pop();
		for (auto n : s) {
			partfs << n << ' ';
			ss << n << ' ';
		}
		partfs << endl;
		ss << endl;
	}
	partfs.close();
	return ss.str();
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

double ValueProcessor::Evaluate(){
	// ֻ��Ҫ����cost
	// ÿ�����Ӽ����бȽ�
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
					loss += costMat[ik][jk];
	}

	return loss;
}

double ValueProcessor::GetAllWeights() {
	double total = 0;

	// ���нڵ� = �����ڵ㣨������Ȩ�أ� + ��ͨ�ڵ�
	set<int> connNodes;
	for (auto n : adjListGraph)
		connNodes.insert(n.first);

	// ��Ϊʹ����ʧ�������
	for (auto i = connNodes.begin(); i != connNodes.end(); ++i) {
		auto j = i;
		for (; j != connNodes.end(); ++j)
			total += costMat[*i][*j];
	}
	
	return total;
}

Decomposer::~Decomposer() = default;

Optimizer::Optimizer(int _n, string _subDir) {
	n = _n;
	subDir = _subDir;
	SUFFIX = OPTFIL;

	// �����ļ�
	getFiles(_subDir, ".txt", decomp_files, DECOMPFIL);
}

void Optimizer::getNodesAllocation() {
	for (auto f = decomp_files.begin(); f != decomp_files.end(); ++f) {
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
	for (auto f = decomp_files.begin(); f != decomp_files.end(); ++f) {
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
				// end ������ ���ܻ����Ҷ�ӽڵ㣡���ڱ˴�

				if (sloc->second != eloc->second) {
					e.targetFile = SUFFIX + to_string(eloc->second);
					e.targetNode = e.end;
					e.end = -1;
				} 		// ���
				if (writingFileMap.find(sloc->second) == writingFileMap.end())
					writingFileMap[sloc->second] = vector<edge>({ e });
				else writingFileMap[sloc->second].push_back(e);
			}

		}
		ifs.close();
	}

	// ������ļ�
	for (auto fn : writingFileMap) {
		fstream ofs(getFileString(fn.first), fstream::out);
		for (auto e : fn.second)
			ofs << e;
		ofs.close();
	}

	// ��������ڵ�
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


Checker::Checker(string _subDir, string _filter, bool _raw) {
	if (_filter == "") {			// ���û�й�������һ�����ⲿ�ļ�
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
			if (_raw)
				readRawFile(cf);
			else
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
			auto it2 = *it2t;			// ��ҪתΪ�ֲ��������бȽ�
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

Evaluator::Evaluator(string _mainDir, string _subDir, bool _raw):
	mainDir(_mainDir), raw(_raw)
{
	subDir = _subDir;
}

void Evaluator::getPartition() {
	map<fileNo, set<int>> fileNodeMap;

	map<int, fileNo> beginNodes;
	map<int, fileNo> leafNodes;
	map<int, fileNo> lockedNodes;		// �������ķ���

	for (auto f = decomp_files.begin(); f != decomp_files.end(); ++f) {
		fstream fs(*f, fstream::in);
		fileNo curFile = parseFileInt(*f);

		while (!fs.eof()) {
			string rl;
			getline(fs, rl);
			if (rl == "") break;
			stringstream rs(rl);
			if (find(rl.begin(), rl.end(), DILIMETER) == rl.end()) {
				// ������Խڵ���ʽ�洢�ģ��Ͱ��ն�Ӧ�ļ�����
				node n;
				rs >> n;
				lockedNodes[n.data] = curFile;
			}
			else {
				edge e;
				rs >> e;
				if (beginNodes.find(e.start) == beginNodes.end())
					beginNodes[e.start] = curFile; // ���Ϊ��һ�γ���
				leafNodes[e.end] = curFile;		// ������ֵĻᱻǰ�渲��
			}
		}

		fs.close();
	}

	// ��ʼ���洢��Ϊ�������ϵĲ�������ʼ�ڵ�ȫ�����룬Ҷ��Ϊʣ�������
	set<int> intersect;
	for (auto b : beginNodes) {
		if (leafNodes.find(b.first) != leafNodes.end())
			intersect.insert(b.first);
		if (lockedNodes.find(b.first) == lockedNodes.end())
			fileNodeMap[b.second].insert(b.first);
	}
	for (auto l : leafNodes)
		if (intersect.find(l.first) == intersect.end()
			&& lockedNodes.find(l.first) == lockedNodes.end())
			fileNodeMap[l.second].insert(l.first);
	for (auto n : lockedNodes)
		fileNodeMap[n.second].insert(n.first);

	// ���������
	for (auto fn = fileNodeMap.begin(); fn != fileNodeMap.end(); ++fn)
		partitions.push(fn->second);

}

bool Evaluator::Check() {
	Checker orignalChecker(mainDir);
	Checker decompChecker(subDir, DECOMPFIL, raw);
	return orignalChecker == decompChecker;
}

void Evaluator::EvaluateWeights() {
	getFiles(subDir, ".txt", decomp_files, DECOMPFIL);

	for (auto f = decomp_files.begin(); f != decomp_files.end(); ++f) {
		fstream rfs(*f, fstream::in);
		if (raw)
			readRawFile(rfs);
		else
			readFile(rfs);
		rfs.close();
	}

	// ��Ҫ�õ�������Ϣ
	getPartition();

	initialAdjMat();
	initialCostMat();

	ev = Evaluate();
	aw = GetAllWeights();

}

Evaluator::~Evaluator() = default;

Finder::Finder(string _subDir) {
	subDir = _subDir;
	SUFFIX = OPTFIL;
	getFiles(subDir, ".txt", opt_files, SUFFIX);
}

fileNo Finder::findStoredFile(int node) {
	for (auto f = opt_files.begin(); f != opt_files.end(); ++f) {
		fileNo cur = parseFileInt(*f);
		if (subGraphs.find(cur) == subGraphs.end())
			loadSubgraph(cur);			// ��ͼ��δ������
		
		for (auto n : subGraphs[cur].adjListGraph)
			if (n.first == node)
				return cur;				// ����������ͨ�ߴ洢
		for (auto n : subGraphs[cur].isoNodes)
			if (n == node)
				return -cur;			// ���������ǹ����ڵ�洢
	}
	return 0;							// û�б��洢
}

void Finder::loadSubgraph(fileNo fn) {
	fstream rf(getFileString(fn), fstream::in);
	FileUnit fu;
	fu.readFile(rf, false);				// �������
	subGraphs[fn] = fu;
}

string Finder::prtReachableNodes() const {
	// ��ӡ�ɴ�ڵ�
	stringstream ss;
	for (auto i = reachableNodes.begin(); i != reachableNodes.end(); ++i)
		ss << *i << ' ';
	ss << endl;
	return ss.str();
}

string Finder::ReachableNodes(int beg) {
	// BFS Ѱ�ҿɴ�ڵ�
	queue<pair<fileNo, queue<int>>> visitFileQ;		// �ļ����ʶ���
	fileNo init = findStoredFile(beg);
	if (init <= 0) {
		if (init == 0)								// û�б��洢 
			return "Node " + to_string(beg) + " is not stored!";
		return "";										// �����ǹ����ڵ�
	}
	visitFileQ.push(make_pair(init, queue<int>({ beg })));
	bool flag = true;								// ����ʼ����Ϊ�ɴ�ڵ�

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
				if (e.end != -1) { // ʵ��
					if (subGraphs[fq.first].nodeVisited.find(e.end) == subGraphs[fq.first].nodeVisited.end())
						subVisitq.push(e.end);
				}
				else { // ���
					if (subGraphs.find(tarNo) == subGraphs.end() ||
						(subGraphs[tarNo].nodeVisited.find(e.targetNode) == subGraphs[tarNo].nodeVisited.end()))
						visitFileMap[tarNo].push(e.targetNode);

				}
			}

			// �ýڵ�������
			subGraphs[fq.first].nodeVisited.insert(tn);
		}

		// �����ļ��������
		for (auto m : visitFileMap)
			visitFileQ.push(m);
	}

	return "Num of Nodes:" + to_string(reachableNodes.size()) + '\n' + prtReachableNodes();
}

void Finder::prtPath(int cur, int target, int finish) {
	if (cur != target) prtPath(prev[cur], target, finish);
	pathss << cur << (cur == finish ? "\n" : "->");
}

string Finder::ShortestPath(int start, int end) {
	// dijkstra
	fileNo fin = findStoredFile(end);

	if (fin == 0){
		error("Node " + to_string(end) + " is not stored!");
		return "INF";
	}

	distance[start] = 0;
	prev[start] = start;

	// �ֲ�ʽ SPFA �㷨
	// Shortest Path Faster Alogrithm
	// SPFA ����ʽ�Ϻ�BFS�ǳ����ƣ���ͬ����BFS��һ������˶��оͲ��������½�����У�����SPFA��һ��������ڳ�����֮���ٴα�������У�Ҳ����һ����Ľ��������ĵ�֮�󣬹���һ��ʱ����ܱ����Ľ��������ٴ������Ľ������ĵ㣬��������������ȥ��

	queue<pair<fileNo, queue<int>>> visitFileQ;		// �ļ����ʶ���
	fileNo init = findStoredFile(start);
	if (init <= 0) {
		if (init == 0)								// û�б��洢 
			return "Node " + to_string(start) + " is not stored!";
		return "INF";									// �����ǹ����ڵ�
	}
	if (findStoredFile(end) == 0)
		return "Node " + to_string(end) + " is not stored!"; // û�б��洢 

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

		// �����ļ��������
		for (auto m : visitFileMap)
			visitFileQ.push(m);
	}

	if (distance.find(end) == distance.end()) {
		return "Two nodes are not connected!";
	}

	pathss.clear();
	prtPath(end, start, end);

	return "Length:" + to_string(distance[end]) + '\n' + pathss.str();
}

Finder::~Finder() = default;