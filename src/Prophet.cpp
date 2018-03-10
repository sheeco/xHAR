#include "Global.h"
#include "Configuration.h"
#include "Node.h"
#include "Sink.h"
#include "SMac.h"
#include "HDC.h"
#include "Prophet.h"
#include "SortHelper.h"
#include "PrintHelper.h"

map<int, map<int, CProphet::CDeliveryPredRecord>> CProphet::deliveryPreds;  //< ID:x, P(this->id, x) >，sink节点ID为0将位于最前，便于查找

int CProphet::INIT_PRED;
int CProphet::DECAY_PRED;
int CProphet::TRANS_PRED;
int CProphet::TRANS_STRICT_BY_PRED;


CProphet::CProphet()
{
	
}

CProphet::~CProphet()
{
	
}

void CProphet::initDeliveryPreds(int now)
{
	vector<CNode*> nodes = CNode::getAllNodes();
	for( CNode* pnode : nodes )
	{
		deliveryPreds[pnode->getID()][CSink::getSink()->getID()] = CDeliveryPredRecord(INIT_PRED, now);
		for( CNode* pWithNode : nodes )
		{
			if( pnode->getID() != pWithNode->getID() )
				deliveryPreds[pnode->getID()][pWithNode->getID()] = CDeliveryPredRecord(INIT_PRED, now);
		}
	}
}

inline void CProphet::decayDeliveryPreds(int forNode, int withNode, int now)
{
	CDeliveryPredRecord& record = deliveryPreds[forNode][withNode];

	if( record.timestamp == now )
		return;

	record.pred *= pow(DECAY_PRED, ( now - record.timestamp ) / getConfig<int>("trace", "interval"));
	record.timestamp = now;
}

inline void CProphet::DecayDeliveryPreds(CNode * node, int now)
{
	if( now % getConfig<int>("trace", "interval") != 0 )
		return;

	int sink = CSink::getSink()->getID();
	for( CNode* pnode : CNode::getAliveNodes() )
	{
		int a = pnode->getID();
		decayDeliveryPreds(a, sink, now);

		for( CNode* pWithNode : CNode::getAliveNodes() )
		{
			int b = pWithNode->getID();
			if( pnode->getID() != pWithNode->getID() )
			{
				decayDeliveryPreds(a, b, now);
			}
		}
	}
}

inline void CProphet::updateDeliveryPredsBetween(int a, int b, int now)
{
	map<int, CDeliveryPredRecord > &recordA = deliveryPreds[a],
		recordB = deliveryPreds[b];

	//avoid duplicate update
	if( recordA[b].timestamp == now )
		return;

	//update P(a, b) & P(b, a)
	decayDeliveryPreds(a, b, now);
	decayDeliveryPreds(b, a, now);
	double oldPred = recordA[b].pred;
	double betweenAB = oldPred + ( 1 - oldPred ) * INIT_PRED;
	recordA[b] = recordB[a] = CDeliveryPredRecord(betweenAB, now);


	int sink = CSink::getSink()->getID();
	decayDeliveryPreds(a, sink, now);
	decayDeliveryPreds(b, sink, now);
	double a2sink = recordA[sink].pred;  //copy
	double b2sink = recordB[sink].pred;

	//update P(a, sink)
	oldPred = a2sink;
	recordA[sink].pred = oldPred + ( 1 - oldPred ) * betweenAB * b2sink * TRANS_PRED;
	recordA[sink].timestamp = now;

	//update P(b, sink)
	oldPred = b2sink;
	recordB[sink].pred = oldPred + ( 1 - oldPred ) * betweenAB * a2sink * TRANS_PRED;
	recordB[sink].timestamp = now;

	for( CNode* pnode : CNode::getAliveNodes() )
	{
		if( pnode->getID() != a
		   && pnode->getID() != b )
		{
			int d = pnode->getID();
			decayDeliveryPreds(a, d, now);
			decayDeliveryPreds(b, d, now);
			double a2d = recordA[d].pred;  //copy
			double b2d = recordB[d].pred;

			//update P(a, d)
			oldPred = a2d;
			recordA[d].pred = oldPred + ( 1 - oldPred ) * betweenAB * b2d* TRANS_PRED;
			recordA[d].timestamp = now;

			//update P(b, d)
			oldPred = b2d;
			recordB[d].pred = oldPred + ( 1 - oldPred ) * betweenAB * a2d * TRANS_PRED;
			recordB[d].timestamp = now;
		}
	}
}

