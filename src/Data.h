#pragma once

#include "GeneralData.h"


class CData : 
	public CGeneralData
{
//protected:

//	int ID;  //data���
//	CCoordinate location;  //δʹ��
//	int time;  //��data���һ��״̬���µ�ʱ���������У�飬��ʼֵӦ����timeBirth
//	bool flag;
//	int node;  //����node
//	int timeBirth;  //����ʱ��
//	int size;  //byte
//	int HOP;


private:

	int timeArrival;  //����sink��ʱ��
	//������TTL����������Epidemic��ѡ��һ��ʹ��
	int TTL;

//	static int ID_MASK;

	//����ͳ��Ͷ���ʺ�ʱ�ӵľ�̬����
	static int ID_COUNT;  //��ֵ����data������
	static int ARRIVAL_COUNT;  //��������ݼ���
	static double DELAY_SUM;  //ʱ�ӼӺͣ����ڼ���ƽ��ʱ��

	static int DELIVERY_AT_HOTSPOT_COUNT;  //���ȵ㴦�õ�Ͷ�ݵ����ݼ���
	static int DELIVERY_ON_ROUTE_COUNT;  //��·���ϵõ�Ͷ�ݵ����ݼ���

	CData()
	{
		CData::init();
	};

	void init() override
	{
		CGeneralData::init();
		this->timeArrival = -1;
		this->TTL = 0;
	}

	//�Զ�����ID
	//ID = node_id * 10 000 000 + data_counter ��������SV��ʶ��Data��Դ
	inline void generateID()
	{
		++ID_COUNT;
//		this->ID = node * ID_MASK + ID_COUNT;
		this->ID = ID_COUNT;
	}


public:

	//������ TTL �������ĳ�ʼֵ���� Epidemic ��ѡ��һ��ʹ�ã�Ĭ��ֵ���� 0�������� main �����л�ͨ�������в�����ֵ
	//ע�⣺���߲���ͬʱȡ����ֵ����Ϊ��ֵҲ���ڸ�ѡ��������ж�
	static int MAX_HOP;
	static int MAX_TTL;

	CData(int node, int timeBirth, int byte)
	{
		CData::init();
		this->node = node;
		this->time = this->timeBirth = timeBirth;
		this->size = byte;
		this->generateID();
		this->HOP = MAX_HOP;
		this->TTL = MAX_TTL;
	}

	~CData(){};

	static void deliverAtHotspot(int n)
	{
		DELIVERY_AT_HOTSPOT_COUNT += n;
	}
	static void deliverOnRoute(int n)
	{
		DELIVERY_ON_ROUTE_COUNT += n;
	}
	//�ú���Ӧ����MA��·������ʱ�������ͳ�ƽ��
	//ע�⣺�������������ͳ�Ʒ�����MA�����������ֵ�ļӺ����Ǵ��ڵ���ARRIVAL_COUNT�ģ�����������;
	static int getDeliveryAtHotspotCount()
	{
		return DELIVERY_AT_HOTSPOT_COUNT;
	}
	//�ú���Ӧ����MA��·������ʱ�������ͳ�ƽ��
	//ע�⣺�������������ͳ�Ʒ�����MA�����������ֵ�ļӺ����Ǵ��ڵ���ARRIVAL_COUNT�ģ�����������;
	static int getDeliveryTotalCount()
	{
		return DELIVERY_AT_HOTSPOT_COUNT + DELIVERY_ON_ROUTE_COUNT;
	}
	static double getDeliveryAtHotspotPercent()
	{
		if(DELIVERY_AT_HOTSPOT_COUNT == 0)
			return 0.0;
		return double(DELIVERY_AT_HOTSPOT_COUNT) / double( DELIVERY_AT_HOTSPOT_COUNT + DELIVERY_ON_ROUTE_COUNT );
	}

	//setters & getters
	inline int getTimeArrival() const
	{
		return timeArrival;
	}

	//���رȽϲ�����������mergeSort
	bool operator < (CData rt) const
	{
		return this->timeBirth < rt.getTimeBirth();
	}

	//���رȽϲ�����������ȥ��
	bool operator == (CData rt) const
	{
		return this->ID == rt.getID();
	}

	//���ز�����==���ڸ���ID�ж�identical
	bool operator == (int id) const
	{
		return this->ID == id;
	}

	//ʵ����ֻ����TTL
	inline void updateStatus(int currentTime)
	{
		if( useTTL() )
			this->TTL -= ( currentTime - time );
		this->time = currentTime;
	}
	inline void arriveSink(int timeArrival)
	{
		this->timeArrival = timeArrival;
		this->time = timeArrival;
		++ARRIVAL_COUNT;
		DELAY_SUM += timeArrival - timeBirth;
	}

	// TODO: call this func when receiving anything
	//�����ݱ�ת�������µĽڵ��Ӧ�õ��õĺ�����������������TTLʣ��ֵ��������ʱ���
	//ע�⣺���ݷ��ͷ�Ӧ�ڷ���֮ǰ���ʣ��HOP����1
	inline void arriveAnotherNode(int currentTime) override
	{
		CGeneralData::arriveAnotherNode(currentTime);

		if( useTTL() )
			this->TTL -= ( currentTime - time );
		this->time = currentTime;
	}

	//�ж��Ƿ��Ѿ�����������(TTL <= 0)������Ӧ����
	inline bool isOverdue() const
	{
		if( ! useTTL() )
			return false;
		else
			return TTL <= 0;
	}
	
	//�ж��Ƿ�����ת����HOP > 0�����������򲻷���SV��
	inline bool allowForward() const
	{
		if( ! useHOP() )
			return true;
		else
			return HOP > 0;
	}

	static bool useHOP()
	{
		if( MAX_HOP > 0 && MAX_TTL > 0 )
		{
			cout << endl << "Error @ CData::useHOP() : INIT_HOP > 0 && INIT_TTL > 0 " << endl;
			_PAUSE_;
			return false;
		}
		else
			return MAX_HOP > 0;
	}
	static bool useTTL()
	{
		if( MAX_HOP > 0 && MAX_TTL > 0 )
		{
			cout << endl << "Error @ CData::useTTL() : INIT_HOP > 0 && INIT_TTL > 0 " << endl;
			_PAUSE_;
			return false;
		}
		else
			return MAX_TTL > 0;	
	}
//	static int getNodeByMask(int id)
//	{
//		return id / ID_MASK;
//	}

	//ͳ������
	static int getDataCount()
	{
		return ID_COUNT;
	}
	static int getDeliveryCount()
	{
		return ARRIVAL_COUNT;
	}
	static double getDeliveryRatio()
	{
		if(ID_COUNT == 0)
			return 0;
		else
			return double(ARRIVAL_COUNT) / double(ID_COUNT);
	}
	static double getAverageDelay()
	{
		if(ARRIVAL_COUNT == 0)
			return 0;
		return DELAY_SUM / ARRIVAL_COUNT;
	}
	static double getAverageEnergyConsumption();

};
