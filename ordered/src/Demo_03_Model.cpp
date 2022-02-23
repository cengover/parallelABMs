/* Demo_03_Model.cpp */

#include <stdio.h>
#include <vector>
#include <boost/mpi.hpp>
#include "repast_hpc/AgentId.h"
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Utilities.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/initialize_random.h"
#include "repast_hpc/SVDataSetBuilder.h"
#include "repast_hpc/Point.h"

#include "Demo_03_Model.h"


RepastHPCDemoAgentPackageProvider::RepastHPCDemoAgentPackageProvider(repast::SharedContext<RepastHPCDemoAgent>* agentPtr): agents(agentPtr){ }

void RepastHPCDemoAgentPackageProvider::providePackage(RepastHPCDemoAgent * agent, std::vector<RepastHPCDemoAgentPackage>& out){
    repast::AgentId id = agent->getId();
    RepastHPCDemoAgentPackage package(id.id(), id.startingRank(), id.agentType(), id.currentRank(), agent->getC(), agent->getTotal());
    out.push_back(package);
}

void RepastHPCDemoAgentPackageProvider::provideContent(repast::AgentRequest req, std::vector<RepastHPCDemoAgentPackage>& out){
    std::vector<repast::AgentId> ids = req.requestedAgents();
    for(size_t i = 0; i < ids.size(); i++){
        providePackage(agents->getAgent(ids[i]), out);
    }
}

RepastHPCDemoAgentPackageReceiver::RepastHPCDemoAgentPackageReceiver(repast::SharedContext<RepastHPCDemoAgent>* agentPtr): agents(agentPtr){}

RepastHPCDemoAgent * RepastHPCDemoAgentPackageReceiver::createAgent(RepastHPCDemoAgentPackage package){
    repast::AgentId id(package.id, package.rank, package.type, package.currentRank);
    return new RepastHPCDemoAgent(id, package.c, package.total);
}

void RepastHPCDemoAgentPackageReceiver::updateAgent(RepastHPCDemoAgentPackage package){
    repast::AgentId id(package.id, package.rank, package.type);
    RepastHPCDemoAgent * agent = agents->getAgent(id);
    agent->set(package.currentRank, package.c, package.total);
}



DataSource_AgentTotals::DataSource_AgentTotals(repast::SharedContext<RepastHPCDemoAgent>* c) : context(c){ }

std::vector<int> DataSource_AgentTotals::getData(){
	std::vector<int> Cs;
	repast::SharedContext<RepastHPCDemoAgent>::const_local_iterator iter    = context->localBegin();
	repast::SharedContext<RepastHPCDemoAgent>::const_local_iterator iterEnd = context->localEnd();
	while( iter != iterEnd) {
		Cs.push_back((*iter)->getC());
		iter++;
	}
	return Cs;
}

DataSource_AgentCTotals::DataSource_AgentCTotals(repast::SharedContext<RepastHPCDemoAgent>* c) : context(c){ }

int DataSource_AgentCTotals::getData(){
	int sum = 0;
	repast::SharedContext<RepastHPCDemoAgent>::const_local_iterator iter = context->localBegin();
	repast::SharedContext<RepastHPCDemoAgent>::const_local_iterator iterEnd = context->localEnd();
	while( iter != iterEnd) {
		sum+= (*iter)->getC();
		iter++;
	}
	return sum;
}