inline void CProphet::updateDeliveryPredsWithSink(int node, int now)
{
	int sink = CSink::getSink()->getID();
	if( deliveryPreds[node][sink].timestamp == now )
		return;

	decayDeliveryPreds(node, sink, now);

	double oldPred = deliveryPreds[node][sink].pred;
	deliveryPreds[node][sink].pred = oldPred + ( 1 - oldPred ) * INIT_PRED;
	deliveryPreds[node][sink].timestamp = now;
}

bool CProphet::shouldForward(int me, int you, int now)
{
	int sink = CSink::getSink()->getID();
	decayDeliveryPreds(me, sink, now);
	decayDeliveryPreds(you, sink, now);

	double me2sink = deliveryPreds[me][sink].pred,
		you2sink = deliveryPreds[you][sink].pred;

	if( EQUAL( me2sink, you2sink ) )
	{
		if( TRANS_STRICT_BY_PRED )
			return false;
		else
			return Bet(0.5);
	}
	else
		return you2sink > me2sink;
}

vector<CPacket*> CProphet::receivePackets(CGeneralNode & gToNode, CGeneralNode & gFromNode, vector<CPacket*> packets, int now)
{
	vector<CPacket*> packetsToSend;

	if( typeid( gToNode ) == typeid( CSink ) )
	{
		CSink* toSink = dynamic_cast< CSink* >( &gToNode );

		/*********************************************** Sink <- Node *******************************************************/

		if( typeid( gFromNode ) == typeid( CNode ) )
		{
			CNode* fromNode = dynamic_cast<CNode*>( &gFromNode );
			packetsToSend = CProphet::receivePackets(toSink, fromNode, packets, now);
		}
	}

	else if( typeid( gToNode ) == typeid( CNode ) )
	{
		CNode* node = dynamic_cast<CNode*>( &gToNode );

		/*********************************************** Node <- Sink *******************************************************/

		if( typeid( gFromNode ) == typeid( CSink ) )
		{
			CSink* fromSink = dynamic_cast< CSink* >( &gFromNode );
			packetsToSend = CProphet::receivePackets(node, fromSink, packets, now);
		}

		/*********************************************** Node <- Node *******************************************************/

		else if( typeid( gFromNode ) == typeid( CNode ) )
		{
			CNode* fromNode = dynamic_cast<CNode*>( &gFromNode );
			packetsToSend = CProphet::receivePackets(node, fromNode, packets, now);
		}
	}

	return packetsToSend;
}

