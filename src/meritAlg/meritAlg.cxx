// $Header: /nfs/slac/g/glast/ground/cvs/merit/src/meritAlg/meritAlg.cxx,v 1.12 2002/06/16 23:51:24 burnett Exp $

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
#include "Event/Recon/CalRecon/CalXtalRecData.h"
#include "Event/Recon/AcdRecon.h"

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
        
        const Event::TkrVertexCol& tracks);
    
    void particleReco( const Event::McParticleCol& particles);

    void clusterReco(const Event::CalClusterCol& clusters, const Event::CalXtalRecCol&);

    void tileReco(const Event::AcdRecon& );
    
    
    FigureOfMerit* m_fm;
    Tuple*   m_tuple;
    std::string m_cuts; 
    
    // places to put stuff found in the TDS
    float m_mce, m_trig, m_angle_diff, m_recon_energy;
    float m_time;

    float m_mc_zdir;

    // from track analysis
    float m_tracks;
    float m_first_hit;
    float m_tkr_gamma_zdir;

    // set by cal analysis
    float m_cal_energy_deposit;
    float m_no_xtals;
    float m_no_xtals_trunc;
    float m_cal_xtal_ratio;
    float m_cal_transv_rms;
    float m_cal_long_rms;
    float m_calFitErrNrm;


    float m_cal_elayer[8];
    float m_cal_z;

    // set by acd analysis
    float m_acd_doca;
    float m_doca[5];
    float m_acd_actdist;
    float m_acd_tileCount;

    int m_generated;
    Hep3Vector m_incident_dir;
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

    static float dummy = -99; // flag for defined, not implemented

    // define tuple items
    new TupleItem("MC_Energy",      &m_mce);
    new TupleItem("MC_Gamma_Err",   &m_angle_diff);
    new TupleItem("MC_zdir",        &m_mc_zdir);
    new TupleItem("elapsed_time",   &m_time);

    new TupleItem("trig_bits",      &m_trig);
    new TupleItem("TKR_No_Tracks",  &m_tracks);
    new TupleItem("TKR_First_XHit", &m_first_hit);
    new TupleItem("TKR_Gamma_zdir", &m_tkr_gamma_zdir);

    new TupleItem("REC_CsI_Corr_Energy", &m_recon_energy);

    new TupleItem("Cal_Energy_Deposit", &m_cal_energy_deposit);
    new TupleItem("Cal_No_Xtals",     &m_no_xtals);
    new TupleItem("Cal_No_Xtals_Trunc",&m_no_xtals_trunc);
    new TupleItem("Cal_Xtal_Ratio",   &m_cal_xtal_ratio);
    new TupleItem("CAL_long_rms",     &m_cal_long_rms);
    new TupleItem("CAL_transv_rms",   &m_cal_transv_rms);
    new TupleItem("CAL_Fit_errNrm",   &m_calFitErrNrm);

    const std::string name_eLayer = "Cal_eLayer";
    const char* digit[8]={"0","1","2","3","4","5","6","7"};
    for (int layer=0; layer<8; layer++){
        
        new TupleItem(std::string("Cal_eLayer")+digit[layer],m_cal_elayer[layer]);
    }
    new TupleItem("CAL_Z",           &m_cal_z);

    new TupleItem("ACD_DOCA",        &m_acd_doca);
    new TupleItem("ACD_TopDOCA",     &m_doca[0]);
    new TupleItem("ACD_S0DOCA",      &m_doca[1]);
    new TupleItem("ACD_S1DOCA",      &m_doca[2]);
    new TupleItem("ACD_S2DOCA",      &m_doca[3]);
    new TupleItem("ACD_Act_Dist",    &m_acd_actdist);
    new TupleItem("ACD_TileCount",   &m_acd_tileCount);
    new TupleItem("ACD_No_SideRow2", &dummy);
    new TupleItem("ACD_No_SideRow3", &dummy);


    m_fm= new FigureOfMerit(*m_tuple, m_cuts);
    
    // setup defaults in case no primary info
    m_incident_dir=Hep3Vector(0,0,-1);
    m_mce = 0.1f;
    return sc;
}


