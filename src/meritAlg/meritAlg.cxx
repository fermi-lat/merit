/** @file meritAlg.cxx
    @brief Declaration and implementation of meritAlg

 $Header: /nfs/slac/g/glast/ground/cvs/merit/src/meritAlg/meritAlg.cxx,v 1.51 2003/08/25 23:38:47 burnett Exp $
*/
// Include files

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/AlgTool.h"


#include "Event/MonteCarlo/McParticle.h"
#include "Event/TopLevel/Event.h"
#include "Event/TopLevel/EventModel.h"
#include "Event/TopLevel/MCEvent.h"
#include "Event/MonteCarlo/Exposure.h"


#include "AnalysisNtuple/IValsTool.h"

#include "FigureOfMerit.h"
#include "analysis/Tuple.h"
#include "ClassificationTree.h"

#include "facilities/Util.h" // for expandEnvVar    

#include "GuiSvc/IGuiSvc.h"
#include "gui/DisplayControl.h"
#include "gui/PrintControl.h"
#include "gui/GuiMgr.h"

#include "ntupleWriterSvc/INTupleWriterSvc.h"

#include "OnboardFilter/FilterStatus.h"

#include <sstream>
#include <algorithm>
#include <numeric>
#include <cassert>

static std::string  default_cuts("LntA");

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/** @class meritAlg
@brief Apply the merit processing as an Algorithm, constructing a NTuple

  */
class meritAlg : public Algorithm {
    
public:
    meritAlg(const std::string& name, ISvcLocator* pSvcLocator); 
    
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();

    void printOn(std::ostream& out)const;
private:
   
    StatusCode setupTools();

    void calculate(); 
    void setupPointingInfo();
    void copyPointingInfo();

    FigureOfMerit* m_fm;
    Tuple*   m_tuple;
    std::string m_cuts; 
    StringProperty m_root_filename;
    StringProperty m_IM_filename;
    INTupleWriterSvc* m_rootTupleSvc;;

    IToolSvc* m_pToolSvc;

    // places to put stuff found in the TDS
    double m_run, m_event, m_mc_src_id;
    double m_time;
    double m_statusHi, m_statusLo;

    int m_generated;

    /// Common interface to analysis tools
    std::vector<IValsTool*> m_toolvec;

    /// classification
    ClassificationTree* m_ctree;

    /// the event tuple name
    StringProperty m_eventTreeName;
    StringProperty m_pointingTreeName;

