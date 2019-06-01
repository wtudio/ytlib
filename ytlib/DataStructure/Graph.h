#pragma once
#include <ytlib/Common/Util.h>
#include <ytlib/LightMath/Matrix.h>
#include <map>
#include <list>
#include <vector>

//todo：待完善
//模板化的图的一些工具。只是作为样板学习之用，实际工程中需要时根据需求参考源码进行改动
//不能使用智能指针，因为会有相互指引的情况
//使用时应将所有的节点的指针放入一个list或vector中
//每个节点指向另一个节点最多只有一条边
namespace ytlib {
	//一个有边的权重的有向图的节点
	typedef double g_sideType;
	typedef Basic_Matrix<g_sideType> g_sideMatrix;

	template<typename T>
	class Graph {
	public:
		
		Graph():visited(false){}
		~Graph() {}
		Graph(const T& _obj) :obj(_obj), visited(false) {}

		T obj; 
		std::map<Graph<T>*, g_sideType> sides;//边以及其权重。权重默认为double
		mutable bool visited;//用于遍历
	};
	//一些对图的基本操作

	//有向图的节点插入，参数为：待插入节点、目标节点、边权值
	template<typename T>
	void insertGraphNode(Graph<T>& val,Graph<T>& targt, g_sideType side) {
		val.sides[&targt] = side;
	} 

	//无向图的节点插入
	template<typename T>
	void connectGraphNode(Graph<T>& val, Graph<T>& targt, g_sideType side) {
		val.sides[&targt] = side;
		targt.sides[&val] = side;
	}

	//判断是否是无向图
	template<typename T>
	bool isUndiGraph(const std::vector<Graph<T>*>& vec) {
		for (uint32_t ii = 0; ii < vec.size(); ++ii) {
			for (auto itr = vec[ii]->sides.begin(); itr != vec[ii]->sides.end(); ++itr) {
				auto itr2 = itr->first->sides.find(vec[ii]);
				if (itr2 == itr->first->sides.end()) return false;
				if (itr2->second != itr->second) return false;
			}
		}
		return true;
	}

	//一些图的基本算法

	//辅助函数，获取节点在vector中的下标
	template<typename T>
	inline size_t getPos(const Graph<T>* p, const std::vector<Graph<T>*>& vec) {
		size_t pos = find(vec.begin(), vec.end(), p) - vec.begin();
		assert(pos < vec.size());
		return pos;
	}

	//辅助函数，清除标志位
	template<typename T>
	inline void clearFlag(const std::vector<Graph<T>*>& vec) {
		for (uint32_t ii = 0; ii < vec.size(); ++ii) vec[ii]->visited = false;
	}
	//辅助函数，释放内存
	template<typename T>
	inline void releaseGraphVec(std::vector<Graph<T>*>& vec) {
		for (uint32_t ii = 0; ii < vec.size(); ++ii) delete vec[ii];
	}
	
	//创建邻接矩阵，M.val[i][j]表示从顶点vec[i]出发到顶点vec[j]的直接距离，-1值表示不直接连接
	template<typename T>
	g_sideMatrix createAdjMatrix(const std::vector<Graph<T>*>& vec) {
		size_t Vnum = vec.size();
		g_sideMatrix M((int32_t)Vnum, (int32_t)Vnum);
		M.setVal(-1);//-1表示不连接
		for (size_t ii = 0; ii < Vnum; ++ii) {
			M.val[ii][ii] = 0;
			for (auto itr = vec[ii]->sides.begin(); itr != vec[ii]->sides.end(); ++itr) {
				size_t pos = getPos(itr->first, vec);
				M.val[ii][pos] = itr->second;
			}
		}
		return M;
	}

	//图的复制
	template<typename T>
	std::vector<Graph<T>*> copyGraph(const std::vector<Graph<T>*>& vec) {
		std::vector<Graph<T>*> re;
		size_t len = vec.size();
		for (size_t ii = 0; ii < len; ++ii) {
			re.push_back(new Graph<T>(vec[ii]->obj));
		}
		for (size_t ii = 0; ii < len; ++ii) {
			for (auto itr = vec[ii]->sides.begin(); itr != vec[ii]->sides.end(); ++itr) {
				size_t pos = getPos(itr->first, vec);
				re[ii]->sides.insert(std::pair<Graph<T>*, g_sideType>(re[pos], itr->second));
			}
		}
		return re;
	}


	//DFS。遍历之前应确定所有节点的visited已经被重置为false
	template<typename T>
	void DFS(Graph<T>& val, std::vector<Graph<T>*>& vec) {
		vec.push_back(&val);//由上层保证此节点没有遍历过
		val.visited = true;
		for (auto itr = val.sides.begin(); itr != val.sides.end(); ++itr) {
			if (!(itr->first->visited)) DFS(*itr->first, vec);
		}
	}

	//BFS。遍历之前应确定所有节点的visited已经被重置为false
	template<typename T>
	void BFS(Graph<T>& val, std::vector<Graph<T>*>& vec) {
		if (!val.visited) {
			//初始节点
			vec.push_back(&val);
			val.visited = true;
		}
		std::vector<Graph<T>*> tmpvec;
		for (auto itr = val.sides.begin(); itr != val.sides.end(); ++itr) {
			if (!(itr->first->visited)) {
				itr->first->visited = true;
				vec.push_back(itr->first);
				tmpvec.push_back(itr->first);
			}
		}
		size_t len = tmpvec.size();
		for (size_t ii = 0; ii < len; ++ii) {
			BFS(*tmpvec[ii], vec);
		}

	}

	

