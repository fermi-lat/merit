/** @file meritAlg.cxx
    @brief Declaration and implementation of mertAlg

 $Header: /nfs/slac/g/glast/ground/cvs/merit/src/meritAlg/meritAlg.cxx,v 1.36 2003/05/08 15:46:39 burnett Exp $
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
#include "GaudiTuple.h"


#include "Event/MonteCarlo/McParticle.h"
#include "Event/TopLevel/Event.h"
#include "Event/TopLevel/EventModel.h"
#include "Event/TopLevel/MCEvent.h"

#include "AnalysisNtuple/IValsTool.h"

#include "FigureOfMerit.h"
#include "analysis/Tuple.h"
#include "ClassificationTree.h"

#include "GuiSvc/IGuiSvc.h"
#include "gui/DisplayControl.h"
#include "gui/PrintControl.h"
#include "gui/GuiMgr.h"

#include "MeritRootTuple.h"


#include <sstream>
#include <algorithm>
#include <numeric>

static std::string  default_cuts("LnA");

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/** @class meritAlg


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
   

    FigureOfMerit* m_fm;
    Tuple*   m_tuple;
    std::string m_cuts; 
    StringProperty m_root_filename;
    StringProperty m_IM_filename;
    
    MeritRootTuple* m_root_tuple;

    IToolSvc* m_pToolSvc;

    // places to put stuff found in the TDS
    double m_event, m_mc_src_id;
    double m_time;

    int m_generated;

    /// Common interface to analysis tools
    std::vector<IValsTool*> m_toolvec;

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static const AlgFactory<meritAlg>  Factory;
const IAlgFactory& meritAlgFactory = Factory;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
meritAlg::meritAlg(const std::string& name, ISvcLocator* pSvcLocator) :
Algorithm(name, pSvcLocator), m_tuple(0), m_root_tuple(0) {
    
    declareProperty("cuts" , m_cuts=default_cuts);
    declareProperty("generated" , m_generated=10000);
    declareProperty("RootFilename", m_root_filename="");
    declareProperty("IM_filename", m_IM_filename="/common/IM_files/PSF_Analysis_12.imw");
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
    log << MSG::INFO << "initialize" << endreq;
    
    // Use the Job options service to get the Algorithm's parameters
    setProperties();
    

    // setup the pseudo-tuple
    std::stringstream title;
    title <<  "TDS: gen(" << m_generated <<  ")";
    m_tuple = new Tuple(title.str());

   // define tuple items
    new TupleItem("Event_ID",       &m_event);
    new TupleItem("MC_src_Id",      &m_mc_src_id);
    new TupleItem("elapsed_time",   &m_time);

    if( setupTools().isFailure()) return StatusCode::FAILURE;

     //now make the parallel ROOT tuple
    if(!m_root_filename.value().empty() ){
        log << MSG::INFO << "Opening " << m_root_filename << " to write ROOT tuple" << endreq;
        m_root_tuple=new MeritRootTuple(m_tuple, m_root_filename);
    }
    // the tuple is made: create the classification object 
    try { 
        ClassificationTree ctree(*m_tuple, m_IM_filename);
    //TODO: finish setup.
    }catch ( classification::Tree::Exception e){
        log << MSG::ERROR << "Classification tree error, ";
            e.printOn(log.stream());
                log << endreq;
    }catch (...)  {
        log << MSG::ERROR << "Unexpected exception creating classification trees" << endreq;
    }

    m_fm= new FigureOfMerit(*m_tuple, m_cuts);
    
    // setup tuple output via the print service
        // get the Gui service
    IGuiSvc* guiSvc=0;
    sc = service("GuiSvc", guiSvc);

    if (!sc.isSuccess ()){
        log << MSG::INFO << "No GuiSvc, so no interactive printout" << endreq;
        return StatusCode::SUCCESS;
    }

    gui::GuiMgr* guimgr = guiSvc->guiMgr();
    gui::PrintControl* printer = &guimgr->printer();
    printer->addPrinter("merit tuple",new gui::Printer_T<meritAlg>(this));


    return sc;
}
//------------------------------------------------------------------------------
void meritAlg::calculate(){

    for( std::vector<IValsTool*>::iterator i =m_toolvec.begin(); i != m_toolvec.end(); ++i){
        (*i)->doCalcIfNotDone();
    }

}
//------------------------------------------------------------------------------
void meritAlg::printOn(std::ostream& out)const{
    out << "Merit tuple, " << "$Revision: 1.36 $" << std::endl;

    for(Tuple::const_iterator tit =m_tuple->begin(); tit != m_tuple->end(); ++tit){
        const TupleItem& item = **tit;
        out << std::setw(25) << item.name() 
            << "  " << std::setprecision(4)<< double(item) << std::endl;
    }
}

//------------------------------------------------------------------------------
StatusCode meritAlg::execute() {
    
    StatusCode  sc = StatusCode::SUCCESS;
  
    calculate(); // setup Bill's tuple items
    SmartDataPtr<Event::EventHeader>   header(eventSvc(),    EventModel::EventHeader);
    SmartDataPtr<Event::MCEvent>     mcheader(eventSvc(),    EventModel::MC::Event);

    m_mc_src_id = mcheader->getSourceId();
    m_event = mcheader->getSequence();
    m_time = header->time();
    m_event = header->event();

    if(m_root_tuple)m_root_tuple->fill();

    // TODO: process classification trees.
    m_fm->execute();
    
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
    if(m_root_tuple !=0) {
        log << MSG::INFO << "Wrote " << m_root_tuple->entries() << " ROOT tuple entries" << endreq;
        delete m_root_tuple;
    }

    
    delete m_fm;
    return StatusCode::SUCCESS;
}