vector<CPacket*> CProphet::receivePackets(CNode* node, CSink* sink, vector<CPacket*> packets, int now)
{
	vector<CPacket*> packetsToSend;
	CCtrl* ctrlToSend = nullptr;
	CCtrl* indexToSend = nullptr;
	CCtrl* nodataToSend = nullptr;  //NODATA包代表缓存为空，没有适合传输的数据
	vector<CData> dataToSend;  //空vector代表拒绝传输数据
	bool wait = false;

	for(vector<CPacket*>::iterator ipacket = packets.begin(); ipacket != packets.end(); )
	{

		/***************************************** rcv Ctrl Message *****************************************/

		if( typeid(**ipacket) == typeid(CCtrl) )
		{
			CCtrl* ctrl = dynamic_cast<CCtrl*>(*ipacket);
			switch( ctrl->getType() )
			{

				/*************************************** rcv RTS **************************************/

			case CCtrl::_rts:

				updateDeliveryPredsWithSink(node->getID(), now);

				if( node->getAllData().empty() )
				{
					return packetsToSend;
				}
				//CTS
				ctrlToSend = new CCtrl(node->getID(), now, getConfig<int>("data", "size_ctrl"), CCtrl::_cts);
				// + DATA
				dataToSend = node->getDataForTrans(0, true);

				wait = true;
				// TODO: mark skipRTS ?
				// TODO: connection established ?
				break;

			case CCtrl::_cts:

				break;

			case CCtrl::_capacity:

				break;

			case CCtrl::_index:

				break;

			case CCtrl::_no_data:

				break;

				/*************************************** rcv ACK **************************************/

			case CCtrl::_ack:

				node->startDiscovering();

				//收到空的ACK时，结束本次数据传输
				if( ctrl->getACK().empty() )
					return packetsToSend;
				//clear data with ack
				else
					node->dropDataByAck( ctrl->getACK() );

				CPrintHelper::PrintCommunication(now, node->getName(), sink->getName(), ctrl->getACK().size());

				return packetsToSend;

				break;

			default:
				break;
			}
			++ipacket;
		}
		else
		{
			++ipacket;
		}
	}

	/********************************** wrap ***********************************/

	if( ctrlToSend != nullptr )
	{
		packetsToSend.push_back(ctrlToSend);
	}
	if( indexToSend != nullptr )
	{
		packetsToSend.push_back(indexToSend);
	}
	if( nodataToSend != nullptr )
	{
		packetsToSend.push_back(nodataToSend);
	}
	if( ! dataToSend.empty() )
	{
		for(auto idata = dataToSend.begin(); idata != dataToSend.end(); ++idata)
			packetsToSend.push_back(new CData(*idata));
	}
	if( wait )
	{
		int timeDelay = CMacProtocol::getTransmissionDelay(packetsToSend);
		node->delayDiscovering(timeDelay);
		node->delaySleep(timeDelay);
	}
	return packetsToSend;
	
}

vector<CPacket*> CProphet::receivePackets(CSink* sink, CNode* fromNode, vector<CPacket*> packets, int now)
{
	vector<CPacket*> packetsToSend;
	CCtrl* ctrlToSend = nullptr;

	for(vector<CPacket*>::iterator ipacket = packets.begin(); ipacket != packets.end(); )
	{
		if( typeid(**ipacket) == typeid(CCtrl) )
		{
			CCtrl* ctrl = dynamic_cast<CCtrl*>(*ipacket);
			switch( ctrl->getType() )
			{
			case CCtrl::_rts:

				break;

			case CCtrl::_cts:

				break;

			case CCtrl::_capacity:

				break;

			case CCtrl::_index:

				break;

			case CCtrl::_ack:

				break;

			case CCtrl::_no_data:

				break;

			default:

				break;
			}
			++ipacket;
		}

		else if( typeid(**ipacket) == typeid(CData) )
		{
			//extract data packet
			vector<CData> datas;
			do
			{
				datas.push_back( *dynamic_cast<CData*>(*ipacket) );
				++ipacket;
			} while( ipacket != packets.end() );

			//accept data into buffer
			vector<CData> ack = CSink::bufferData(now, datas);

			//ACK（如果收到的数据全部被丢弃，发送空的ACK）
			ctrlToSend = new CCtrl(CSink::getSink()->getID(), ack, now, getConfig<int>("data", "size_ctrl"), CCtrl::_ack);
		}
	}

	/********************************** wrap ***********************************/

	if( ctrlToSend != nullptr )
		packetsToSend.push_back(ctrlToSend);

	return packetsToSend;

}

