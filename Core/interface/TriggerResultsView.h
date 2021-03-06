#ifndef TriggerResultsView_h
#define TriggerResultsView_h

#include "CommonFSQFramework/Core/interface/EventViewBase.h"

class TriggerResultsView: public EventViewBase{
    public:
       TriggerResultsView(const edm::ParameterSet& ps, TTree * tree);
       void doBeginRun(const edm::Run&, const edm::EventSetup&);

    private:
      virtual void fillSpecific(const edm::Event&, const edm::EventSetup&);
      std::string m_process;
      std::vector<std::string > m_triggerNames;
      std::map<std::string, std::vector<std::string > > m_triggerClasses;



};
#endif
