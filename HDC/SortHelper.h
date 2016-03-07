#pragma once

#include "Hotspot.h"
#include "Helper.h"
#include "Node.h"


//一些预处理函数包装成的类
class CSortHelper :
	public CHelper
{
private:

	CSortHelper(){};
	~CSortHelper(){};


public:

	/***************************************** 用于归并排序的函数 *****************************************/

	//template <class E>
	//static vector<E> merge(vector<E> left, vector<E> right, bool(*Comp)(E, E));
	//template <class E>
	//static vector<E> mergeSort(vector<E> v, bool(*Comp)(E, E));

	//CData按照时间排序
	static vector<CData> merge(vector<CData> left, vector<CData> right, bool(*Comp)(CData, CData));
	static vector<CData> mergeSort(vector<CData> v, bool(*Comp)(CData, CData));

	//CPosition类按照x坐标排序
	static vector<CPosition *> merge(vector<CPosition *> left, vector<CPosition *> right);
	static vector<CPosition *> mergeSort(vector<CPosition *> v);

	//CNode类按照x坐标排序
	static vector<CNode *> merge(vector<CNode *> left, vector<CNode *> right);
	static vector<CNode *> mergeSort(vector<CNode *> v);
	//CNode类按照ID排序
	static vector<CNode *> merge(vector<CNode *> left, vector<CNode *> right, bool(*Comp)(CNode *, CNode *));
	static vector<CNode *> mergeSort(vector<CNode *> v, bool(*Comp)(CNode *, CNode *));

	//CHotspot类按照x坐标或者ratio排序
	static vector<CHotspot *> merge(vector<CHotspot *> left, vector<CHotspot *> right, bool(*Comp)(CHotspot *, CHotspot *));
	static vector<CHotspot *> mergeSort(vector<CHotspot *> v, bool(*Comp)(CHotspot *, CHotspot *));
	//CHotspot类静态拷贝按照( endTime - 900, endTime )期间的投递技术的降序排列
	static vector<CHotspot> mergeByDeliveryCount(vector<CHotspot> left, vector<CHotspot> right, int endTime);
	static vector<CHotspot> mergeSortByDeliveryCount(vector<CHotspot> v, int endTime);

	//用于作为参数传入mergeSort函数的Comparison函数
	static bool ascendByLocationX(CHotspot *left, CHotspot *right);
	static bool ascendByRatio(CHotspot *left, CHotspot *right);
	static bool descendByInt(int left, int right){	return left > right;	};
	static bool ascendByInt(int left, int right){	return left < right;	};
	static bool ascendByData(CData left, CData right){	return left < right;	};
	static bool ascendByID(CNode *left, CNode *right){	return left->getID() < right->getID();	};

};