vector<CPacket*> CProphet::receivePackets(CNode* node, CNode* fromNode, vector<CPacket*> packets, int now)
{
	vector<CPacket*> packetsToSend;
	CCtrl* ctrlToSend = nullptr;
	CCtrl* capacityToSend = nullptr;
	CCtrl* indexToSend = nullptr;
	CCtrl* nodataToSend = nullptr;  //NODATA包代表缓存为空，没有适合传输的数据
	vector<CData> dataToSend;  //空vector代表拒绝传输数据
	int capacity = -1;

//	int debugNodeID = 0;
//	int debugFromID = 0;

	for(vector<CPacket*>::iterator ipacket = packets.begin(); ipacket != packets.end(); )
	{

		/***************************************** rcv Ctrl Message *****************************************/

		if( typeid(**ipacket) == typeid(CCtrl) )
		{
			CCtrl* ctrl = dynamic_cast<CCtrl*>(*ipacket);
			switch( ctrl->getType() )
			{

				/*************************************** rcv RTS **************************************/

			case CCtrl::_rts:

				//skip if has spoken recently
				if( node->hasCommunicatedRecently(fromNode->getID(), now) )
				{
					CPrintHelper::PrintCommunication(now, node->getName(), fromNode->getName(), "skip");
					return packetsToSend;
				}
				//rcv RTS from node
				else
				{
					//CTS
					ctrlToSend = new CCtrl(node->getID(), now, getConfig<int>("data", "size_ctrl"), CCtrl::_cts);

					// + Capacity
					if( getConfig<CConfiguration::EnumRelayScheme>("node", "scheme_relay") == config::_selfish 
						&& node->hasData() )
						capacityToSend = new CCtrl(node->getID(), now, getConfig<int>("data", "size_ctrl"), CCtrl::_capacity);

				}

				// TODO: mark skipRTS ?
				// TODO: connection established ?
				break;

				/*************************************** rcv CTS **************************************/

			case CCtrl::_cts:

				// TODO: connection established ?
				break;

				/************************************* rcv capacity ***********************************/

			case CCtrl::_capacity:

				capacity = ctrl->getCapacity();

				break;

				/****************************** rcv Data Index ( dp / sv ) ****************************/

			// _index用于触发节点之间的DP更新，在每个节点对之间只需进行一次，因此_index仅随_rts发送
			case CCtrl::_index:

				//update preds
				updateDeliveryPredsBetween(node->getID(), fromNode->getID(), now);

				// + DATA / NODATA
				//路由协议允许向该节点转发数据
				if( shouldForward(node->getID(), fromNode->getID(), now ) )
				{
					if( capacity == 0 )
					{
						return packetsToSend;
					}

					dataToSend = node->getDataForTrans(0, true);

					//但缓存为空
					if( dataToSend.empty() )
						nodataToSend = new CCtrl(node->getID(), now, getConfig<int>("data", "size_ctrl"), CCtrl::_no_data);
					else if( capacity > 0 )
					{
						CGeneralNode::removeDataByCapacity(dataToSend, capacity, false);
					}
					
					//如果路由协议允许向该节点发送数据，对方节点就不允许发送数据，因此无需发送capacity
					if( capacityToSend != nullptr )
					{
						free(capacityToSend);
						capacityToSend = nullptr;
					}
				}
				//否则，不允许转发数据时，dataToSend留空

				//注意：如果（取决于Prophet的要求）两个节点都拒绝发送数据，此处将导致空的响应，直接结束本次数据传输
				//     否则并不结束传输，还将为对方发来的数据发送ACK；

				break;

				/*************************************** rcv ACK **************************************/

			case CCtrl::_ack:

				//加入最近邻居列表
				node->updateCommunicationHistory( fromNode->getID(), now );

				node->startDiscovering();

				//收到空的ACK时，结束本次数据传输
				if( ctrl->getACK().empty() )
					return packetsToSend;
				//clear data with ack
				else
					node->dropDataByAck( ctrl->getACK() );

				CPrintHelper::PrintCommunication(now, node->getName(), fromNode->getName(), ctrl->getACK().size());

				return packetsToSend;

				break;

			case CCtrl::_no_data:

				//收到NODATA，也将回复一个空的ACK，即，也将被认为数据传输成功
				//空的ACK
				ctrlToSend = new CCtrl(node->getID(), vector<CData>(), now, getConfig<int>("data", "size_ctrl"), CCtrl::_ack);
				//加入最近邻居列表
				node->updateCommunicationHistory( fromNode->getID(), now );
				node->startDiscovering();


				//维持数据传输的单向性：如果收到数据或NODATA，就不发送数据或NODATA（注意：前提是控制包必须在数据包之前）
				dataToSend.clear();
				if( nodataToSend != nullptr )
				{
					free(nodataToSend);
					nodataToSend = nullptr;
				}

				break;

			default:
				break;
			}
			++ipacket;
		}

		/******************************************* rcv Data *******************************************/

		else if( typeid(**ipacket) == typeid(CData) )
		{
			//extract data packet
			vector<CData> datas;
			do
			{
				datas.push_back( *dynamic_cast<CData*>(*ipacket) );
				++ipacket;
			} while( ipacket != packets.end() );

			//维持数据传输的单向性：如果收到数据或NODATA，就不发送数据或NODATA（注意：前提是控制包必须在数据包之前）
			dataToSend.clear();
			if( nodataToSend != nullptr )
			{
				free(nodataToSend);
				nodataToSend = nullptr;
			}

			//accept data into buffer
			vector<CData> ack;
			//存入收到的数据并发送ACK
			ack = node->bufferData(now, datas);
			ctrlToSend = new CCtrl(node->getID(), ack, now, getConfig<int>("data", "size_ctrl"), CCtrl::_ack);
			//加入最近邻居列表
			node->updateCommunicationHistory( fromNode->getID(), now );
			node->startDiscovering();

		}
	}

	/********************************** wrap ***********************************/

	if( ctrlToSend != nullptr )
	{
		packetsToSend.push_back(ctrlToSend);
	}
	if( capacityToSend != nullptr )
	{
		packetsToSend.push_back(capacityToSend);
	}
	if( indexToSend != nullptr )
	{
		packetsToSend.push_back(indexToSend);
	}
	if( nodataToSend != nullptr )
	{
		packetsToSend.push_back(nodataToSend);
	}
	if( ! dataToSend.empty() )
	{
		for(auto idata = dataToSend.begin(); idata != dataToSend.end(); ++idata)
			packetsToSend.push_back(new CData(*idata));
	}

	return packetsToSend;

}

