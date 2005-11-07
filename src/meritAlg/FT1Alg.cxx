/** @file FT1Alg.cxx
@brief Declaration and implementation of Gaudi algorithm FT1Alg

$Header$
*/
// Include files

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/SmartDataPtr.h"

#include "astro/PointingTransform.h"

#include "Event/MonteCarlo/Exposure.h"

#include "ntupleWriterSvc/INTupleWriterSvc.h"

#include <cassert>

// forward declatation of the worker
class FT1worker;

namespace { // anonymous namespace for file-global
    INTupleWriterSvc* rootTupleSvc;
    long nbOfEvtsInFile(100000);
    std::string treename("MeritTuple");
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/** @class FT1Alg
@brief Extract info from tuple, etc. to add ft1 items to this of another tree
*/
class FT1Alg : public Algorithm {

public:
    FT1Alg(const std::string& name, ISvcLocator* pSvcLocator); 

    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();
private:
    /// this guy does the work!
    FT1worker * m_worker;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static const AlgFactory<FT1Alg>  Factory;
const IAlgFactory& FT1AlgFactory = Factory;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class FT1worker{ 
public:
    FT1worker();

    void evaluate(const Event::Exposure& exp);

private:
    std::map<std::string, double> getCelestialCoords(const Event::Exposure& exp,
        const Hep3Vector glastDir);

    bool useVertex(){ //TODO: implement
        return false;
    }

    template <typename T>
        void addItem(std::string name, const T & value)
    {
        rootTupleSvc->addItem(treename, name, &value);
    }

    class Item {
    public:
        Item(std::string name)
        {
            m_isFloat = rootTupleSvc->getItem(treename, name, m_pvalue);
        }

        operator double()
        {
            return m_isFloat? *(float*)m_pvalue : *(double*)m_pvalue;
        }

        void* m_pvalue;
        bool m_isFloat;
    };

    // tuple items expect to find
    Item EvtRun, EvtEventId;
    Item EvtEnergyCorr;
    Item VtxXDir, VtxYDir, VtxZDir;
    Item VtxX0, VtxY0, VtxZ0;
    Item Tkr1XDir, Tkr1YDir, Tkr1ZDir;
    Item Tkr1X0, Tkr1Y0, Tkr1Z0;
    Item Tkr1FirstLayer;

    //FT1 entries to create
    float m_ft1eventid;
    float m_ft1energy;
    float m_ft1theta,m_ft1phi,m_ft1ra,m_ft1dec,m_ft1zen,m_ft1azim;
    float m_ft1convpointx,m_ft1convpointy,m_ft1convpointz,m_ft1convlayer;

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FT1Alg::FT1Alg(const std::string& name, ISvcLocator* pSvcLocator) :
Algorithm(name, pSvcLocator)
{
    declareProperty("TreeName",  treename="MeritTuple");
    declareProperty("NbOfEvtsInFile", nbOfEvtsInFile=100000);

}

StatusCode FT1Alg::initialize()
{
    StatusCode  sc = StatusCode::SUCCESS;

    MsgStream log(msgSvc(), name());

    // Use the Job options service to get the Algorithm's parameters
    setProperties();

    // get a pointer to RootTupleSvc 
    if( (sc = service("RootTupleSvc", rootTupleSvc, true) ). isFailure() ) {
        log << MSG::ERROR << " failed to get the RootTupleSvc" << endreq;
        return sc;
    }
    m_worker = new FT1worker();
    
    return sc;
}

StatusCode FT1Alg::execute()
{

    //First get the coordinates from the ExposureCol
    Event::ExposureCol* elist = 0;
    eventSvc()->retrieveObject("/Event/MC/ExposureCol",(DataObject *&)elist);
    assert( elist!=0); // should not happen, but make sure ok.
    const Event::Exposure& exp = **(*elist).begin();

    // now have the worker do it
    m_worker->evaluate(exp);

    return StatusCode::SUCCESS;
}

StatusCode FT1Alg::finalize()
{
    return StatusCode::SUCCESS;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

FT1worker::FT1worker()
// initialize pointers to current items
: EvtRun("EvtRun")
, EvtEventId("EvtEventId")
, EvtEnergyCorr("EvtEnergyCorr")
, Tkr1FirstLayer("Tkr1FirstLayer")
, VtxXDir("VtxXDir")
, VtxYDir("VtxYDir")
, VtxZDir("VtxZDir")
, VtxX0("VtxX0")
, VtxY0("VtxY0")
, VtxZ0("VtxZ0")
, Tkr1XDir("Tkr1XDir")
, Tkr1YDir("Tkr1YDir")
, Tkr1ZDir("Tkr1ZDir")
, Tkr1X0("Tkr1X0")
, Tkr1Y0("Tkr1Y0")
, Tkr1Z0("Tkr1Z0")
{
   //now create new items 
   /** @page MeritTuple MeritTuple definitions
    
    @section FT1  Event summary for generation of the FT1 record


    see <a href="http://glast.gsfc.nasa.gov/ssc/dev/fits_def/definitionFT1.html">FT1 definition</a>

    @param FT1EventId  RunNo*(number of events in file) + EventNo  
    @param FT1Energy   (MeV) estimate for energy  
    @param FT1Theta,FT1Phi  (deg) reconstructed direction with respect to instrument coordinate system      
    @param FT1Ra,FT1Dec  (deg) reconstructed direction in equatorial coordinates       
    @param FT1ZenithTheta,FT1EarthAzimuth (deg) reconstucted direction with respect to local zenith system
    @param FT1ConvPointX,FT1ConvPointY,FT1ConvPointZ (m) conversion point of event, whether single track or vertex, 999 if no tracks

    */
    addItem( "FT1EventId",       m_ft1eventid);
    addItem( "FT1Energy",        m_ft1energy);
    addItem( "FT1Theta",         m_ft1theta);
    addItem( "FT1Phi",           m_ft1phi);
    addItem( "FT1Ra",            m_ft1ra);
    addItem( "FT1Dec",           m_ft1dec);
    addItem( "FT1ZenithTheta",   m_ft1zen);
    addItem( "FT1EarthAzimuth",  m_ft1azim);
    addItem( "FT1ConvPointX",    m_ft1convpointx);
    addItem( "FT1ConvPointY",    m_ft1convpointy);
    addItem( "FT1ConvPointZ",    m_ft1convpointz);
}


void FT1worker::evaluate(const Event::Exposure& exp)
{

 
    //eventId and Time are always defined
    m_ft1eventid = EvtRun * nbOfEvtsInFile + EvtEventId;

    // Give default "guard" values in case there are no tracks in the event
    m_ft1energy    = EvtEnergyCorr;
    m_ft1theta = 666; m_ft1phi = 666; m_ft1ra   = 666;
    m_ft1dec   = 666; m_ft1zen = 666; m_ft1azim = 666;
    m_ft1convpointx = 999; m_ft1convpointy = 999; m_ft1convpointz = 999; 
    m_ft1convlayer = -1;

    Hep3Vector convPoint;
    Hep3Vector glastDir;


    if( useVertex() ) {

        glastDir = Hep3Vector(VtxXDir,VtxYDir,VtxZDir);
        m_ft1convpointx  = VtxX0;
        m_ft1convpointy  = VtxY0;
        m_ft1convpointz  = VtxZ0;
    
    } else{

        glastDir = Hep3Vector(Tkr1XDir, Tkr1YDir,Tkr1ZDir);
        m_ft1convpointx  = Tkr1X0;
        m_ft1convpointy  = Tkr1Y0;
        m_ft1convpointz  = Tkr1Z0;
    }
    m_ft1convlayer   = Tkr1FirstLayer;

    glastDir = - glastDir.unit();

    // celestial coords in degree
    std::map<std::string,double> cel_coords = getCelestialCoords(exp, glastDir);
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
std::map<std::string, double> 
FT1worker::getCelestialCoords(const Event::Exposure& exp, const Hep3Vector glastDir)
{
    using namespace astro;

    std::map<std::string, double> fields;


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


