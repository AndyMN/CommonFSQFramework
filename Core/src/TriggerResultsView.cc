#include "CommonFSQFramework/Core/interface/TriggerResultsView.h"
#include "FWCore/Common/interface/TriggerResultsByName.h"
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerReadoutRecord.h"

TriggerResultsView::TriggerResultsView(const edm::ParameterSet& iConfig, TTree * tree):
EventViewBase(iConfig,  tree)
{
    // fetch config data
    m_process = iConfig.getParameter<std::string>("process");
    m_triggerNames = iConfig.getParameter<std::vector<std::string> >("triggers");
    for (unsigned int i=0; i < m_triggerNames.size();++i){
        // check, if it's for L1GT readout
        if (m_triggerNames.at(i).find("L1GT") != std::string::npos) {
            m_triggerClasses[m_triggerNames.at(i)] = std::vector<std::string>();
        } else if (iConfig.exists(m_triggerNames.at(i))) {
            std::vector<std::string> triggerClass =  iConfig.getParameter<std::vector<std::string> >(m_triggerNames.at(i));
            m_triggerClasses[m_triggerNames.at(i)] = triggerClass;
            //std::cout << "Trigger created via pset: " << m_triggerNames.at(i) << std::endl; 

        } else {
            // TODO: check, that '*' is at the end only
            std::string keyName = m_triggerNames.at(i);
            if (m_triggerNames.at(i).at(m_triggerNames.at(i).size()-1 ) == '*'){
                //std::cout << "Star: " << m_triggerNames.at(i) << std::endl;
                // TODO: remove trailing "_" ???
                keyName = std::string(m_triggerNames.at(i), 0, m_triggerNames.at(i).size()-1);
            } else {
                //std::cout << "Normal: " << m_triggerNames.at(i) << std::endl;
            }
            std::vector<std::string > vec;
            vec.push_back( m_triggerNames.at(i));
            m_triggerClasses[keyName]= vec;
        }
    }

    // register branches
    std::map<std::string, std::vector<std::string> >::const_iterator it, itE;
    it = m_triggerClasses.begin();
    itE = m_triggerClasses.end();
    for(;it != itE;++it){
        if (it->first.find("L1GT") != std::string::npos) {
            registerVecInt(it->first, tree);
        } else {
            registerInt(it->first, tree);
        }
    }
}

void TriggerResultsView::doBeginRun(const edm::Run& r, const edm::EventSetup& es) {
    
    //std::cout << "This is executed during doBeginRun() in the TriggerResultsView" << std::endl;
    
}


void TriggerResultsView::fillSpecific(const edm::Event& iEvent, const edm::EventSetup& iSetup){

    edm::TriggerResultsByName trbn = iEvent.triggerResultsByName(m_process);
    // TODO error message?
    if (!trbn.isValid()) {
        edm::LogWarning(" TriggerResultsByName ") << " Cannot read TriggerResultsByName " << std::endl;
        return; 
    }

    const std::vector< std::string > names = trbn.triggerNames(); 
    //for (unsigned int i = 0; i<names.size(); ++i){
    //    std::cout << names.at(i) << std::endl;
    //}


    edm::Handle<L1GlobalTriggerReadoutRecord> gtReadoutRecord;
    iEvent.getByLabel(edm::InputTag("gtDigis"), gtReadoutRecord);
    if( !gtReadoutRecord.isValid() || gtReadoutRecord.failedToGet() ) {
        edm::LogWarning(" GTReadoutRecord ") << " Cannot read gtReadoutRecord " << std::endl;
        return;
    };

    TechnicalTriggerWord TechTrigg = gtReadoutRecord->technicalTriggerWord();
    DecisionWord AlgoTrig          = gtReadoutRecord->decisionWord();


    std::map<std::string, std::vector<std::string> >::const_iterator it, itE;
    it = m_triggerClasses.begin();
    itE = m_triggerClasses.end();
    for(;it != itE;++it){
        //it->first  - branch name
        //it->second - list of triggers to check
        //
        //std::cout << "Trying " << it->first << std::endl;

        if (it->first == "L1GTTech") {
            for (unsigned int i=0; i < TechTrigg.size(); ++i) addToIVec(it->first, TechTrigg.at(i));
            continue;
        } else if (it->first == "L1GTAlgo") {
            for (unsigned int i=0; i < AlgoTrig.size(); ++i)  addToIVec(it->first, AlgoTrig.at(i));
            continue;
        }

        int accept = 0;
        for (unsigned int i=0; i < it->second.size();++i){
            std::string name = it->second.at(i);
            if (name.find("*")!= std::string::npos){ // wildcard entry
                //std::cout << "TODO:" << it->second.at(i) << std::endl;
                for (unsigned iName = 0; iName < names.size(); ++iName){
                    std::string nameForSearch = std::string(it->second.at(i), 0, it->second.at(i).size()-1); // strip the star
                    if (names.at(iName).find(nameForSearch)==0) { // starts with
                        //std::cout << "Found for start\n";
                        if (trbn.accept(names.at(iName)))  accept = 1;
                    }
                }
            } else { // normal entry
                if (trbn.accept( it->second.at(i))) accept = 1;
            }
            //std::cout << "Accept: " << it->first <<  " "  << accept << std::endl;
            setI(it->first, accept);
        }
    }




}