RepastHPCDemoModel::RepastHPCDemoModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm): context(comm){
	props = new repast::Properties(propsFile, argc, argv, comm);
	stopAt = repast::strToInt(props->getProperty("stop.at"));
	countOfAgents = repast::strToInt(props->getProperty("count.of.agents"));
	initializeRandom(*props, comm);

	if(repast::RepastProcess::instance()->rank() == 0) props->writeToSVFile("./output/record.csv");
	provider = new RepastHPCDemoAgentPackageProvider(&context);
	receiver = new RepastHPCDemoAgentPackageReceiver(&context);
	
	int w = 40;
	int h = 4;
	repast::Point<double> origin(0,0);
	repast::Point<double> extent(w,h);

	repast::GridDimensions gd(origin, extent);
	int worldSize = repast::RepastProcess::instance()->worldSize();
	std::vector<int> processDims;
	processDims.push_back(worldSize);
	processDims.push_back(0);

	discreteSpace = new repast::SharedDiscreteSpace<RepastHPCDemoAgent, repast::WrapAroundBorders, repast::SimpleAdder<RepastHPCDemoAgent> >("AgentDiscreteSpace", gd, processDims, 1, comm);

	std::cout << "RANK " << repast::RepastProcess::instance()->rank() << " BOUNDS: " << discreteSpace->bounds().origin() << " " << discreteSpace->bounds().extents() << std::endl;
    
   	context.addProjection(discreteSpace);
    
	// Data collection
	// Create the data set builder
	std::string fileOutputName("./output/agent_total_data.csv");
	repast::SVDataSetBuilder builder(fileOutputName.c_str(), ",", repast::RepastProcess::instance()->getScheduleRunner().schedule());
	
	// Create the individual data sets to be added to the builder
	//DataSource_AgentTotals* agentTotals_DataSource = new DataSource_AgentTotals(&context);
	//builder.addDataSource(createSVDataSource("Total", agentTotals_DataSource, std::plus<int>()));

	DataSource_AgentCTotals* agentCTotals_DataSource = new DataSource_AgentCTotals(&context);
	builder.addDataSource(createSVDataSource("C", agentCTotals_DataSource, std::plus<int>()));

	// Use the builder to create the data set
	agentValues = builder.createDataSet();
}

RepastHPCDemoModel::~RepastHPCDemoModel(){
	delete props;
	delete provider;
	delete receiver;	
	delete agentValues;

}

void RepastHPCDemoModel::init(){
	int rank = repast::RepastProcess::instance()->rank();
	int worldSize = repast::RepastProcess::instance()->worldSize();
	int portion = 40/worldSize;
	int count = 0;
		for (int x = rank*portion; x < rank*portion + portion ; x++){
			for (int y = 0; y < 4; y++){
				repast::Point<int> initialLocation(x,y);
				repast::AgentId id(count, rank, 0);
				id.currentRank(rank);
				RepastHPCDemoAgent* agent = new RepastHPCDemoAgent(id);
				//if(x == (40/worldSize)-1){agent->c = 1;}
				agent->c = 1.0;
				agent->total = 1.0;
				//repast::Random::instance()->nextDouble();; //Only agent at (3,0) starts with 1
				context.addAgent(agent);
				discreteSpace->moveTo(id, initialLocation);
				std::cout <<  "RANK " << repast::RepastProcess::instance()->rank()<<"	LOC "<<initialLocation<<" AGENTID "<<agent->getId()<<std::endl;
				count++;
		}
			}
}

void RepastHPCDemoModel::requestAgents(){
	int rank = repast::RepastProcess::instance()->rank();
	int worldSize = repast::RepastProcess::instance()->worldSize();
	repast::AgentRequest req(rank);
	int t = repast::RepastProcess::instance()->rank()+1;
	if (t == worldSize){
		t = 0;
	} 
	for(int i = 0; i < worldSize; i++){                     // For each process
		if(i == t&&t!=repast::RepastProcess::instance()->rank()){                                      // ... except this one
			std::vector<RepastHPCDemoAgent*> agents;
			repast::AgentId id(1, i, 0, i);
			//RepastHPCDemoAgent* agent = context.getAgent(id);
			id.currentRank(i);
			req.addRequest(id);                      // Add it to the agent request
		
		}
	}
	t = repast::RepastProcess::instance()->rank()-1;
	if (t ==-1){
		t = worldSize-1;
	} 
	for(int i = 0; i < worldSize; i++){                     // For each process
		if(i == t&&t!=repast::RepastProcess::instance()->rank()){                                      // ... except this one
			std::vector<RepastHPCDemoAgent*> agents;
			repast::AgentId id(countOfAgents/worldSize-1, i, 0, i);
			//RepastHPCDemoAgent* agent = context.getAgent(id);
			id.currentRank(i);
			req.addRequest(id);                      // Add it to the agent request
		
		}
	}
    repast::RepastProcess::instance()->requestAgents<RepastHPCDemoAgent, RepastHPCDemoAgentPackage, RepastHPCDemoAgentPackageProvider, RepastHPCDemoAgentPackageReceiver>(context, req, *provider, *receiver, *receiver);
}

