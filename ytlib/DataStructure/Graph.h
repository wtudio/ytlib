#pragma once
#include <ytlib/Common/Util.h>
#include <ytlib/LightMath/Matrix.h>
#include <map>
#include <list>


//模板化的图的一些工具。todo：待完善
//不能使用智能指针，因为会有相互指引的情况
//使用时应将所有的节点放入一个list中
namespace ytlib {

	//一个有边的权重的有向图的节点
	template<typename T, typename sideType = uint32_t>
	class Graph {
	public:
		typedef std::list<Graph<T, sideType> > graphContainer;

		T obj;

		map<Graph<T, sideType>*, sideType> sides;//边以及其权重

	};
	//一些对图的基本操作

	//有向图的节点插入




	//无向图的节点插入



	//生成邻接矩阵





	//一些图的基本算法

	//DFS


	//BFS



	//迪杰斯特拉算法

	//最小生成树 


	//A*

}