//------------------------------------------------------------------------------
StatusCode meritAlg::execute() {
    
    StatusCode  sc = StatusCode::SUCCESS;
    
    SmartDataPtr<Event::EventHeader>   header(eventSvc(),    EventModel::EventHeader);
    SmartDataPtr<Event::McParticleCol> particles(eventSvc(), EventModel::MC::McParticleCol);
    SmartDataPtr<Event::TkrVertexCol>  tracks(eventSvc(),    EventModel::TkrRecon::TkrVertexCol);
    SmartDataPtr<Event::CalClusterCol> clusters(eventSvc(),  EventModel::CalRecon::CalClusterCol);
    SmartDataPtr<Event::CalXtalRecCol> xtalrecs(eventSvc(),  EventModel::CalRecon::CalXtalRecCol);
    SmartDataPtr<Event::AcdRecon> acdrec(eventSvc(),         EventModel::AcdRecon::Event);
    
    if( particles!=0 )particleReco(particles);

    processTDS( header,  tracks);
    clusterReco(clusters, xtalrecs);

    tileReco(acdrec);
    
    m_fm->execute();
    
    return sc;
}
//------------------------------------------------------------------------------
void meritAlg::particleReco(const Event::McParticleCol& particles)
{
    if( ! particles.empty() ){
       // assume first mc particle is the primary.   
       const Event::McParticle& primary= **particles.begin();
       m_mce = (primary.initialFourMomentum().e() - primary.initialFourMomentum().mag())*1e-3;
       m_incident_dir = Hep3Vector(primary.initialFourMomentum()).unit();
       m_mc_zdir = m_incident_dir.z();
    }
    
}
//------------------------------------------------------------------------------
void meritAlg::processTDS(const Event::EventHeader& header,
                          const Event::TkrVertexCol& tracks)
{
    
    // Procedure and Method:  Process the collection of Monte carlo particles
    
    MsgStream   log( msgSvc(), name() );
    
    m_time = header.time();
    
 
    m_tracks = tracks.size();
    m_trig = header.trigger();
    
    if(m_tracks==0) {
        m_angle_diff = 1.0;
    }else {
        const Event::TkrVertex& track = *(tracks.front());
        
        const Event::TkrFitPar fitpar=track.getTrackPar();
        Point p = track.getPosition();
        Vector dir = track.getDirection();
        
        m_tkr_gamma_zdir = dir.z();

        // get difference from incident mc direction: allow either convention on direction of 
        // output.
        m_angle_diff = acos( fabs( m_incident_dir * dir) );
        
        // conversion layer
        m_first_hit = track.getLayer();
    }
    
}
//------------------------------------------------------------------------------
void meritAlg::clusterReco(const Event::CalClusterCol& clusters, const Event::CalXtalRecCol& crl)
{
    // process the cluster(s)
    if( clusters.num()==0 ){
        m_recon_energy = 0;
        return;
    }
    // code from the former CalNtupleAlg.cxx, slightly modified for new interface access names.
    
    double fit_ener,fitalpha,fitlambda,profchi2,eleak,start;
    float energy_sum;
    
    // loop through the clusters, 
    for( Event::CalClusterCol::const_iterator icl=clusters.begin(); icl!=clusters.end(); ++icl){

        const Event::CalCluster* cl = *icl;

        energy_sum = cl->getEnergySum();
        
        //stop writing NANs to the tuple
        float zpos = -1000.0;
        float xpos = -1000.0;
        float ypos = -1000.0;
        
        zpos = (cl->getPosition()).z(); 
        xpos = (cl->getPosition()).x();
        ypos = (cl->getPosition()).y();
        
        const std::vector<double>& eneLayer = cl->getEneLayer();

        const std::vector<Vector>& posLayer = cl->getPosLayer();
        
        float trans_rms = cl->getRmsTrans();
        float long_rms = cl->getRmsLong();
        Vector caldir = cl->getDirection();
        float caltheta = -1000.0;
        float calphi = -1000.0;
        if(abs(caldir.z())<1.){ caltheta=acos(caldir.z());
        calphi=float(M_PI/2.);
        if(caldir.x()!=0.) calphi = atan(caldir.y()/caldir.x());
        }
        fit_ener = cl->getFitEnergy();
        profchi2 = cl->getProfChisq();
        fitalpha = cl->getCsiAlpha();
        fitlambda = cl->getCsiLambda();
        start = cl->getCsiStart();
        eleak = cl->getEnergyLeak();
        float transvOffset = cl->getTransvOffset();
        
        float eFactor = .26 + .35 / (cl->getEnergyCorrected());
        float calFitErrNrm = transvOffset/eFactor;

        
        
/*
        sc = m_ntupleWriteSvc->addItem(m_tupleName.c_str(), "Cal_Transv_Offset", transvOffset);
        sc = m_ntupleWriteSvc->addItem(m_tupleName.c_str(), "Cal_Fit_errNrm", calFitErrNrm);
        
        
        sc = m_ntupleWriteSvc->addItem(m_tupleName.c_str(), "Cal_Energy_Deposit", energy_sum);
        sc = m_ntupleWriteSvc->addItem(m_tupleName.c_str(), "Cal_Energy_Inc_Prof", fit_ener);
        sc = m_ntupleWriteSvc->addItem(m_tupleName.c_str(), "Cal_Energy_Inc_LeakCorr", eleak);
        sc = m_ntupleWriteSvc->addItem(m_tupleName.c_str(), "Cal_Energy_Incident", fit_ener);  // for the moment
        sc = m_ntupleWriteSvc->addItem(m_tupleName.c_str(), "Energy_Incident_CalTkr", energy_sum);  // to be updated
        
        const std::string name_eLayer = "Cal_eLayer";
        const char* digit[8]={"0","1","2","3","4","5","6","7"};
        for (int layer=0; layer<8; layer++)
            sc = m_ntupleWriteSvc->addItem(m_tupleName.c_str(), (name_eLayer+digit[layer]).c_str(), eneLayer[layer]);
        
        sc = m_ntupleWriteSvc->addItem(m_tupleName.c_str(), "Cal_Z", zpos);
        sc = m_ntupleWriteSvc->addItem(m_tupleName.c_str(), "Cal_X", xpos);
        sc = m_ntupleWriteSvc->addItem(m_tupleName.c_str(), "Cal_Y", ypos);
        sc = m_ntupleWriteSvc->addItem(m_tupleName.c_str(), "Cal_transv_rms", trans_rms);
        sc = m_ntupleWriteSvc->addItem(m_tupleName.c_str(), "Cal_long_rms", long_rms);
        sc = m_ntupleWriteSvc->addItem(m_tupleName.c_str(), "Cal_theta", caltheta);
        sc = m_ntupleWriteSvc->addItem(m_tupleName.c_str(), "Cal_phi", calphi);
 */       
    // set tuple items for this cluster. (What if more????)
        m_cal_z=zpos;
        std::copy(eneLayer.begin(), eneLayer.end(), m_cal_elayer);
        m_calFitErrNrm = calFitErrNrm;
        m_cal_transv_rms = trans_rms;
        m_cal_long_rms = long_rms;

    }
    
    
    int no_xtals=0;
    int no_xtals_trunc=0;
    for( Event::CalXtalRecCol::const_iterator jlog=crl.begin(); jlog != crl.end(); ++jlog){

        const Event::CalXtalRecData& recLog = **jlog;
        
        double eneLog = recLog.getEnergy();
        if(eneLog>0)no_xtals++;
        if(eneLog>0.01*energy_sum)no_xtals_trunc++;
    }
    float cal_xtal_ratio= (no_xtals>0) ? float(no_xtals_trunc)/no_xtals : 0;
    /*
    sc = m_ntupleWriteSvc->addItem(m_tupleName.c_str(), "Cal_No_Xtals", no_xtals);
    sc = m_ntupleWriteSvc->addItem(m_tupleName.c_str(), "Cal_No_Xtals_Trunc", no_xtals_trunc);
    sc = m_ntupleWriteSvc->addItem(m_tupleName.c_str(), "Cal_Xtal_Ratio", cal_xtal_ratio);
    
    */
    // set tuple items
    
    m_no_xtals = no_xtals;
    m_no_xtals_trunc = no_xtals_trunc;
    m_cal_xtal_ratio = cal_xtal_ratio;
    
    m_recon_energy = clusters.getCluster(0)->getEnergySum()*1e-3; // convert to GeV
    
}
//------------------------------------------------------------------------------
void meritAlg::tileReco(const Event::AcdRecon& acd)
{
    //TODO: set appropriate stuff
    m_acd_doca = acd.getDoca();
    m_acd_actdist = acd.getActiveDist();
    m_acd_tileCount = acd.getTileCount();
    const std::vector<double> & doca = acd.getRowDocaCol();
    int nd = doca.size();
    std::copy(doca.begin(), doca.end(), m_doca);


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

