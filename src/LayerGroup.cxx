/** @file LayerGroup.cxx
   @brief implementation of the LayerGroup class.
   $Id: LayerGroup.cxx,v 1.6 2003/11/15 15:25:03 burnett Exp $
   */
//////////////////////////////////////////////////////////////////////

#include "LayerGroup.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
// -- June 12, 2001 changed fst_X_Lyr to TKR_First_XHit   TU
//////////////////////////////////////////////////////////////////////

LayerGroup::LayerGroup(const Tuple& t, int min_layer, int max_layer, CATEGORY cat)
:  Analyze(t, "Tkr1FirstLayer", "Events used")
,  m_psf(t,0,0, cat==ONE_TRACK? "McTkr1DirErr" : "McDirErr" )
,  m_energy(t)
,  m_minlayer(min_layer), m_maxlayer(max_layer)
, m_category(cat)
, m_vertexProb(cat!=ALL? t.tupleItem("IMvertexProb") : 0)
{
}

void LayerGroup::report(std::ostream& out)
{
    static const char* catname[]={"All", "Vertex", "1-Track"};
    if( m_maxlayer>m_minlayer) {
        out << std::endl<<" Layers " << m_minlayer << '-'<< m_maxlayer;
        out << "  " << catname[m_category];
    }
    Analyze::report(out);
    m_psf.report(out);
    m_energy.report(out);
}
double LayerGroup::sigma() {return m_psf.sigma(); }

bool LayerGroup::apply()
{
    int layer = static_cast<int>(item());
    if( m_maxlayer>m_minlayer && ( layer<m_minlayer || layer > m_maxlayer) )return false;

    if(    m_category==ALL 
        || m_category==VERTEX && *m_vertexProb>0.5 
        || m_category==ONE_TRACK && *m_vertexProb<0.5){
            m_psf();
            m_energy();
            return true;
        }
        return false;
}

LayerGroup::~LayerGroup()
{
}