inline void CProphet::CommunicateBetweenNeighbors(int now)
{
	static bool print = false;
	if( now == 0
	   || print )
	{
		CPrintHelper::PrintHeading(now, "DATA DELIVERY");
		print = false;
	}

	// Prophet: sink => nodes
	CSink* sink = CSink::getSink();
	//use default neighbor pool (only sensor nodes)
	CMacProtocol::transmitFrame(*sink, sink->sendRTS(now), now, receivePackets);

	vector<CNode*> nodes = CNode::getAllNodes();

	for( vector<CNode*>::iterator srcNode = nodes.begin(); srcNode != nodes.end(); ++srcNode )
	{
		if( ( *srcNode )->isDiscovering() )
		{
			CMacProtocol::transmitFrame(**srcNode, ( *srcNode )->sendRTSWithCapacityAndIndex(now), now, receivePackets);
			( *srcNode )->finishDiscovering();
		}
	}

	if( ( now + getConfig<int>("simulation", "slot") ) % getConfig<int>("log", "slot_log") == 0 )
	{
		CPrintHelper::PrintPercentage("Delivery Ratio", CData::getDeliveryRatio());
		CPrintHelper::PrintNewLine();
		print = true;
	}
}

bool CProphet::Init(int now)
{
	if(getConfig<CConfiguration::EnumMacProtocolScheme>("simulation", "mac_protocol") == config::_hdc)
		CHDC::Init();
	else if(getConfig<CConfiguration::EnumMacProtocolScheme>("simulation", "mac_protocol") == config::_smac)
		CSMac::Init();

	INIT_PRED = getConfig<double>("prophet", "init_pred");
	DECAY_PRED = getConfig<double>("prophet", "decay_pred");
	TRANS_PRED = getConfig<double>("prophet", "trans_pred");
	TRANS_STRICT_BY_PRED = getConfig<bool>("prophet", "trans_strict_by_pred");

	CProphet::initDeliveryPreds(now);
	return true;
}

bool CProphet::Operate(int now)
{
	if( getConfig<CConfiguration::EnumMacProtocolScheme>("simulation", "mac_protocol") == config::_hdc )
		CHDC::Prepare(now);
	else if( getConfig<CConfiguration::EnumMacProtocolScheme>("simulation", "mac_protocol") == config::_smac )
		CSMac::Prepare(now);

	if( !CNode::UpdateNodeStatus(now) )
		return false;

	CommunicateBetweenNeighbors(now);

	PrintInfo(now);

	return true;
}
