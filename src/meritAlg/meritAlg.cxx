/** @file meritAlg.cxx
@brief Declaration and implementation of meritAlg

$Header: /nfs/slac/g/glast/ground/cvs/merit/src/meritAlg/meritAlg.cxx,v 1.83 2004/10/11 21:24:58 burnett Exp $
*/
// Include files

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/SmartDataPtr.h"


#include "Event/MonteCarlo/McParticle.h"
#include "Event/TopLevel/Event.h"
#include "Event/TopLevel/EventModel.h"
#include "Event/TopLevel/MCEvent.h"
#include "Event/MonteCarlo/Exposure.h"
#include "LdfEvent/Gem.h"
#include "LdfEvent/EventSummaryData.h"

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
#include "OnboardFilter/FilterAlgTds.h"
#include "astro/PointingTransform.h"

#include <sstream>
#include <algorithm>
#include <map>
#include <numeric>
#include <cassert>

static std::string  default_cuts("LntA");

namespace gui 
{
#include "gui/Command.h"

    /** @class PrinterByPrefix_T
    *    @brief template class for adding a printer to the gui menu that
    *           depends on a prefix (actually any string)
    */
    template<class T> 
    class PrinterByPrefix_T : public Command 
    {
    public:
        PrinterByPrefix_T(const T* t, std::string prefix, std::ostream& out=std::cout)
            : m_t(t),m_out(out), m_prefix(prefix){}
            void execute(){m_t->printOnByPrefix(m_out, m_prefix);}
    private:
        const T* m_t;
        std::ostream& m_out;
        std::string m_prefix;
    };  
}

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
    ///Print the fields in the merit ntuple, that contain the string 'prefix' 
    void printOnByPrefix(std::ostream& out, std::string prefix)const;
