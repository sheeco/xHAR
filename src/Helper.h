/***********************************************************************************************************************************

次级类 CHelper ： （继承自顶级类 CProcess ）所有由一些零散的辅助函数包装成的类（一般来说没有成员属性），应该继承自这个类

***********************************************************************************************************************************/

#pragma once

#ifndef __HELPER_H__
#define __HELPER_H__

#include "Process.h"


class CHelper : 
	virtual public CProcess
{
public:

	CHelper(){};
	virtual ~CHelper() = 0
	{};

};

#endif // __HELPER_H__