void RepastHPCDemoModel::cancelAgentRequests(){
	int rank = repast::RepastProcess::instance()->rank();
	if(rank == 0) std::cout << "CANCELING AGENT REQUESTS" << std::endl;
	repast::AgentRequest req(rank);
	
	repast::SharedContext<RepastHPCDemoAgent>::const_state_aware_iterator non_local_agents_iter  = context.begin(repast::SharedContext<RepastHPCDemoAgent>::NON_LOCAL);
	repast::SharedContext<RepastHPCDemoAgent>::const_state_aware_iterator non_local_agents_end   = context.end(repast::SharedContext<RepastHPCDemoAgent>::NON_LOCAL);
	while(non_local_agents_iter != non_local_agents_end){
		req.addCancellation((*non_local_agents_iter)->getId());
		non_local_agents_iter++;
	}
    repast::RepastProcess::instance()->requestAgents<RepastHPCDemoAgent, RepastHPCDemoAgentPackage, RepastHPCDemoAgentPackageProvider, RepastHPCDemoAgentPackageReceiver>(context, req, *provider, *receiver, *receiver);
	
	std::vector<repast::AgentId> cancellations = req.cancellations();
	std::vector<repast::AgentId>::iterator idToRemove = cancellations.begin();
	while(idToRemove != cancellations.end()){
		context.importedAgentRemoved(*idToRemove);
		idToRemove++;
	}
}

void RepastHPCDemoModel::removeLocalAgents(){
	int rank = repast::RepastProcess::instance()->rank();
	if(rank == 0) std::cout << "REMOVING LOCAL AGENTS" << std::endl;
	for(int i = 0; i < 1; i++){
		repast::AgentId id(i, rank, 0);
		repast::RepastProcess::instance()->agentRemoved(id);
		context.removeAgent(id);
	}
    repast::RepastProcess::instance()->synchronizeAgentStatus<RepastHPCDemoAgent, RepastHPCDemoAgentPackage, RepastHPCDemoAgentPackageProvider, RepastHPCDemoAgentPackageReceiver>(context, *provider, *receiver, *receiver);
}

void RepastHPCDemoModel::doSomething(){
	int worldSize = repast::RepastProcess::instance()->worldSize();
	int rank = repast::RepastProcess::instance()->rank();
	std::vector<RepastHPCDemoAgent*> agents;
	int portion = 40/worldSize;
	int count = 0;
	for (int x = rank*portion; x < rank*portion+portion; x++){
		for(int y = 0; y < 4; y++){
			repast::AgentId id(count, rank, 0);
			id.currentRank(rank);
			RepastHPCDemoAgent* agent = new RepastHPCDemoAgent(id);
			agent = context.getAgent(id);
			if(agent!=NULL){			
				agents.push_back(agent);
			}
			count++;
		}
	}
	//context.selectAgents(repast::SharedContext<RepastHPCDemoAgent>::LOCAL, countOfAgents, agents);
	std::vector<RepastHPCDemoAgent*>::iterator it = agents.begin();
	int temp = 0;
	while(it != agents.end()){
        	(*it)->play(&context, discreteSpace); 
		it++;
    	}
	//std::cout << "Rank " << repast::RepastProcess::instance()->rank() << " is doing something: " << repast::Random::instance()->nextDouble() << std::endl;
	//discreteSpace->balance();
    	//repast::RepastProcess::instance()->synchronizeAgentStatus<RepastHPCDemoAgent, RepastHPCDemoAgentPackage, RepastHPCDemoAgentPackageProvider, RepastHPCDemoAgentPackageReceiver>(context, *provider, *receiver, *receiver);
    
    //repast::RepastProcess::instance()->synchronizeProjectionInfo<RepastHPCDemoAgent, RepastHPCDemoAgentPackage, RepastHPCDemoAgentPackageProvider, RepastHPCDemoAgentPackageReceiver>(context, *provider, *receiver, *receiver);

	repast::RepastProcess::instance()->synchronizeAgentStates<RepastHPCDemoAgentPackage, RepastHPCDemoAgentPackageProvider, RepastHPCDemoAgentPackageReceiver>(*provider, *receiver);
    
}