    double m_point_info[11];

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static const AlgFactory<meritAlg>  Factory;
const IAlgFactory& meritAlgFactory = Factory;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
meritAlg::meritAlg(const std::string& name, ISvcLocator* pSvcLocator) :
Algorithm(name, pSvcLocator), m_tuple(0)
, m_rootTupleSvc(0)
{
    
    declareProperty("cuts" , m_cuts=default_cuts);
    declareProperty("generated" , m_generated=10000);
    declareProperty("RootFilename", m_root_filename="");
    declareProperty("EventTreeName",     m_eventTreeName="MeritTuple");
    declareProperty("PointingTreeName", m_pointingTreeName="Exposure");
    declareProperty("IM_filename", m_IM_filename="$(CLASSIFICATIONROOT)/xml/PSF_Analysis.xml");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
StatusCode meritAlg::setupTools() {
    MsgStream log(msgSvc(), name());
   // set up tools
    IToolSvc* pToolSvc = 0;
    
    StatusCode sc = service("ToolSvc", pToolSvc, true);
    if (!sc.isSuccess ()){
        log << MSG::ERROR << "Can't find ToolSvc, will quit now" << endreq;
        return StatusCode::FAILURE;
    }
    
    const char * toolnames[] = {"McValsTool", "GltValsTool", "TkrValsTool", 
        "VtxValsTool", "CalValsTool", "AcdValsTool", "EvtValsTool"};
    
    for( int i =0; i< sizeof(toolnames)/sizeof(void*); ++i){
        m_toolvec.push_back(0);
        sc = pToolSvc->retrieveTool(toolnames[i], m_toolvec.back());
        if( sc.isFailure() ) {
            log << MSG::ERROR << "Unable to find a  tool" << toolnames[i] << endreq;
            return sc;
        }
    }

    //grab Bill's tuples
    class VisitBill : virtual public IValsTool::Visitor
    {
    public:
        VisitBill( meritAlg* me) 
            : m_merit(me){}
        IValsTool::Visitor::eVisitorRet analysisValue(std::string varName, const double& value) const
        {
         //   std::cout << "Creating tupleitem  from AnalysisNtuple value " << varName << std::endl;
            double * val = const_cast<double*>(&value);
            new TupleItem(varName, val);
            return IValsTool::Visitor::CONT;
        }
        
    private:
        meritAlg* m_merit;
    };

    VisitBill visitor(this);
    for( std::vector<IValsTool*>::iterator it =m_toolvec.begin(); it != m_toolvec.end(); ++it){
        if( (*it)->traverse(&visitor,false)==IValsTool::Visitor::ERROR) {
            log << MSG::ERROR << *it << " traversal failed" << endreq;
            return StatusCode::FAILURE;
        }
    }
    return StatusCode::SUCCESS;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
StatusCode meritAlg::initialize() {
    StatusCode  sc = StatusCode::SUCCESS;
    
    MsgStream log(msgSvc(), name());
   
    // Use the Job options service to get the Algorithm's parameters
    setProperties();
    
    if(m_root_filename.value().empty()){
        log << MSG::WARNING << "file name was set, but is not used now" << endreq;
    }
    // setup the pseudo-tuple
    std::stringstream title;
    title <<  "TDS: gen(" << m_generated <<  ")";
    m_tuple = new Tuple(title.str());

   // define tuple items
    new TupleItem("Run",            &m_run);
    new TupleItem("Event_ID",       &m_event);
    new TupleItem("MC_src_Id",      &m_mc_src_id);
    new TupleItem("elapsed_time",   &m_time);
    new TupleItem("FilterStatus_HI",&m_statusHi);
    new TupleItem("FilterStatus_LO",&m_statusLo);

    // add some of the AnalysisNTuple items
    if( setupTools().isFailure()) return StatusCode::FAILURE;

    // the tuple is made: create the classification object 
    try { 
        const char * pkgpath = ::getenv("CLASSIFICATIONROOT");
        std::string path =  m_IM_filename.value(); 
        facilities::Util::expandEnvVar(&path);
        m_ctree = new  ClassificationTree(*m_tuple, log.stream(), path);
    //TODO: finish setup.
    }catch ( std::exception& e){
        log << MSG::ERROR << "Exception caught, class  "<< typeid( e ).name( ) << ", message:"
              << e.what() <<endreq;
    }catch (...)  {
        log << MSG::ERROR << "Unexpected exception creating classification trees" << endreq;
    }

    //now make the parallel ROOT tuple--the treename must be set
    if(!m_eventTreeName.value().empty() ){
        // get a pointer to RootTupleSvc 
        if( (sc = service("RootTupleSvc", m_rootTupleSvc)). isFailure() ) {
            log << MSG::ERROR << " failed to get the RootTupleSvc" << endreq;
            return sc;
        }

    }

    m_fm= new FigureOfMerit(*m_tuple, m_cuts);
   
    if( m_rootTupleSvc !=0) {
         // now tell the root tuple service about our tuple
        for(Tuple::iterator tit =m_tuple->begin(); tit != m_tuple->end(); ++tit){
            TupleItem& item = **tit;
            m_rootTupleSvc->addItem(m_eventTreeName.value(), item.name(), &item.value());
        }

        // and also the pointing branch
        setupPointingInfo();
    }



    // setup tuple output via the print service
        // get the Gui service
    IGuiSvc* guiSvc=0;
    sc = service("GuiSvc", guiSvc);

    
    if (!sc.isSuccess ()){
        log << MSG::DEBUG << "No GuiSvc, so no interactive printout" << endreq;
        return StatusCode::SUCCESS;
    }

    gui::GuiMgr* guimgr = guiSvc->guiMgr();
    gui::PrintControl* printer = &guimgr->printer();
    printer->addPrinter("merit tuple",new gui::Printer_T<meritAlg>(this));


    return sc;
}
//------------------------------------------------------------------------------
void meritAlg::setupPointingInfo(){

    std::string treeName= m_pointingTreeName.value();
    if( treeName.empty()) return;
    const char * point_info_name[] = {"time","lat","lon","alt","posx","posy","posz","rax","decx","raz","decz"};
    for( int i = 0; i< sizeof(m_point_info)/sizeof(double); ++i){
        m_rootTupleSvc->addItem(std::string(m_pointingTreeName), point_info_name[i], m_point_info+i);
    }
}
//------------------------------------------------------------------------------
void meritAlg::copyPointingInfo(){

        Event::ExposureCol* elist = 0;
        eventSvc()->retrieveObject("/Event/MC/ExposureCol",(DataObject *&)elist);
        //Event::ExposureCol::iterator curEntry = (*elist).begin();
        const Event::Exposure& exp = **(*elist).begin();
        int n= 0;
        m_point_info[n++]=exp.intrvalstart();
        m_point_info[n++]=exp.lat();
        m_point_info[n++]=exp.lon();
        m_point_info[n++]=exp.alt();
        m_point_info[n++]=exp.posX();
        m_point_info[n++]=exp.posY();
        m_point_info[n++]=exp.posZ();
        m_point_info[n++]=exp.RAX();
        m_point_info[n++]=exp.DECX();
        m_point_info[n++]=exp.RAZ();
        m_point_info[n++]=exp.DECZ();
        assert( n==sizeof(m_point_info)/sizeof(double));
}
//------------------------------------------------------------------------------
void meritAlg::calculate(){

    for( std::vector<IValsTool*>::iterator i =m_toolvec.begin(); i != m_toolvec.end(); ++i){
        (*i)->doCalcIfNotDone();
    }

}
//------------------------------------------------------------------------------
void meritAlg::printOn(std::ostream& out)const{
    out << "Merit tuple, " << "$Revision: 1.51 $" << std::endl;

    for(Tuple::const_iterator tit =m_tuple->begin(); tit != m_tuple->end(); ++tit){
        const TupleItem& item = **tit;
        out << std::setw(25) << item.name() 
            << "  " << std::setprecision(4)<< double(item) << std::endl;
    }
}

//------------------------------------------------------------------------------
StatusCode meritAlg::execute() {
    
    StatusCode  sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());
  
    calculate(); // setup Bill's tuple items
    SmartDataPtr<Event::EventHeader>   header(eventSvc(),    EventModel::EventHeader);
    SmartDataPtr<Event::MCEvent>     mcheader(eventSvc(),    EventModel::MC::Event);

    m_run = header->run();
    m_mc_src_id = mcheader->getSourceId();
    m_event = mcheader->getSequence();
    m_time = header->time();
    m_event = header->event();

    SmartDataPtr<OnboardFilterTds::FilterStatus> filterStatus(eventSvc(), "/Event/Filter/FilterStatus");
    if( filterStatus ){
        m_statusHi=filterStatus->getHigh();
        m_statusLo=filterStatus->getLow();
    }else{
        m_statusHi=m_statusLo=0;
        log << MSG::ERROR << "did not find the filterstatus" << endreq;
    }
    m_ctree->execute();
    m_fm->execute();
    if( m_rootTupleSvc) {
            copyPointingInfo();
            m_rootTupleSvc->storeRowFlag(true);
    }
    
    return sc;
}
//------------------------------------------------------------------------------
StatusCode meritAlg::finalize() {
    
    MsgStream log(msgSvc(), name());
    log << MSG::INFO ;
    if(log.isActive()) {
        try {
            m_fm->report(log.stream());
        } catch (...){
            log << "Failure to generate full output due to unknown exception";
        }
    }
    log << endreq;
    delete m_tuple;
    delete m_fm;
    delete m_ctree;
    return StatusCode::SUCCESS;
}