private:

    StatusCode setupTools();

    //! add an item to the MeritTuple

    void addItem(const std::string & itemName,  double* pdouble)
    {
        new TupleItem(itemName, pdouble);

    }
    void addItem(const std::string & itemName,  float* pfloat)
    {
        new TupleItem(itemName, pfloat);
    }
    void addItem(const std::string & itemName,  int* pint)
    {
    }

    void calculate(); 

    /** nested class to manage extra TTrees 
    */
    class TTree { 
    public:
        /**
        */
        TTree(     INTupleWriterSvc* rootTupleSvc, 
            std::string treeName,  
            std::vector<const char* >leaf_names)
            :m_name(treeName), m_leafNames(leaf_names)
        {
            m_values.resize(leaf_names.size());
            int i=0;
            for( std::vector<const char*>::const_iterator  it = leaf_names.begin();
                it!=leaf_names.end(); 
                ++it){
                    rootTupleSvc->addItem(treeName,  *it,  &m_values[i++]);
                }
        }
        void fill(int n, float value){ m_values[n]=value;};

        /** called from gui printer */
        void printOn(std::ostream& out)const {
            out << "Tree "<< m_name << std::endl;
            std::vector<float>::const_iterator dit=m_values.begin();
            for( std::vector<const char*> ::const_iterator nit=m_leafNames.begin(); nit!=m_leafNames.end(); ++nit,++dit){
                out << std::setw(15) << *nit << "  " <<  *dit << std::endl;
            }
        }
        /// data values: name of tree, leaves, and the values
        std::string m_name;
        std::vector<const char*> m_leafNames;
        std::vector< float> m_values;
    };
    //-----------------------------------------------
    /// TTree objects to manage the pointing and FT1 tuples
    TTree* m_pointingTuple;
    void setupPointingInfo();
    void copyPointingInfo();
    void copyFT1Info();

    ///Helper functions to get RA, DEC, ZENITH_THETA and EARTH_AZIMUTH 
    ///for a given direction in GLAST referential
    std::map<std::string, double> meritAlg::getCelestialCoords(const Hep3Vector glastDir);

    FigureOfMerit* m_fm;
    Tuple*   m_tuple;
    std::string m_cuts; 
    StringProperty m_IM_filename;
    INTupleWriterSvc* m_rootTupleSvc;;

    IToolSvc* m_pToolSvc;

    // places to put stuff found in the TDS
    float m_run, m_event, m_mc_src_id;

    double m_time;
    float m_livetime;
    double m_statusHi, m_statusLo,m_separation;
    double m_filterAlgStatus;


    int m_generated;
    int m_warnNoFilterStatus;   // count WARNINGs: no FilterStatus found

    //FT1 entries
    float m_ft1eventid;
    float m_ft1energy;
    float m_ft1theta,m_ft1phi,m_ft1ra,m_ft1dec,m_ft1zen,m_ft1azim;
    float m_ft1convpointx,m_ft1convpointy,m_ft1convpointz,m_ft1convlayer;

    /// Common interface to analysis tools
    std::vector<IValsTool*> m_toolvec;

    /// classification
    ClassificationTree* m_ctree;

    /// the various tree names
    StringProperty m_eventTreeName;
    StringProperty m_pointingTreeName;
    StringProperty m_primaryType;
    long           m_nbOfEvtsInFile;

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static const AlgFactory<meritAlg>  Factory;
const IAlgFactory& meritAlgFactory = Factory;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
meritAlg::meritAlg(const std::string& name, ISvcLocator* pSvcLocator) :
Algorithm(name, pSvcLocator), m_tuple(0), m_rootTupleSvc(0)
{

    declareProperty("cuts" , m_cuts=default_cuts);
    declareProperty("generated" , m_generated=10000);
    declareProperty("EventTreeName",     m_eventTreeName="MeritTuple");
    declareProperty("PointingTreeName", m_pointingTreeName="Exposure");
    declareProperty("IM_filename", m_IM_filename="$(MERITROOT)/xml/classification.imw");
    declareProperty("PrimaryType", m_primaryType="RECO"); // or "MC" (why not a bool?)
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
    //------------------------
    // nested vistor class to put Bill's tuple values into our tuple
    class VisitBill : virtual public IValsTool::Visitor
    {
    public:
        VisitBill( meritAlg* me) : m_merit(me){}
        IValsTool::Visitor::eVisitorRet analysisValue(std::string varName, const double& value) const
        {
            double * val = const_cast<double*>(&value);
            m_merit->addItem( varName, val);
            return IValsTool::Visitor::CONT;
        }
      IValsTool::Visitor::eVisitorRet analysisValue(std::string varName, const float& value) const
        {
            float * val = const_cast<float*>(&value);
            m_merit->addItem( varName, val);
            return IValsTool::Visitor::CONT;
        }
      IValsTool::Visitor::eVisitorRet analysisValue(std::string varName, const int& value) const
        {
#if 1 // not yet implemented
            return IValsTool::Visitor::ERROR;
#else
            int * val = const_cast<int*>(&value);
            m_merit->addItem( varName, val);
            return IValsTool::Visitor::CONT;
#endif
        }

    private:
        meritAlg* m_merit;
    };
    //----------------------

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

    // get a pointer to RootTupleSvc 
    if( (sc = service("RootTupleSvc", m_rootTupleSvc, true) ). isFailure() ) {
        log << MSG::ERROR << " failed to get the RootTupleSvc" << endreq;
        return sc;
    }
    // set up the local tuple
    std::stringstream title;
    title << "TDS: gen(" << m_generated << ")";
    m_tuple = new Tuple(title.str());

    addItem( "FilterStatus_HI",   &m_statusHi );
    addItem( "FilterStatus_LO",   &m_statusLo );

    addItem( "FilterAlgStatus",  &m_filterAlgStatus );
    addItem( "FilterAngSep",     &m_separation );

    //FT1 INFO:
    addItem( "FT1EventId",          &m_ft1eventid);
    addItem( "FT1Energy",           &m_ft1energy);
    addItem( "FT1Theta",            &m_ft1theta);
    addItem( "FT1Phi",              &m_ft1phi);
    addItem( "FT1Ra",               &m_ft1ra);
    addItem( "FT1Dec",              &m_ft1dec);
    addItem( "FT1ZenithTheta",      &m_ft1zen);
    addItem( "FT1EarthAzimuth",     &m_ft1azim);
    addItem( "FT1ConvPointX",       &m_ft1convpointx);
    addItem( "FT1ConvPointY",       &m_ft1convpointy);
    addItem( "FT1ConvPointZ",       &m_ft1convpointz);

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



    m_fm= new FigureOfMerit(*m_tuple, m_cuts);

    if( m_rootTupleSvc !=0) {
        // now tell the root tuple service about our tuple
        for(Tuple::iterator tit =m_tuple->begin(); tit != m_tuple->end(); ++tit){
            TupleItem& item = **tit;
            if( item.isFloat() ){
                m_rootTupleSvc->addItem(m_eventTreeName.value(), item.name(), item.pvalue()) ;
            }else {
                m_rootTupleSvc->addItem(m_eventTreeName.value(), item.name(), &item.value());
            }
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

    printer->addPrinter("Acd tree", new gui::PrinterByPrefix_T<meritAlg>(this,"Acd"));
    printer->addPrinter("Cal tree", new gui::PrinterByPrefix_T<meritAlg>(this,"Cal"));
    printer->addPrinter("Tkr tree", new gui::PrinterByPrefix_T<meritAlg>(this,"Tkr"));
    printer->addPrinter("Vtx tree", new gui::PrinterByPrefix_T<meritAlg>(this,"Vtx"));
    printer->addPrinter("Evt tree", new gui::PrinterByPrefix_T<meritAlg>(this,"Evt"));
    printer->addPrinter("Mc  tree", new gui::PrinterByPrefix_T<meritAlg>(this,"Mc"));
    printer->addPrinter("Glt tree", new gui::PrinterByPrefix_T<meritAlg>(this,"Glt"));
    printer->addPrinter("IM  tree", new gui::PrinterByPrefix_T<meritAlg>(this,"IM"));
    printer->addPrinter("FT1 tree", new gui::PrinterByPrefix_T<meritAlg>(this,"FT1"));
    printer->addPrinter("Pt tree", new gui::PrinterByPrefix_T<meritAlg>(this,"Pt"));
    printer->addPrinter("Filter tree", new gui::PrinterByPrefix_T<meritAlg>(this,"Filt"));
    // also for the exposure tree
    printer->addPrinter("Exposure tree", new gui::Printer_T<meritAlg::TTree>(m_pointingTuple));

    return sc;
}
//------------------------------------------------------------------------------
void meritAlg::setupPointingInfo(){

    std::string treeName= m_pointingTreeName.value();
    if( treeName.empty()) return;

    std::vector<const char* > names;
    const char * point_info_name[] = {"PtTime","PtLat","PtLon","PtAlt","PtPosx","PtPosy","PtPosz","PtRax","PtDecx","PtRaz","PtDecz"};
    //const char * point_info_name[] = {"elapsed_time","lat","lon","alt","posx","posy","posz","rax","decx","raz","decz"};
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

void meritAlg::copyFT1Info(){


    MsgStream log(msgSvc(), name());

    //eventId and Time are always defined
    m_ft1eventid = m_run * m_nbOfEvtsInFile + m_event;

    // Give default "guard" values in case there are no tracks in the event
    m_ft1energy = -1.;
    m_ft1theta = 666; m_ft1phi = 666; m_ft1ra   = 666;
    m_ft1dec   = 666; m_ft1zen = 666; m_ft1azim = 666;
    m_ft1convpointx = 999; m_ft1convpointy = 999; m_ft1convpointz = 999; 
    m_ft1convlayer = -1;

    Hep3Vector convPoint;
    Hep3Vector glastDir;

    if(m_primaryType.value() == "MC")
    {
        m_ft1energy    = m_tuple->tupleItem("McEnergy")->value(); 
        glastDir = Hep3Vector(m_tuple->tupleItem("McXDir")->value(),
            m_tuple->tupleItem("McYDir")->value(),
            m_tuple->tupleItem("McZDir")->value());
        m_ft1convpointx  = m_tuple->tupleItem("McX0")->value();
        m_ft1convpointy  = m_tuple->tupleItem("McY0")->value();
        m_ft1convpointz  = m_tuple->tupleItem("McZ0")->value();
        m_ft1convlayer = -1;
    }
    else if(m_primaryType.value() == "RECO")
    {
        if(m_ctree->useVertex())
        {
            // Retrieve Vertex to get summary info from reco
            //	  m_ft1energy    = m_tuple->tupleItem("TkrSumConEne")->value();
            m_ft1energy    = m_tuple->tupleItem("EvtEnergySumOpt")->value();
            glastDir = Hep3Vector(m_tuple->tupleItem("VtxXDir")->value(),
                m_tuple->tupleItem("VtxYDir")->value(),
                m_tuple->tupleItem("VtxZDir")->value());
            m_ft1convpointx  = m_tuple->tupleItem("VtxX0")->value();
            m_ft1convpointy  = m_tuple->tupleItem("VtxY0")->value();
            m_ft1convpointz  = m_tuple->tupleItem("VtxZ0")->value();
        }
        else
        {
            //	  m_ft1energy    = m_tuple->tupleItem("Tkr1ConEne")->value();
            m_ft1energy    = m_tuple->tupleItem("EvtEnergySumOpt")->value();
            glastDir = Hep3Vector(m_tuple->tupleItem("Tkr1XDir")->value(),
                m_tuple->tupleItem("Tkr1YDir")->value(),
                m_tuple->tupleItem("Tkr1ZDir")->value());
            m_ft1convpointx  = m_tuple->tupleItem("Tkr1X0")->value();
            m_ft1convpointy  = m_tuple->tupleItem("Tkr1Y0")->value();
            m_ft1convpointz  = m_tuple->tupleItem("Tkr1Z0")->value();
        }
        m_ft1convlayer   = m_tuple->tupleItem("Tkr1FirstLayer")->value();
    }

    glastDir = - glastDir.unit();

    // celestial coords in degree
    std::map<std::string,double> cel_coords = getCelestialCoords(glastDir);
    m_ft1ra   = cel_coords["RA"];
    m_ft1dec  = cel_coords["DEC"];
    m_ft1zen  = cel_coords["ZENITH_THETA"];
    m_ft1azim = cel_coords["EARTH_AZIMUTH"];

    // instrument coords in degree
    m_ft1theta = glastDir.theta()*180/M_PI;
    double phi_deg = glastDir.phi(); 
    if( phi_deg<0 ) phi_deg += 2*M_PI;
    m_ft1phi =  phi_deg*180/M_PI;



}
//------------------------------------------------------------------------------
void meritAlg::calculate(){

    for( std::vector<IValsTool*>::iterator i =m_toolvec.begin(); i != m_toolvec.end(); ++i){
        (*i)->doCalcIfNotDone();
    }
}
//------------------------------------------------------------------------------
void meritAlg::printOn(std::ostream& out)const{
    out << "Merit tuple, " << "$Revision: 1.83 $" << std::endl;

    for(Tuple::const_iterator tit =m_tuple->begin(); tit != m_tuple->end(); ++tit){
        const TupleItem& item = **tit;
        out << std::setw(25) << item.name() 
            << "  " << std::setprecision(4)<< double(item) << std::endl;
    }
}

//------------------------------------------------------------------------------
void meritAlg::printOnByPrefix(std::ostream& out, std::string prefix)const{
    out << "Merit tuple/Prefix= "<<prefix<< std::endl;

    for(Tuple::const_iterator tit =m_tuple->begin(); tit != m_tuple->end(); ++tit){
        const TupleItem& item = **tit;
        if(!item.name().find(prefix))
        {
            out << std::setw(25) << item.name() 
                << "  " << std::setprecision(4)<< double(item) << std::endl;
        }
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
    if( mcheader )    m_mc_src_id = mcheader->getSourceId();
    m_time = header->time();
    m_event = header->event();
    m_livetime = header->livetime();

    SmartDataPtr<OnboardFilterTds::FilterStatus> filterStatus(eventSvc(), "/Event/Filter/FilterStatus");
    if( filterStatus ){
        m_statusHi=filterStatus->getHigh();
        m_statusLo=filterStatus->getLow();
        m_separation=filterStatus->getSeparation();
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
    SmartDataPtr<FilterAlgTds::FilterAlgData> filterAlgStatus(eventSvc(),"/Event/Filter/FilterAlgData");
    if(filterAlgStatus){
        m_filterAlgStatus=(double)filterAlgStatus->getVetoWord();
    }
 
    m_ctree->execute();
    m_fm->execute();

    // always write the event tuple
    m_rootTupleSvc->storeRowFlag(this->m_eventTreeName.value(), true);
    // write out the FT1 and pointing  tuples only for reconstructed
    copyFT1Info();
#if 0 // for Julie: so she gets all events as friends.
    double track_count = m_tuple->tupleItem("TkrNumTracks")->value();
    if( m_rootTupleSvc && track_count>0 ) {
#endif
        copyPointingInfo();
        m_rootTupleSvc->storeRowFlag(this->m_pointingTreeName, true);
#if 0 //see above
    }
#endif

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
    if(m_warnNoFilterStatus>0)
        log << MSG::INFO << "Number of warnings (FilterStatus not found): "<< m_warnNoFilterStatus << endreq;

    delete m_tuple;
    delete m_fm;
    delete m_ctree;
    setFinalized(); //  prevent being called again

    return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
std::map<std::string, double> 
meritAlg::getCelestialCoords(const Hep3Vector glastDir)
{
    using namespace astro;

    std::map<std::string, double> fields;

    //First get the coordinates from the ExposureCol
    Event::ExposureCol* elist = 0;
    eventSvc()->retrieveObject("/Event/MC/ExposureCol",(DataObject *&)elist);
    if( elist==0) return fields; // should not happen, but make sure ok.
    const Event::Exposure& exp = **(*elist).begin();

    // create a transformation object -- first get local directions
    SkyDir zsky( exp.RAZ(), exp.DECZ() );
    SkyDir xsky( exp.RAX(), exp.DECX() );
    // orthogonalize, since interpolation and transformations destory orthogonality (limit is 10E-8)
    Hep3Vector xhat = xsky() -  xsky().dot(zsky()) * zsky() ;
    PointingTransform toSky( zsky, xhat );

    // make zenith (except for oblateness correction) unit vector
    Hep3Vector position( exp.posX(),  exp.posY(),  exp.posZ() );
    SkyDir zenith(position.unit());

    SkyDir sdir = toSky.gDir(glastDir);

    //zenith_theta and earth_azimuth
    double zenith_theta = sdir.difference(zenith); 
    if( fabs(zenith_theta)<1e-8) zenith_theta=0;

    SkyDir north_dir(90,0);
    SkyDir east_dir( north_dir().cross(zenith()) );
    double earth_azimuth=atan2( sdir().dot(east_dir()), sdir().dot(north_dir()) );
    if( earth_azimuth <0) earth_azimuth += 2*M_PI; // to 0-360 deg.
    if( fabs(earth_azimuth)<1e-8) earth_azimuth=0;

    fields["RA"]            = sdir.ra();
    fields["DEC"]           = sdir.dec();
    fields["ZENITH_THETA"]  = zenith_theta*180/M_PI;
    fields["EARTH_AZIMUTH"] = earth_azimuth*180/M_PI;
    return fields;
}



