/* Demo_03_Agent.cpp */

#include "Demo_03_Agent.h"
#include "repast_hpc/VN2DGridQuery.h"
#include "repast_hpc/Point.h"

RepastHPCDemoAgent::RepastHPCDemoAgent(repast::AgentId id): id_(id), c(0), total(0){ }

RepastHPCDemoAgent::RepastHPCDemoAgent(repast::AgentId id, double newC, double newTotal): id_(id), c(newC), total(newTotal){ }

RepastHPCDemoAgent::~RepastHPCDemoAgent(){ }


void RepastHPCDemoAgent::set(int currentRank, double newC, double newTotal){
    id_.currentRank(currentRank);
    c     = newC;
    total = newTotal;
}

void RepastHPCDemoAgent::play(repast::SharedContext<RepastHPCDemoAgent>* context,
                              repast::SharedDiscreteSpace<RepastHPCDemoAgent, repast::WrapAroundBorders, repast::SimpleAdder<RepastHPCDemoAgent> >* space){
	std::vector<RepastHPCDemoAgent*> agentsToPlay;
	int worldSize = repast::RepastProcess::instance()->worldSize();
	std::vector<int> agentLoc;
	space->getLocation(id_, agentLoc);
	repast::Point<int> center(agentLoc);
	/*repast::Point<int> next(center[0]+1,center[1]+0);
	if (center[0]+1 == 10){
		repast::Point<int> t (0,center[1]);
		next = t;
	}
	cout<<endl;	
	RepastHPCDemoAgent* agent = space->getObjectAt(next);*/
        repast::VN2DGridQuery<RepastHPCDemoAgent> vn2DGridQuery(space);
        vn2DGridQuery.query(center, 1, false, agentsToPlay);
	std::vector<RepastHPCDemoAgent*>::iterator it = agentsToPlay.begin();
	int temp = 0;
	int m = 0;
        while(it != agentsToPlay.end()){
  	    temp = this->c+(*it)->c;
	    if(temp == 1){
		this->c = 1;
	    }
	    else{
	    	this->c = 0;
	    }
	    it++;
	    m++;
        }    	
	//c = temp;
	cout<<"Center	"<<center<<"	C	"<<c<<"  M "<<m<<endl;
	/*if (agent!=NULL){
		c = agent->c;
		cout<<" Center	"<<center<<" Neighbor "<<next<<" AGENT	"<<agent->getId()<<" C "<<c<<endl;
	}*/
}

void RepastHPCDemoAgent::asynch(repast::SharedContext<RepastHPCDemoAgent>* context,
                              repast::SharedDiscreteSpace<RepastHPCDemoAgent, repast::WrapAroundBorders, repast::SimpleAdder<RepastHPCDemoAgent> >* space){
	c = total;
}

/* Serializable Agent Package Data */

RepastHPCDemoAgentPackage::RepastHPCDemoAgentPackage(){ }

RepastHPCDemoAgentPackage::RepastHPCDemoAgentPackage(int _id, int _rank, int _type, int _currentRank, double _c, double _total):
id(_id), rank(_rank), type(_type), currentRank(_currentRank), c(_c), total(_total){ }
