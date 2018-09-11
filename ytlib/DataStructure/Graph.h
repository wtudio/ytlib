#pragma once
#include <ytlib/Common/Util.h>
#include <ytlib/LightMath/Matrix.h>
#include <map>
#include <list>
#include <vector>

//模板化的图的一些工具。todo：待完善
//不能使用智能指针，因为会有相互指引的情况
//使用时应将所有的节点的智能指针放入一个list或vector中
//每个节点指向另一个节点最多只有一条边
namespace ytlib {

	//一个有边的权重的有向图的节点
	template<typename T, typename sideType = uint32_t>
	class Graph {
	public:
		Graph():visited(false){}
		~Graph() {}
		Graph(const T& _obj):obj(_obj), visited(false) {}

		T obj;
		std::map<Graph<T, sideType>*, sideType> sides;//边以及其权重
		//一些为了遍历所设置的标志位
		bool visited;

	};
	//一些对图的基本操作

	//有向图的节点插入，参数为：待插入节点、目标节点、边权值
	template<typename T, typename sideType = uint32_t>
	void insertGraphNode(Graph<T, sideType>& val,const Graph<T, sideType>& targt, sideType side) {
		val.sides[&targt] = side;
	}

	//无向图的节点插入
	template<typename T, typename sideType = uint32_t>
	void connectGraphNode(Graph<T, sideType>& val, Graph<T, sideType>& targt, sideType side) {
		val.sides[&targt] = side;
		targt.sides[&val] = side;
	}

	//图的复制
	template<typename T, typename sideType = uint32_t>
	std::vector<Graph<T, sideType>*> copyGraph(const std::vector<Graph<T, sideType>*>& vec) {
		std::vector<Graph<T, sideType>*> re;



		return re;
	}


	//创建邻接矩阵
	template<typename T, typename sideType = uint32_t>
	Basic_Matrix<sideType> createAdjMatrix(const std::vector<Graph<T, sideType>*>& vec) {
		Basic_Matrix<sideType> M;

		return std::move(M);
	}

	//一些图的基本算法

	//DFS。遍历之前应确定所有节点的visited已经被重置为false
	template<typename T, typename sideType = uint32_t>
	void DFS(Graph<T, sideType>& val, std::vector<Graph<T, sideType>*>& vec) {
		vec.push_back(&val);
		val.visited = true;
		for (typename std::map<Graph<T, sideType>*, sideType>::iterator itr = val.sides.begin(); itr != val.sides.end(); ++itr) {
			if (itr->first != NULL && !(itr->first->visited)) DFS(*itr->first, vec);
		}
	}

	//BFS。遍历之前应确定所有节点的visited已经被重置为false
	template<typename T, typename sideType = uint32_t>
	void BFS(Graph<T, sideType>& val, std::vector<Graph<T, sideType>*>& vec) {
		if (!val.visited) {
			//初始节点
			vec.push_back(&val);
			val.visited = true;
		}
		std::vector<Graph<T, sideType>*> tmpvec;
		for (typename std::map<Graph<T, sideType>*, sideType>::iterator itr = val.sides.begin(); itr != val.sides.end(); ++itr) {
			if (itr->first != NULL && !(itr->first->visited)) {
				vec.push_back(itr->first);
				itr->first->visited = true;
				tmpvec.push_back(itr->first);
			}
		}
		size_t len = tmpvec.size();
		for (size_t ii = 0; ii < len; ++ii) {
			BFS(*tmpvec[ii], vec);
		}

	}


	//dijkstra算法：求一个节点到其他节点的最短路径
	template<typename T, typename sideType = uint32_t>
	std::vector<sideType> dijkstra(Graph<T, sideType>& beginNode,const std::vector<Graph<T, sideType>*>& vec) {
		std::vector<sideType> re;


		return re;
	}
	//floyd算法：求所有节点到其他所有节点的最短路径
	template<typename T, typename sideType = uint32_t>
	Basic_Matrix<sideType> floyd(const std::vector<Graph<T, sideType>*>& vec) {
		Basic_Matrix<sideType> M;

		return M;
	}


	//最大流
	template<typename T, typename sideType = uint32_t>
	sideType maxFlow(Graph<T, sideType>& beginNode, Graph<T, sideType>& endNode) {
		sideType re = 0;


		return re;
	}

	//Kruskal算法：最小生成树，加边法。将在输入的vector上进行修改，返回根节点的指针
	template<typename T, typename sideType = uint32_t>
	Graph<T, sideType>* kruskal(std::vector<Graph<T, sideType>*>& vec) {
		
	}


	//Prim算法：最小生成树，加点法。将在输入的vector上进行修改，返回根节点的指针
	template<typename T, typename sideType = uint32_t>
	Graph<T, sideType>* prim(std::vector<Graph<T, sideType>*>& vec) {
		
	}

	//A*

}


