// LayerGroup.cxx: implementation of the LayerGroup class.
//
// $Id: LayerGroup.cxx,v 1.1 1999/07/05 19:08:03 burnett Exp $
//////////////////////////////////////////////////////////////////////

#include "LayerGroup.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LayerGroup::LayerGroup(const Tuple& t, int min_layer, int max_layer)
:  Analyze(t, "fst_X_Lyr", "Events used")
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
