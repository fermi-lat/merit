// LayerGroup.h: interface for the LayerGroup class.
//
// $Id: LayerGroup.h,v 1.1 1999/07/05 19:08:03 burnett Exp $
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LAYERGROUP_H__EF7251C1_324A_11D3_8437_006008B7A02D__INCLUDED_)
#define AFX_LAYERGROUP_H__EF7251C1_324A_11D3_8437_006008B7A02D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PSFanalysis.h"
#include "EnergyAnalysis.h"

class LayerGroup : public Analyze 
// analyze and report on the effective area, PSF, and reconstructed energy for a group of layers
{
public:
    LayerGroup(const Tuple& t, int min_layer=0, int min_layer2=0);
    // all layers is the default

    virtual ~LayerGroup();

    void report(std::ostream& out);

    double sigma(); // return projected sigma

private:
    bool apply();

    PSFanalysis m_psf;
    EnergyAnalysis m_energy;
    int m_minlayer, m_maxlayer;
    
};

#endif // !defined(AFX_LAYERGROUP_H__EF7251C1_324A_11D3_8437_006008B7A02D__INCLUDED_)
