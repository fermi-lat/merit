// $Header:  $

// Include files

#include "Gaudi/Algorithm/Algorithm.h"
#include "Gaudi/MessageSvc/MsgStream.h"
#include "Gaudi/Kernel/AlgFactory.h"
#include "Gaudi/Interfaces/IDataProviderSvc.h"
#include "Gaudi/DataSvc/SmartDataPtr.h"
#include "Gaudi/NTupleSvc/NTuple.h"
#include "Gaudi/Interfaces/INTupleSvc.h"
#include "GaudiTuple.h"

#include "merit/FigureOfMerit.h"
#include "analysis/Tuple.h"

static std::string  default_cuts("12rbA");

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class meritAlg : public Algorithm {
    
public:
    meritAlg(const std::string& name, ISvcLocator* pSvcLocator); 
    
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();
private:
    FigureOfMerit* m_fm;
    GaudiTuple* m_tuple;
    std::string m_cuts;    
    std::string m_tuple_file;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static const AlgFactory<meritAlg>  Factory;
const IAlgFactory& meritAlgFactory = Factory;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
meritAlg::meritAlg(const std::string& name, ISvcLocator* pSvcLocator) :
Algorithm(name, pSvcLocator) {
    declareProperty("cuts" , m_cuts=default_cuts);
    declareProperty("tuple_path", m_tuple_file="AORECON");
}

StatusCode meritAlg::initialize() {
    StatusCode  sc = StatusCode::SUCCESS;
    
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "initialize" << endreq;
    
    // Use the Job options service to get the Algorithm's parameters
    setProperties();
    
    // setup the tuple somehow
    
    std::string top = "/NTUPLES/FILE1";
    NTupleDirPtr dir(ntupleSvc(), top );
    
    NTuplePtr nt(dir, "/1");
    if( 0==nt) {
        log << MSG::ERROR << "Could not open the tuple file" << endreq;
        return StatusCode::FAILURE;
    }
    
    // make the wrapper for access from all the cuts
    m_tuple = new GaudiTuple(nt->title(), &nt);
    
    m_fm= new FigureOfMerit(*m_tuple, m_cuts);
    
       
    // Access the N tuple event by event
    while ( sc.isSuccess() ) {
        sc = ntupleSvc()->readRecord(nt.ptr());
        if ( sc.isSuccess() ) {
            m_fm->execute();
        }
    }
    m_fm->report(std::cout);
    
    return sc;
}


//------------------------------------------------------------------------------
StatusCode meritAlg::execute() {
    
    StatusCode  sc = StatusCode::SUCCESS;
   
    return sc;
}


//------------------------------------------------------------------------------
StatusCode meritAlg::finalize() {
    
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "finalize" << endreq;
    
    delete m_fm;
    return StatusCode::SUCCESS;
}

