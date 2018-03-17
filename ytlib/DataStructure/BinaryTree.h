#pragma once
#include <ytlib/Common/Util.h>
#include <memory>
#include <vector>


namespace ytlib {

	//一个BinTreeNode实例表示一个二叉树节点，也表示以此节点为根节点的一棵二叉树
	//如果根节点被析构，那么整个树中所有子节点将被析构，除非子节点有另外的智能指针指着
	template<typename T>
	class BinTreeNode : public std::enable_shared_from_this<BinTreeNode<T> > {
	private:
		typedef std::shared_ptr<BinTreeNode<T> > nodePtr;
	public:
		BinTreeNode(){}
		virtual ~BinTreeNode(){}

		BinTreeNode(const T& _obj) :obj(_obj) {

		}


		T obj;
		BinTreeNode<T>* pf;//父节点。父节点不可使用智能指针，否则会造成循环引用
		nodePtr pl;//左子节点
		nodePtr pr;//右子节点

		//将一个节点作为左/右子节点（与原左/右子节点断开。插入的节点与其原父节点断开）
		void setLChild(nodePtr& pnd) {
			assert(pnd);
			pnd->pf = this;
			pl = pnd;
		}
		void setRChild(nodePtr& pnd) {
			assert(pnd);
			pnd->pf = this;
			pr = pnd;
		}

		//与左/右子树断开
		void breakLChild() {
			pl->pf = NULL;
			pl.reset();
		}
		void breakRChild() {
			pr->pf = NULL;
			pr.reset();
		}


		//判断是父节点的左节点还是右节点。true表示左。使用前应检查父节点是否为空
		bool getLR() {
			assert(pf != NULL);
			if (this == pf->pl.get()) return true;
			if (this == pf->pr.get()) return false;
			assert(0);//不是父节点的左右节点。报错
			return true;
		}

		//获取深度，根节点深度为0
		virtual size_t getDepth() {
			BinTreeNode<T>* tmp = pf;
			size_t count = 0;
			while (tmp!=NULL) {
				++count;
				tmp = tmp->pf;
			}
			return count;
		}

		//获取高度，叶子节点高度为1
		virtual size_t getHeight() {
			size_t lh = 0, rh = 0;
			if (pl) lh = pl->getHeight();
			if (pr) rh = pr->getHeight();

			return max(lh, rh) + 1;

		}

		//以当前节点为根节点，前序遍历，返回一个指针数组。以当前节点为根节点
		void DLR(std::vector<nodePtr>& vec) {
			vec.push_back(shared_from_this());
			if (pl) pl->DLR(vec);
			if (pr) pr->DLR(vec);
		}

		//中序遍历
		void LDR(std::vector<nodePtr>& vec) {
			if (pl) pl->LDR(vec);
			vec.push_back(shared_from_this());
			if (pr) pr->LDR(vec);
		}

		//后续遍历
		void LRD(std::vector<nodePtr>& vec) {
			if (pl) pl->LRD(vec);
			if (pr) pr->LRD(vec);
			vec.push_back(shared_from_this());
		}

	};

	//二叉查找树，T需要支持比较运算
	template<typename T>
	class BinSearchTreeNode : public BinTreeNode<T> {
	private:
		typedef std::shared_ptr<BinSearchTreeNode<T> > BSTNodePtr;
	public:
		BinSearchTreeNode():BinTreeNode<T>(){}
		virtual ~BinSearchTreeNode() {}

		BinSearchTreeNode(const T& _obj) :BinTreeNode<T>(_obj) {

		}

		//向当前节点为根节点的二叉查找树中插入一个节点
		void insert(BSTNodePtr& ndptr) {
			assert(ndptr);
			if (ndptr->obj < obj) {
				if (pl) pl->insert(ndptr);
				else setLChild(ndptr);
			}
			else {
				if (pr) pr->insert(ndptr);
				else setRChild(ndptr);
			}
		}

		//删除当前节点。
		void erase() {
			
			if (!pl && !pr) {
				//左右都为空，为叶子节点
				if (pf == NULL) return;
				if (getLR) pf->breakLChild();//todo:测试这样调用会不会有问题
				else pf->breakRChild();

			}
			else if (pl && !pr) {
				//只有左子树
				if (pf == NULL) breakLChild();
				else {
					pl->pf = pf;

				}

			}
			else if (!pl && pr) {
				//只有右子树


			}
			else {

			}


		}

	};



}