void RepastHPCDemoModel::doasynch(){

	std::vector<RepastHPCDemoAgent*> agents;
	context.selectAgents(repast::SharedContext<RepastHPCDemoAgent>::LOCAL, countOfAgents, agents);
	std::vector<RepastHPCDemoAgent*>::iterator it = agents.begin();
	while(it != agents.end()){
        	(*it)->asynch(&context, discreteSpace); 
		it++;
    	}
	//std::cout << "Rank " << repast::RepastProcess::instance()->rank() << " is doing something: " << repast::Random::instance()->nextDouble() << std::endl;
	//discreteSpace->balance();
    	//repast::RepastProcess::instance()->synchronizeAgentStatus<RepastHPCDemoAgent, RepastHPCDemoAgentPackage, RepastHPCDemoAgentPackageProvider, RepastHPCDemoAgentPackageReceiver>(context, *provider, *receiver, *receiver);
    
    //repast::RepastProcess::instance()->synchronizeProjectionInfo<RepastHPCDemoAgent, RepastHPCDemoAgentPackage, RepastHPCDemoAgentPackageProvider, RepastHPCDemoAgentPackageReceiver>(context, *provider, *receiver, *receiver);

	repast::RepastProcess::instance()->synchronizeAgentStates<RepastHPCDemoAgentPackage, RepastHPCDemoAgentPackageProvider, RepastHPCDemoAgentPackageReceiver>(*provider, *receiver);
    
}

void RepastHPCDemoModel::initSchedule(repast::ScheduleRunner& runner){
	runner.scheduleEvent(0, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::requestAgents)));
	runner.scheduleEvent(1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::doSomething)));
	//runner.scheduleEvent(1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::doasynch)));
	runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::recordResults)));
	runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::recordAgents)));
	runner.scheduleStop(stopAt);
	
	// Data collection
	runner.scheduleEvent(2.5, 5, repast::Schedule::FunctorPtr(new repast::MethodFunctor<repast::DataSet>(agentValues, &repast::DataSet::record)));
	runner.scheduleEvent(10.6, 10, repast::Schedule::FunctorPtr(new repast::MethodFunctor<repast::DataSet>(agentValues, &repast::DataSet::write)));
	runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<repast::DataSet>(agentValues, &repast::DataSet::write)));
}

void RepastHPCDemoModel::recordResults(){
	if(repast::RepastProcess::instance()->rank() == 0){
		props->putProperty("Result","Passed");
		std::vector<std::string> keyOrder;
		keyOrder.push_back("RunNumber");
		keyOrder.push_back("stop.at");
		keyOrder.push_back("Result");
		props->writeToSVFile("/home/ozi/Documents/repasthpc/parallelSIMDemos/output/results.csv", keyOrder);
    }
}

void RepastHPCDemoModel::recordAgents(){
		ofstream myfile;
		std::string s = "./output/example_";
		int rank = repast::RepastProcess::instance()->rank();
		int worldSize = repast::RepastProcess::instance()->worldSize();
		std::string r = props->getProperty("global.random.seed");
		std::string file = s + std::to_string(rank)+"_"+std::to_string(worldSize)+"_"+r+".csv";
      		myfile.open (file);
      		std::vector<RepastHPCDemoAgent*> agents;
		context.selectAgents(repast::SharedContext<RepastHPCDemoAgent>::LOCAL, countOfAgents/worldSize, agents);
		std::vector<RepastHPCDemoAgent*>::iterator it = agents.begin();
		while(it != agents.end()){
			myfile << (*it)->getId();
			myfile << "\t"; 
        		myfile << (*it)->c;
			myfile << "\n"; 
			it++;
    		}
      		myfile.close();
}





	
