/***********************************************************************************************************************************

基类 CMacProtocol ： （继承自 CProcess > CAlgorithm > CProtocol ）所有MAC层协议算法的实现类，都应该继承自这个类

***********************************************************************************************************************************/

#pragma once

#include "Protocol.h"

using namespace std;


class CMacProtocol :
	public CProtocol
{
public:

	CMacProtocol(){};
	~CMacProtocol(){};

};

