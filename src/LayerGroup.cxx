// LayerGroup.cxx: implementation of the LayerGroup class.
//
// $Id: LayerGroup.cxx,v 1.2 2001/06/14 19:51:59 usher Exp $
//////////////////////////////////////////////////////////////////////

#include "LayerGroup.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
// -- June 12, 2001 changed fst_X_Lyr to TKR_First_XHit   TU
//////////////////////////////////////////////////////////////////////

LayerGroup::LayerGroup(const Tuple& t, int min_layer, int max_layer)
:  Analyze(t, "Tkr1_1stLayer", "Events used")
,  m_psf(t,0,0)
,  m_energy(t)
,  m_minlayer(min_layer), m_maxlayer(max_layer)
{


}
void LayerGroup::report(std::ostream& out)
{
    if( m_maxlayer>m_minlayer) {
        out << std::endl<<" Layers " << m_minlayer << '-'<< m_maxlayer;
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

    m_psf();
    m_energy();
    return true;
}

LayerGroup::~LayerGroup()
{

}
