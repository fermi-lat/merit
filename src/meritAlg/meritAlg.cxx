// $Header: /nfs/slac/g/glast/ground/cvs/merit/src/meritAlg/meritAlg.cxx,v 1.2 2002/05/26 03:35:11 burnett Exp $

// Include files

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiTuple.h"

#include "Event/MonteCarlo/McParticle.h"
#include "Event/TopLevel/Event.h"
#include "Event/TopLevel/EventModel.h"

#include "Event/Recon/TkrRecon/TkrFitTrack.h"
#include "Event/Recon/TkrRecon/TkrFitTrackCol.h"
#include "Event/Recon/TkrRecon/TkrVertex.h"
#include "Event/Recon/TkrRecon/TkrVertexCol.h"
#include "Event/Recon/CalRecon/CalCluster.h"

#include "FigureOfMerit.h"
#include "analysis/Tuple.h"

#include <sstream>

static std::string  default_cuts("nA");

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class meritAlg : public Algorithm {
    
public:
    meritAlg(const std::string& name, ISvcLocator* pSvcLocator); 
    
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();
private:
    void processTDS(
        const Event::McParticleCol& particles, 
        const Event::TkrFitTrackCol& tracks,
        const Event::TkrVertexCol& vertices,
        const Event::CalClusterCol& clusters);

    FigureOfMerit* m_fm;
    Tuple*   m_tuple;
    std::string m_cuts; 
    
    // places to put stuff found in the TDS
    float m_mce, m_trig, 
        m_first_hit, 
        m_angle_diff, 
        m_recon_energy;
    float m_tracks;
    float m_vertices;
    int m_generated;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static const AlgFactory<meritAlg>  Factory;
const IAlgFactory& meritAlgFactory = Factory;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
meritAlg::meritAlg(const std::string& name, ISvcLocator* pSvcLocator) :
Algorithm(name, pSvcLocator), m_tuple(0) {

    declareProperty("cuts" , m_cuts=default_cuts);
    declareProperty("generated" , m_generated=10000);
    declareProperty("cuts" , m_cuts=default_cuts);
}

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
    new TupleItem("MC_Energy",      &m_mce);
    new TupleItem("trig_bits",      &m_trig);
    new TupleItem("TKR_No_Tracks",  &m_tracks);
    new TupleItem("TKR_First_XHit", &m_first_hit);
    new TupleItem("MC_Gamma_Err",   &m_angle_diff);
    new TupleItem("REC_CsI_Corr_Energy", &m_recon_energy);

    m_fm= new FigureOfMerit(*m_tuple, m_cuts);
    
       
    
    return sc;
}


//------------------------------------------------------------------------------
StatusCode meritAlg::execute() {
    
    StatusCode  sc = StatusCode::SUCCESS;

    SmartDataPtr<Event::EventHeader> header(eventSvc(), EventModel::EventHeader);

    SmartDataPtr<Event::McParticleCol> particles(eventSvc(), EventModel::MC::McParticleCol);

    SmartDataPtr<Event::TkrFitTrackCol> tracks(eventSvc(), EventModel::TkrRecon::TkrFitTrackCol);

    SmartDataPtr<Event::TkrVertexCol> vertices(eventSvc(), EventModel::TkrRecon::TkrVertexCol);

    SmartDataPtr<Event::CalClusterCol> clusters(eventSvc(), EventModel::CalRecon::CalClusterCol);

    processTDS( particles, tracks, vertices, clusters);

    m_fm->execute();

    return sc;
}
void meritAlg::processTDS(const Event::McParticleCol& particles, 
                          const Event::TkrFitTrackCol& tracks,
                          const Event::TkrVertexCol& vertices,
                          const Event::CalClusterCol& clusters)
{
    
    // Procedure and Method:  Process the collection of Monte carlo particles
    
    MsgStream   log( msgSvc(), name() );
    
    // assume first mc particle is the primary.   
    const Event::McParticle& primary = **particles.begin();
    m_mce = (primary.initialFourMomentum().e() - primary.initialFourMomentum().mag())*1e-3;
    
    
    // assume first track is what we want. (must const cast)
    Event::TkrFitTrackCol& mytracks = const_cast<Event::TkrFitTrackCol&>(tracks);
    m_tracks = tracks.getNumTracks();

    
    // assume first vertex it what we want. (must const cast)
    Event::TkrVertexCol& myvertices = const_cast<Event::TkrVertexCol&>(vertices);
    m_vertices =  vertices.getNumVertices();

    // process the cluster(s)
    if( clusters.num() >0 ){
        m_recon_energy = clusters.getCluster(0)->getEnergySum()*1e-3; // convert to GeV
    }else {
        m_recon_energy = 0;
    }
    
    if( m_tracks==0) return;

    //
    //-------tracks --------------
    //
    Event::TkrFitCol::const_iterator it = mytracks.getTrackIterBegin();
    
    const Event::TkrFitTrack& track = **it;
    
    const Event::TkrFitPar fitpar=track.getTrackPar();
    //
    //-----vertices ----------
    //
    if( m_vertices==0) return;
//    Event::TkrVertexCol::const_iterator vit = myvertices.getVertexIterBegin();

    const Event::TkrVertex& vertex = *myvertices.getVertex(0);
    Point p = vertex.getPosition();
    Vector dir = vertex.getDirection();
    
    m_first_hit = static_cast<float>(vertex.getLayer());
    // get difference
    m_angle_diff = acos( Hep3Vector(primary.initialFourMomentum()).unit() * dir );
    
}

//------------------------------------------------------------------------------
StatusCode meritAlg::finalize() {
    
    MsgStream log(msgSvc(), name());
    log << MSG::INFO ;

    m_fm->report(log.stream());
    log << endreq;
    delete m_tuple;

    delete m_fm;
    return StatusCode::SUCCESS;
}

