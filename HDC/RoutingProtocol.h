/***********************************************************************************************************************************

基类 CRoutingProtocol ： （继承自 CProcess > CAlgorithm > CProtocol ）所有路由协议算法的实现类，都应该继承自这个类

***********************************************************************************************************************************/

#pragma once

#include "Protocol.h"


class CRoutingProtocol :
	public CProtocol
{
protected:

	static int SLOT_DATA_SEND;  //数据发送slot

	//在限定范围内随机增删一定数量的node
	static void ChangeNodeNumber(int currentTime);
	//更新所有node的坐标、占空比和工作状态
	static void UpdateNodeStatus(int currentTime);


public:

	static bool TEST_DYNAMIC_NUM_NODE;
	static bool TEST_HOTSPOT_SIMILARITY;
	static int SLOT_CHANGE_NUM_NODE;  //动态节点个数测试时，节点个数发生变化的周期

CRoutingProtocol(){};
	~CRoutingProtocol(){};

	//打印相关信息到文件
	static void PrintInfo(int currentTime);
	static void PrintFinal(int currentTime);

};

