// $Header: /nfs/slac/g/glast/ground/cvs/merit/src/meritAlg/meritAlg.cxx,v 1.8 2002/06/01 15:33:25 burnett Exp $

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

#include "Event/Recon/TkrRecon/TkrVertex.h"
#include "Event/Recon/CalRecon/CalCluster.h"

#include "FigureOfMerit.h"
#include "analysis/Tuple.h"

#ifndef DEFECT_NO_STRINGSTREAM
# include <sstream>
#else
# include <strstream>
#endif

static std::string  default_cuts("LnA");

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class meritAlg : public Algorithm {
    
public:
    meritAlg(const std::string& name, ISvcLocator* pSvcLocator); 
    
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();
private:
    void processTDS(
        const Event::EventHeader & header,

        const Event::McParticleCol& particles, 
        const Event::TkrVertexCol& tracks,
        const Event::CalClusterCol& clusters);

    FigureOfMerit* m_fm;
    Tuple*   m_tuple;
    std::string m_cuts; 
    
    // places to put stuff found in the TDS
    float m_mce, m_trig, m_first_hit, m_angle_diff, m_recon_energy;
    float m_tracks,
        m_time;
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
}

StatusCode meritAlg::initialize() {
    StatusCode  sc = StatusCode::SUCCESS;
    
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "initialize" << endreq;
    
    // Use the Job options service to get the Algorithm's parameters
    setProperties();

    // setup the pseudo-tuple
#ifndef DEFECT_NO_STRINGSTREAM
    std::stringstream title;
    title <<  "TDS: gen(" << m_generated <<  ")";
    m_tuple = new Tuple(title.str());
#else
    std::strstream title;
    title <<  "TDS: gen(" << m_generated <<  ")";
    m_tuple = new Tuple(title.str());
#endif
    // define tuple items
    new TupleItem("MC_Energy",      &m_mce);
    new TupleItem("trig_bits",      &m_trig);
    new TupleItem("TKR_No_Tracks",  &m_tracks);
    new TupleItem("TKR_First_XHit", &m_first_hit);
    new TupleItem("MC_Gamma_Err",   &m_angle_diff);
    new TupleItem("REC_CsI_Corr_Energy", &m_recon_energy);
    new TupleItem("elapsed_time",   &m_time);


    m_fm= new FigureOfMerit(*m_tuple, m_cuts);
    
       
    
    return sc;
}


//------------------------------------------------------------------------------
StatusCode meritAlg::execute() {
    
    StatusCode  sc = StatusCode::SUCCESS;

    SmartDataPtr<Event::EventHeader>   header(eventSvc(),    EventModel::EventHeader);
    SmartDataPtr<Event::McParticleCol> particles(eventSvc(), EventModel::MC::McParticleCol);
    SmartDataPtr<Event::TkrVertexCol>  tracks(eventSvc(),    EventModel::TkrRecon::TkrVertexCol);
    SmartDataPtr<Event::CalClusterCol> clusters(eventSvc(),  EventModel::CalRecon::CalClusterCol);

    processTDS( header, particles, tracks, clusters);

    m_fm->execute();

    return sc;
}
void meritAlg::processTDS(const Event::EventHeader& header,
                          const Event::McParticleCol& particles, 
                          const Event::TkrVertexCol& tracks,
                          const Event::CalClusterCol& clusters)
{
    
    // Procedure and Method:  Process the collection of Monte carlo particles
    
    MsgStream   log( msgSvc(), name() );

    m_time = header.time();
    
    // assume first mc particle is the primary.   
    const Event::McParticle& primary = **particles.begin();
    m_mce = (primary.initialFourMomentum().e() - primary.initialFourMomentum().mag())*1e-3;
    
    
    
    // process the cluster(s)
    if( clusters.num() >0 ){
        m_recon_energy = clusters.getCluster(0)->getEnergySum()*1e-3; // convert to GeV
    }else {
        m_recon_energy = 0;
    }
    
   
    m_tracks = tracks.size();
    
    if(m_tracks==0) {
        m_angle_diff = 1.0;
    }else {
        const Event::TkrVertex& track = *(tracks.front());
        
        const Event::TkrFitPar fitpar=track.getTrackPar();
        Point p = track.getPosition();
        Vector dir = track.getDirection();
        
        
        // get difference
        m_angle_diff = acos( Hep3Vector(primary.initialFourMomentum()).unit() * dir );

        // conversion layer
        m_first_hit = track.getLayer();
    }
    
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