	//dijkstra算法：求一个节点到其他节点的最短路径，返回距离数组和路径数组。禁止负权边
	template<typename T>
	std::pair<std::vector<g_sideType>, std::vector<int32_t> > dijkstra(const Graph<T>& beginNode,const std::vector<Graph<T>*>& vec) {
		size_t len = vec.size();
		std::vector<g_sideType> re(len, -1);
		std::vector<uint8_t> flag(len, 0);
		std::vector<int32_t> path(len, -1);

		size_t curPos = getPos(&beginNode, vec), nextPos = curPos;
		re[curPos] = 0;
		path[curPos] = (int32_t)curPos;
		do {
			curPos = nextPos;
			flag[curPos] = 1;
			for (auto itr = vec[curPos]->sides.begin(); itr != vec[curPos]->sides.end(); ++itr) {
				size_t pos = find(vec.begin(), vec.end(), itr->first) - vec.begin();
				if (flag[pos] == 0) {
					g_sideType sideLen = ((re[curPos] < 0) ? 0 : re[curPos]) + itr->second;
					if (re[pos] < 0 || re[pos]>sideLen) {
						re[pos] = sideLen;
						path[pos] = (int32_t)curPos;
					}
				}
			}
			g_sideType minLen = -1;
			for (size_t ii = 0; ii < len; ++ii) {
				if (flag[ii] == 0 && re[ii]>=0 && (minLen<0 || minLen>re[ii])) {
					minLen = re[ii];
					nextPos = ii;
				}
			}			
		} while (nextPos != curPos);
		return std::pair<std::vector<g_sideType>, std::vector<int32_t> >(std::move(re), std::move(path));
	}
	//根据dijkstra返回的路径数组求特定节点到另一个节点的最短路径。返回的是倒推的路径
	static std::vector<int32_t> dijkstraPath(int32_t dstIdx,const std::vector<int32_t>& path) {
		std::vector<int32_t> re;
		assert(path[dstIdx] >= 0);
		do {
			dstIdx = path[dstIdx];
			re.push_back(dstIdx);
		} while (dstIdx != path[dstIdx]);
		return re;
	}

	//floyd算法：求所有节点到其他所有节点的最短路径，返回距离矩阵和路径矩阵
	template<typename T>
	std::pair<g_sideMatrix, Matrix_i> floyd(const std::vector<Graph<T>*>& vec) {
		size_t len = vec.size();
		g_sideMatrix distanceM = createAdjMatrix(vec);
		Matrix_i pathM((int32_t)len, (int32_t)len);
		pathM.setVal(-1);
		
		for (size_t ii = 0; ii < len; ++ii) {
			for (size_t jj = 0; jj < len; ++jj) {
				if (distanceM.val[ii][jj] >= 0)	pathM.val[ii][jj] = (int32_t)ii;
			}
		}		
		for (size_t kk = 0; kk < len; ++kk) {
			for (size_t ii = 0; ii < len; ++ii) {
				if (ii == kk) continue;
				for (size_t jj = 0; jj < len; ++jj) {
					if (ii == jj || jj == kk) continue;
					if (distanceM.val[ii][kk] >= 0 && distanceM.val[kk][jj] >= 0) {
						g_sideType d = distanceM.val[ii][kk] + distanceM.val[kk][jj];
						if (distanceM.val[ii][jj]<0 || distanceM.val[ii][jj]>d) {
							distanceM.val[ii][jj] = d;
							pathM.val[ii][jj] = (int32_t)kk;
						}
					}
				}
			}
		}
		return std::pair<g_sideMatrix, Matrix_i>(std::move(distanceM), std::move(pathM));
	}

	//根据floyd返回的路径矩阵求一个节点到另一个节点的最短路径。返回的是正推的路径
	static std::vector<int32_t> floydPath(int32_t srcIdx, int32_t dstIdx, const Matrix_i& path) {
		std::vector<int32_t> re;
		assert(path.val[srcIdx][dstIdx]>=0);
		re.push_back(srcIdx);
		do {
			srcIdx = path.val[srcIdx][dstIdx];
			re.push_back(srcIdx);
		} while (srcIdx != path.val[srcIdx][dstIdx]);
		return re;
	}


	//最大流。边权值作为流量值
	template<typename T>
	g_sideType maxFlow(Graph<T>& beginNode, Graph<T>& endNode) {
		g_sideType re = 0;


		return re;
	}

	//Kruskal算法：最小生成树，加边法。将在输入的vector上进行修改，返回根节点的指针
	template<typename T>
	Graph<T>* kruskal(std::vector<Graph<T>*>& vec) {
		
	}


	//Prim算法：最小生成树，加点法。将在输入的vector上进行修改，返回根节点的指针
	template<typename T>
	Graph<T>* prim(std::vector<Graph<T>*>& vec) {
		
	}

	//A*

}


