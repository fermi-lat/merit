/** @file meritAlg.cxx
    @brief Declaration and implementation of meritAlg

 $Header: /nfs/slac/g/glast/ground/cvs/merit/src/meritAlg/meritAlg.cxx,v 1.58 2003/10/16 22:35:23 burnett Exp $
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
#include "Event/Recon/TkrRecon/TkrVertex.h"


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
#include "astro/PointingTransform.h"

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

    /** nested class to manage extra TTrees 
    */
    class TTree { 
    public:
        /**
        */
        TTree(     INTupleWriterSvc* rootTupleSvc, 
                       std::string treename,  
                       std::vector<const char* >leaf_names);
        void fill(int n, double value){ m_values[n]=value;};
        std::vector< double> m_values;
    private:
    };

    /// TTree objects to manage the pointing and FT1 tuples
    TTree* m_pointingTuple;
    TTree* m_FT1tuple;
    void setupPointingInfo();
    void copyPointingInfo();
    
    void setupFT1info();
    void copyFT1info();
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
    int m_warnNoFilterStatus;   // count WARNINGs: no FilterStatus found

    /// Common interface to analysis tools
    std::vector<IValsTool*> m_toolvec;

    /// classification
    ClassificationTree* m_ctree;

  /// the various tree names
  StringProperty m_eventTreeName;
  StringProperty m_pointingTreeName;
  StringProperty m_FT1TreeName;
  StringProperty m_primaryType;
  long           m_nbOfEvtsInFile;
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
    declareProperty("FT1TreeName", m_FT1TreeName="FT1");
    declareProperty("IM_filename", m_IM_filename="$(CLASSIFICATIONROOT)/xml/PSF_Analysis.xml");
    declareProperty("PrimaryType", m_primaryType="RECO");
    declareProperty("NbOfEvtsInFile", m_nbOfEvtsInFile=100000);

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
    
    for( int i =0; i< (int)(sizeof(toolnames)/sizeof(void*)); ++i){
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
    m_warnNoFilterStatus = 0;   // Zero counter for warnings

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
      //        const char * pkgpath = ::getenv("CLASSIFICATIONROOT");
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
        if( (sc = service("RootTupleSvc", m_rootTupleSvc, true) ). isFailure() ) {
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
        setupFT1info();
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

    std::vector<const char* > names;
    const char * point_info_name[] = {"time","lat","lon","alt","posx","posy","posz","rax","decx","raz","decz"};
    for( int i = 0; i< (int)(sizeof(point_info_name)/sizeof(void*)); ++i){ 
         names.push_back(point_info_name[i]); }
    
    m_pointingTuple = new TTree( m_rootTupleSvc,  std::string(m_pointingTreeName),  names);
}
//------------------------------------------------------------------------------
void meritAlg::copyPointingInfo(){

        Event::ExposureCol* elist = 0;
        eventSvc()->retrieveObject("/Event/MC/ExposureCol",(DataObject *&)elist);
        if( elist==0) return; // should not happen, but make sure ok.
        //Event::ExposureCol::iterator curEntry = (*elist).begin();
        const Event::Exposure& exp = **(*elist).begin();
        int n= 0;
        m_pointingTuple->fill(n++,exp.intrvalstart());
        m_pointingTuple->fill(n++,exp.lat());
        m_pointingTuple->fill(n++,exp.lon());
        m_pointingTuple->fill(n++,exp.alt());
        m_pointingTuple->fill(n++,exp.posX());
        m_pointingTuple->fill(n++,exp.posY());
        m_pointingTuple->fill(n++,exp.posZ());
        m_pointingTuple->fill(n++,exp.RAX());
        m_pointingTuple->fill(n++,exp.DECX());
        m_pointingTuple->fill(n++,exp.RAZ());
        m_pointingTuple->fill(n++,exp.DECZ());
}//------------------------------------------------------------------------------
void meritAlg::setupFT1info(){

    std::string treeName= m_FT1TreeName.value();
    if( treeName.empty()) return;
    std::vector<const char* > names;
    const char * FT1_names[] = {"energy", 
				"ra", 
				"dec", 
				"theta", 
				"phi", 
				"zenith_angle", 
				"earth_azimuth", 
				"time", 
				"event_id",
				"convPointX",
				"convPointY",
				"convPointZ",
				"convLayer"};
    for( int i = 0; i< sizeof(FT1_names)/sizeof(void*); ++i){ names.push_back(FT1_names[i]); }
    
    m_FT1tuple = new TTree( m_rootTupleSvc,  std::string(m_FT1TreeName),  names);
}
//------------------------------------------------------------------------------
void meritAlg::copyFT1info(){

    using namespace astro;

    MsgStream log(msgSvc(), name());

    double energy;
    Hep3Vector convPoint;
    // "glastdir" is the predicted direction to the source, in instrument coords
    Hep3Vector glastDir;
    int convLayer;

    if(m_primaryType.value() == "MC")
      {
	// Temporary: get the MC direction here for verification
	// Recover MC Pointer
	SmartDataPtr<Event::McParticleCol> pMcParticle(eventSvc(), EventModel::MC::McParticleCol);
	Event::McParticleCol::const_iterator pMCPrimary = pMcParticle->begin();
	// Skip the first particle... it's for bookkeeping.
	// The second particle is the first real propagating particle.
	pMCPrimary++;
	HepLorentzVector Mc_p0 = (*pMCPrimary)->initialFourMomentum();
	
	energy    = Mc_p0.t(); 
	glastDir  = - Mc_p0.vect().unit();
	convPoint = (*pMCPrimary)->initialPosition();
	convLayer = 0;
      }
    else if(m_primaryType.value() == "RECO")
      {
	// Retrieve Vertex to get summary info from reco
	Event::TkrVertexCol* pVtxCol = 
	  SmartDataPtr<Event::TkrVertexCol>(eventSvc(), EventModel::TkrRecon::TkrVertexCol);
	if(!pVtxCol)
	  {
	    log << MSG::DEBUG << "Vertex Col not found in TDS" << endreq;
	    return;
	  }
	if(pVtxCol->size()==0) 
	  {
	    log << MSG::DEBUG << "Vertex Col found in TDS BUT empty" << endreq;
	    return;
	  }
	//Assuming the best vertex comes always first.....    
	Event::TkrVertex* pTheBestVtx = pVtxCol->front();
	
	energy    = pTheBestVtx->getEnergy();
	glastDir  = - pTheBestVtx->getDirection().unit();
	convPoint = pTheBestVtx->getPosition();
	convLayer = pTheBestVtx->getLayer();
      }


    //Get Event_ID
    Event::EventHeader* pEvent = 
      SmartDataPtr<Event::EventHeader>(eventSvc(), EventModel::EventHeader);
    if(!pEvent)
      {
	log << MSG::ERROR << "EventHeader not found in TDS" << endreq;
	return;
      }
    long run_id = pEvent->run();
    long event_id = run_id * m_nbOfEvtsInFile + pEvent->event();

    //Now we get the coordinates: frol the ExposureCol
    Event::ExposureCol* elist = 0;
    eventSvc()->retrieveObject("/Event/MC/ExposureCol",(DataObject *&)elist);
    if( elist==0) return; // should not happen, but make sure ok.
    const Event::Exposure& exp = **(*elist).begin();

    // create a transformation object -- first get local directions
    SkyDir zsky(exp.RAZ(), exp.DECZ(), SkyDir::CELESTIAL);
    SkyDir xsky(exp.RAX(), exp.DECX(), SkyDir::CELESTIAL );
    // orthogonalize, since interpolation and transformations destory orthogonality (limit is 10E-8)
    Hep3Vector xhat = xsky() -  xsky().dot(zsky()) * zsky() ;
    PointingTransform toSky( zsky, xhat );
    // make zenith (except for oblateness correction) unit vector
    Hep3Vector zenith( exp.posX(),  exp.posY(), exp.posZ());
    zenith = zenith.unit();
    

    SkyDir sdir = toSky.gDir(glastDir);

    //zenith_theta and earth_azimuth in degree:
    double zenith_theta = acos(zenith.dot(sdir()));
    Hep3Vector north(0.,0.,1.);
    Hep3Vector planar_dir   = sdir() - (zenith.dot(sdir()))*zenith;
    Hep3Vector planar_north = sdir() - (zenith.dot(north))*zenith;
    Hep3Vector planar_east  = planar_north.rotate(M_PI/2.,zenith);
    planar_dir   = planar_dir.unit();
    planar_north = planar_north.unit();
    planar_east  = planar_east.unit();

    Hep3Vector north_to_east = planar_north.cross(planar_east);
    double biplan = planar_north.dot(planar_dir);
    double sine_sign = (planar_north.cross(planar_dir).unit() == north_to_east)?1.0:-1.0;
    double earth_azimuth = planar_north.cross(planar_dir).mag();
    if ( biplan < 0 && sine_sign > 0) //2nd quadrant
      {
	earth_azimuth += M_PI/2.;
      }
    else if (biplan < 0 && sine_sign < 0)//3rd quadrant
      {
	earth_azimuth += M_PI;
      }
    else if (biplan > 0 && sine_sign < 0)//4th quadrant
      {
	earth_azimuth += 3.*M_PI/2.;
      }

    // celestial coords
    m_FT1tuple->fill(0,  energy);
    m_FT1tuple->fill(1,  sdir.ra() ) ;
    m_FT1tuple->fill(2,  sdir.dec() );

    // instrument coords
    m_FT1tuple->fill(3,  glastDir.theta() );

    double phi_deg = glastDir.phi(); 
    if( phi_deg<0 ) phi_deg += 2*M_PI;
    m_FT1tuple->fill(4,  phi_deg);

    // zenith-based coords
    m_FT1tuple->fill(5, zenith_theta);
    m_FT1tuple->fill(6, earth_azimuth);

    // time
    m_FT1tuple->fill(7,  exp.intrvalstart() );

    // event_id
    m_FT1tuple->fill(8,  event_id );

    m_FT1tuple->fill(9,   convPoint.x() );
    m_FT1tuple->fill(10,  convPoint.y() );
    m_FT1tuple->fill(11,  convPoint.z() );
    m_FT1tuple->fill(12,  convLayer     );
}
//------------------------------------------------------------------------------
void meritAlg::calculate(){

    for( std::vector<IValsTool*>::iterator i =m_toolvec.begin(); i != m_toolvec.end(); ++i){
        (*i)->doCalcIfNotDone();
    }
}
//------------------------------------------------------------------------------
void meritAlg::printOn(std::ostream& out)const{
    out << "Merit tuple, " << "$Revision: 1.58 $" << std::endl;

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
    }else {
        m_statusHi = m_statusLo = 0;

        m_warnNoFilterStatus++;
        if (   m_warnNoFilterStatus <= 10 ) {
            log << MSG::WARNING << "FilterStatus not found" ;
          if ( m_warnNoFilterStatus == 10 ) {
            log << " -- Further WARNINGs on missing FilterStatus are suppressed"; }
          log  << endreq;
        }
    }
    m_ctree->execute();
    m_fm->execute();
    if( m_rootTupleSvc) {
            copyPointingInfo();
            copyFT1info();
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
    log << MSG::INFO << "Number of warnings (FilterStatus not found): "<< m_warnNoFilterStatus << endreq;

    delete m_tuple;
    delete m_fm;
    delete m_ctree;
    return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
//                               meritAlg::TTree implementation
//------------------------------------------------------------------------------
meritAlg::TTree::TTree(
                       INTupleWriterSvc* rootTupleSvc, 
                       std::string treeName,  
                       std::vector<const char* >leaf_names)
{
    m_values.resize(leaf_names.size());
    int i=0;
    for( std::vector<const char*>::const_iterator  it = leaf_names.begin();
        it!=leaf_names.end(); 
        ++it){
        rootTupleSvc->addItem(treeName,  *it,  &m_values[i++]);
    }
}
