#include "Hotspot.h"
#include "Preprocessor.h"

int CHotspot::ID_COUNT = 0;
int CHotspot::nHotspotCandidates = 0;
vector<CHotspot *> CHotspot::hotspotCandidates;
vector<CHotspot *> CHotspot::selectedHotspots;
vector<CHotspot *> CHotspot::oldSelectedHotspots;
vector<CHotspot *> CHotspot::deletedHotspots;

bool CHotspot::ifPositionExists(CPosition* pos)
{
	if(coveredPositions.empty())
		return false;

	vector<CPosition *>::iterator ipos;
	for(ipos = coveredPositions.begin(); ipos != coveredPositions.end(); ipos++)
	{
		if(*ipos == pos)
			return true;
	}
	return false;
}

void CHotspot::removePosition(CPosition* pos)
{
	if(! ifPositionExists(pos))
		return;

	vector<CPosition *>::iterator ipos;
	for(ipos = coveredPositions.begin(); ipos != coveredPositions.end(); ipos++)
	{
		if(*ipos == pos)
		{
			coveredPositions.erase(ipos);
			break;
		}
	}
}

void CHotspot::removePositionList(vector<CPosition *> positions)
{
	if(positions.empty())
		return;

	vector<CPosition *>::iterator ipos;
	for(ipos = positions.begin(); ipos != positions.end(); ipos++)
	{
		removePosition(*ipos);
	}		
}

void CHotspot::addPosition(CPosition* pos)
{
	if(ifPositionExists(pos))
		return;

	coveredPositions.push_back(pos);
}

void CHotspot::recalculateCenter()
{
	if(coveredPositions.empty())
		return;

	double sum_x = 0;
	double sum_y = 0;
	vector<CPosition *>::iterator ipos;
	for(ipos = coveredPositions.begin(); ipos != coveredPositions.end(); ipos++)
	{
		sum_x += (*ipos)->getX();
		sum_y += (*ipos)->getY();
	}

	this->x = sum_x / coveredPositions.size();
	this->y = sum_y / coveredPositions.size();
}

int CHotspot::getNCoveredPositionsForNode(int inode)
{
	int count = 0;
	for(vector<CPosition *>::iterator ipos = coveredPositions.begin(); ipos != coveredPositions.end(); ipos++)
	{
		if((*ipos)->getNode() == inode)
			count++;
	}
	return count;
}

bool CHotspot::ifNodeExists(int inode)
{
	return ( ifExists(coveredNodes, inode) );
}

void CHotspot::generateCoveredNodes()
{
	this->coveredNodes.clear();
	for(vector<CPosition *>::iterator ipos = coveredPositions.begin(); ipos != coveredPositions.end(); ipos++)
	{
		addToListUniquely(coveredNodes, (*ipos)->getNode());
	}
}

string CHotspot::toString(bool withDetails)
{
	ostringstream os;
	os << this->time << TAB << this->age << TAB << this->ID << TAB << this->x << TAB << this->y << TAB << coveredPositions.size() << TAB ;
	if(withDetails)
	{
		for(int i = 0; i < coveredPositions.size(); i++)
			os << coveredPositions[i]->getID() << TAB ;
	}
	return os.str();
}

double CHotspot::getOverlapArea(CHotspot *oldHotspot, CHotspot *newHotspot)
{
	double distance = CBasicEntity::getDistance(*oldHotspot, *newHotspot);
	double cos = ( distance / 2 ) / TRANS_RANGE;
	double sin = sqrt( 1 - cos * cos );
	double angle = acos(cos);
	double sector = ( angle / 2 ) * TRANS_RANGE * TRANS_RANGE;
	double triangle = ( TRANS_RANGE * ( distance / 2 ) ) * sin / 2;

	return ( sector - triangle ) * 4 ;
}

double CHotspot::getOverlapArea(vector<CHotspot *> oldHotspots, vector<CHotspot *> newHotspots)
{
	//Sort by Coordinate X to save time
	CPreprocessor::mergeSort(oldHotspots, CPreprocessor::ascendByLocationX);
	CPreprocessor::mergeSort(newHotspots, CPreprocessor::ascendByLocationX);

	vector<CHotspot *>::iterator iOld, iNew;
	double sumArea = 0;
	for(iOld = oldHotspots.begin(); iOld != oldHotspots.end(); iOld++)
	{
		for(iNew = newHotspots.begin(); iNew != newHotspots.end(); iNew++)
		{
			if( (*iOld)->getID() == (*iNew)->getID() )
			{
				sumArea += AREA_SINGE_HOTSPOT;
				continue;
			}

			if( (*iNew)->getX() + 2 * TRANS_RANGE <= (*iOld)->getX() )
				continue;
			if( (*iOld)->getX() + 2 * TRANS_RANGE <= (*iNew)->getX() )
				break;

			if( CBasicEntity::getDistance(**iOld, **iNew) < 2 * TRANS_RANGE)
			{
				sumArea += getOverlapArea(*iOld, *iNew);
			}
		}
	}

	return sumArea;
}

double CHotspot::getOverlapArea(vector<CHotspot *> hotspots)
{
	return getOverlapArea(hotspots, hotspots) - AREA_SINGE_HOTSPOT * hotspots.size();
}