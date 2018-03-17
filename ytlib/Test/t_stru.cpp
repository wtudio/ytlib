#include "t_stru.h"
#include <iostream>

namespace ytlib
{
	bool test_heap() {

		int data[10] = { 7,9,2,6,4,8,1,3,10,5 };
		Heap<int> h1(data, 10);
		for (int ii = 0; ii < 10; ++ii) {
			std::cout << h1.container[ii] << " ";
		}
		std::cout << std::endl;

		h1.sort();
		for (int ii = 0; ii < 10; ++ii) {
			std::cout << h1.container[ii] << " ";
		}
		std::cout << std::endl;


		int data2[15] = { 7,-5,9,-7,2,6,-6,4,8,1,3,-8,10,5, -9};
		h1.assign(data2, 15);
		h1.sort();

		for (int ii = 0; ii < 15; ++ii) {
			std::cout << h1.container[ii] << " ";
		}
		std::cout << std::endl;

		h1.push(-3);
		h1.push(-10);
		h1.push(11);
		for (int ii = 0; ii < 18; ++ii) {
			std::cout << h1.container[ii] << " ";
		}

		h1.pop();
		h1.pop();
		h1.pop();
		for (int ii = 0; ii < 15; ++ii) {
			std::cout << h1.container[ii] << " ";
		}
		std::cout << std::endl;



		return true;
	}

	bool test_bintree() {
		typedef BinTreeNode<int> mybt;
		typedef std::shared_ptr<mybt> mybtPtr;
		mybtPtr rootp = std::make_shared<mybt>(10);
		rootp->setLChild(std::make_shared<mybt>(5));
		rootp->setRChild(std::make_shared<mybt>(3));

		mybtPtr p = rootp->pl;
		p->setLChild(std::make_shared<mybt>(1));
		p->setRChild(std::make_shared<mybt>(7));

		p = p->pr;
		p->setRChild(std::make_shared<mybt>(4));

		p = rootp->pr;
		p->setRChild(std::make_shared<mybt>(8));
		p = p->pr;
		p->setLChild(std::make_shared<mybt>(9));

		std::cout << rootp->getHeight() << std::endl;
		std::cout << p->getDepth() << std::endl;

		//前序遍历
		std::vector<mybtPtr> vec;
		rootp->DLR(vec);
		for (size_t ii = 0; ii < vec.size(); ++ii) {
			std::cout << vec[ii]->obj << " ";
		}
		std::cout << std::endl;

		//中序遍历
		vec.clear();
		rootp->LDR(vec);
		for (size_t ii = 0; ii < vec.size(); ++ii) {
			std::cout << vec[ii]->obj << " ";
		}
		std::cout << std::endl;

		//后续遍历
		vec.clear();
		rootp->LRD(vec);
		for (size_t ii = 0; ii < vec.size(); ++ii) {
			std::cout << vec[ii]->obj << " ";
		}
		std::cout << std::endl;



		return true;
	}


}