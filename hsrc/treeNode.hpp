#ifndef TREENODE_HPP_
#define TREENODE_HPP_
#pragma once
#include <vector>
using std::cout;
template <class T> class treeNode {
	public:
		treeNode() { // default construct
		}
		treeNode(const T& t) {
			this->t = t;
		}
		void addChild(treeNode<T>* toAdd) {
			this->children.push_back(toAdd);
		}
		void setValue(const T& t) {
			this->t = t;
		}
		T& getValue() {
			return this->t;
		}
		T getCopy() {
			return this->t;
		}
		std::vector<treeNode*> getChildren() {
			return this->children;
		}
	private:
		T t;
		std::vector<treeNode*> children;
};
#endif