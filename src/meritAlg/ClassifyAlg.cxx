/** @file ClassifyAlg.cxx
@brief Declaration and implementation of Gaudi algorithm ClassifyAlg

$Header$
*/

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/SmartDataPtr.h"

#include "ntupleWriterSvc/INTupleWriterSvc.h"
#include "facilities/Util.h" // for expandEnvVar    

#include "classifier/DecisionTree.h"

#include "GlastClassify/AtwoodTrees.h"
#include "GlastClassify/ITupleInterface.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class GleamItem : public GlastClassify::Item {
public:
    GleamItem(const std::string& treename, const std::string& name, INTupleWriterSvc* tuplesvc)
    {
        m_isFloat = tuplesvc->getItem(treename, name, m_pdata);
    }
    operator double()const{
        return m_isFloat? *(float*) m_pdata : *(double*)m_pdata;
    }
private:
    void * m_pdata;
    bool m_isFloat;
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class GleamTuple : public GlastClassify::ITupleInterface {
public:
    GleamTuple( INTupleWriterSvc* tuple, const std::string& treename)
        : m_tuple(tuple)
        , m_treename(treename)
    {}

    const GlastClassify::Item* getItem(const std::string& name)const
    {
        const GlastClassify::Item* item =new GleamItem(m_treename, name, m_tuple);
        return item;
    }

    /// create a new item (float only for now) in the tuple, which will take the given value
    void addItem(const std::string& name, float & value)
    {
        m_tuple->addItem(m_treename, name, &value);
    }

private:
    INTupleWriterSvc* m_tuple;
    const std::string& m_treename;
};



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/** @class ClassifyAlg
@brief Extract info from tuple, etc. to add ft1 items to this of another tree
*/
class ClassifyAlg : public Algorithm {

public:
    ClassifyAlg(const std::string& name, ISvcLocator* pSvcLocator); 

    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();

private:
    StringProperty m_treename;
    StringProperty m_infoPath;

    /// this guy does the work!
    GlastClassify::AtwoodTrees * m_ctree;
    GleamTuple* m_tuple;

    int m_events;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static const AlgFactory<ClassifyAlg>  Factory;
const IAlgFactory& ClassifyAlgFactory = Factory;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ClassifyAlg::ClassifyAlg(const std::string& name, ISvcLocator* pSvcLocator) 
: Algorithm(name, pSvcLocator)
,  m_ctree(0)
,  m_events(0)

{
    declareProperty("TreeName", m_treename="MeritTuple");
    declareProperty("InfoPath", m_infoPath="$(GLASTCLASSIFYROOT)/treeinfo");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
StatusCode ClassifyAlg::initialize()
{
    StatusCode  sc = StatusCode::SUCCESS;

    MsgStream log(msgSvc(), name());

    // Use the Job options service to get the Algorithm's parameters
    setProperties();

    // get a pointer to RootTupleSvc 
    INTupleWriterSvc* rootTupleSvc(0);

    if( (sc = service("RootTupleSvc", rootTupleSvc, true) ). isFailure() ) {
        log << MSG::ERROR << " failed to get the RootTupleSvc" << endreq;
        return sc;
    }

    // create our interface to the tuple
    m_tuple= new GleamTuple(rootTupleSvc, m_treename);

    // create the classification object if requested
    try { 
        std::string path(  m_infoPath.value()); 
        if(! path.empty() ){
            facilities::Util::expandEnvVar(&path);
            m_ctree = new  GlastClassify::AtwoodTrees(*m_tuple, log.stream(), path);
            log << MSG::INFO << "Loading classification trees from " << path << endreq;
        } else {
            log << MSG::ERROR << "No classification trees found" << endreq;
            sc = StatusCode::FAILURE;
            
        }
        //TODO: finish setup.
    }catch ( std::exception& e){
        log << MSG::ERROR << "Exception caught, class  "<< typeid( e ).name( ) << ", message:"
            << e.what() <<endreq;
        sc = StatusCode::FAILURE;
    }catch (...)  {
        log << MSG::ERROR << "Unexpected exception loading classification trees" << endreq;
        sc = StatusCode::FAILURE;
    }
    return sc;
}

StatusCode ClassifyAlg::execute() 
{
    MsgStream log(msgSvc(), name());
    if( m_ctree!=0){
        m_ctree->execute();
        ++m_events;
    }
    return StatusCode::SUCCESS;
}

StatusCode ClassifyAlg::finalize() 
{
    MsgStream log(msgSvc(), name());

    log << MSG::INFO << "Processed " << m_events << " events." << endreq;
    delete m_ctree;
    delete m_tuple;
    setFinalized(); //  prevent being called again
    return StatusCode::SUCCESS;
}

