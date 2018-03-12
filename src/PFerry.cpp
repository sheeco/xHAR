#include "PFerry.h"


string CTracePrediction::KEYWORD_PREDICT;
string CTracePrediction::EXTENSION_PAN_FILE;


int CPFerryTask::COUNT_TASK_MET;
int CPFerryTask::COUNT_TASK;
map<int, int> CPFerryTask::countTaskForNodes;

bool CPFerryMANode::RETURN_ONCE_TASK_MET = false;

vector<CPFerryMANode*> CPFerry::allMAs;
vector<CPFerryMANode*> CPFerry::freeMAs;
vector<CPFerryMANode*> CPFerry::busyMAs;


vector<CNode*> CPFerry::targetNodes;
//vector<CNode*> CPFerry::missNodes;
vector<CNode*> CPFerry::candidateNodes; //untargetted nodes

map<int, map<int, CTracePrediction*>> CPFerry::predictions;
vector<int> CPFerry::strides;
map<int, CPFerry::CNodeRecord> CPFerry::collectionRecords;  //known latest collection time for nodes

string CPFerry::PATH_PREDICT;
config::EnumRoutingProtocolScheme CPFerry::PRESTAGE_PROTOCOL = config::_xhar;
int CPFerry::STARTTIME = INVALID;


CPFerryMANode::~CPFerryMANode()
{
	this->tasks.clear();
	for( pair<int, CPFerryTask*> p : tasks )
		FreePointer(p.second);
	FreePointerVector(this->taskHistory);
}

void CPFerryMANode::updateStatus(int now)
{
	if( this->time < 0 )
		this->time = now;

	/********************************* Vacant MA ************************************/
	
	if( !this->isBusy() )
	{
		if( !CBasicEntity::withinRange(*this, *CSink::getSink(), getConfig<int>("trans", "range_trans")) )
			throw string("CPFerryMANode::updateStatus(): " + this->getName() + " is free but not around Sink.");

		this->setTime(now);
		return;
	}

	/********************************* Busy MA ************************************/

	int duration = now - this->time;

	//updateTimerOccupied(time);

	//���������������������sink
	if( !this->isReturningToSink()
	   && this->isFull() )
	{
		CPrintHelper::PrintDetail(time, this->getName() + " is returning to Sink due to buffer filled.", 2);
		this->setReturningToSink();
	}

	/********************* Returning to Sink **********************/

	if( isReturningToSink() )
	{
		this->moveToward(*CSink::getSink(), duration, this->getSpeed());
		this->setTime(now);
		return;
	}

	/********************* Moving on the Route **********************/

	/***************** Waiting ****************/

	int timeLeftAfterWaiting = 0;
	if( this->isWaiting() )
	{
		CBasicEntity* waypoint = getAtWaypoint();
		int copyWaitingWindow = this->waitingWindow;
		timeLeftAfterWaiting = this->wait(duration);

		//����ȴ��Ѿ���������¼һ�εȴ�
		if( !this->isWaiting() )
		{
			if( waypoint == nullptr )
				throw string("CPFerryMANode::updateStatus(): waypoint = nullptr");

			CPrintHelper::PrintDetail(this->time + duration - timeLeftAfterWaiting
									  , this->getName() + " has waited at " + waypoint->format() + " for " 
									  + STRING(copyWaitingWindow) + "s.", 3);
		}
	}
	else
	{
		timeLeftAfterWaiting = duration;
	}

	if( timeLeftAfterWaiting == 0 )
	{
		this->setTime(now);
		return;
	}

	/***************** Actually Moving ****************/

	this->setTime(now - timeLeftAfterWaiting);  //��ʱ�����ڵȴ��������ƶ�������ʼ��ʱ���
	atPoint = nullptr;  //�뿪֮ǰ��·��

	CBasicEntity *toPoint = route->getToPoint();
	int minWaitingTime = route->getWaitingTime();
	int timeLeftAfterArrival = this->moveToward(*toPoint, timeLeftAfterWaiting, this->getSpeed());

	//����ѵ���Ŀ�ĵ�
	if( timeLeftAfterArrival >= 0 )
	{
		CSink *psink = nullptr;

		//��Ŀ�ĵص�������waypoint
		CPosition *ppos = nullptr;
		if( ( ppos = dynamic_cast< CPosition * >( toPoint ) ) != nullptr )
		{
			this->atPoint = toPoint;

			CPrintHelper::PrintDetail(now, this->getName() + " arrives at task position " + ppos->format() + ".", 2);

			CPFerryTask* task = this->findTask(ppos->getNode());
			if( !task )
				throw string("CPFerryMANode::updateStatus(): Cannot find task for node id " + STRING(ppos->getNode()) + ".");
			else
			{
				if(!RETURN_ONCE_TASK_MET
				   || !task->isMet())
				{
					//set waiting time
					int timePred = task->getTime();
					this->setWaiting(max(minWaitingTime, timePred - this->getTime() + minWaitingTime));
				}
			}
		}
		//��Ŀ�ĵص������� sink
		else if( ( psink = dynamic_cast< CSink * >( toPoint ) ) != nullptr )
		{
			CPrintHelper::PrintDetail(time, this->getName() + " has returned to Sink.", 2);
			this->setReturningToSink();
		}
	}
	return;
}

CPFerry::CPFerry()
{
}

CPFerry::~CPFerry()
{
}
