#pragma once

#include "BasicEntity.h"


class CGeneralData :
	public CBasicEntity
{
//protected:

//	int ID;  //data���
//	CCoordinate location;  //δʹ��
//	int time;  //��data���һ��״̬���µ�ʱ���������У�飬��ʼֵӦ����timeBirth
//	bool flag;


protected:

	int node;  //����node
	int timeBirth;  //����ʱ��
	int size;  //byte
	int HOP;

	virtual void init();


public:

	CGeneralData();
	virtual ~CGeneralData();

	//setters & getters
//	inline void setNode(int node)
//	{
//		this->node = node;
//	}
	inline int getNode() const
	{
		return node;
	}
	inline int getTimeBirth() const
	{
		return timeBirth;
	}
	inline int getSize() const
	{
		return size;
	}

	//�����ݱ�ת�������µĽڵ��Ӧ�õ��õĺ�����������������TTLʣ��ֵ��������ʱ���
	//ע�⣺���ݷ��ͷ�Ӧ�ڷ���֮ǰ���ʣ��HOP����1
	virtual inline void arriveAnotherNode(int currentTime)
	{
		if( HOP > 0 )
			this->HOP--;
	}

	//	inline bool allowForward() const
//	{
//		return HOP > 0;
//	}

};
