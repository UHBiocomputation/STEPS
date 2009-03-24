////////////////////////////////////////////////////////////////////////////////
// STEPS - STochastic Engine for Pathway Simulation
// Copyright (C) 2005-2008 Stefan Wils. All rights reserved.
//
// This file is part of STEPS.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA
//
// $Id$
////////////////////////////////////////////////////////////////////////////////

// Autotools definitions.
#ifdef HAVE_CONFIG_H
#include <steps/config.h>
#endif

// STL headers.
#include <cassert>
#include <sstream>
#include <string>
#include <iostream>

// STEPS headers.
#include <steps/common.h>
#include <steps/error.hpp>
#include <steps/model/model.hpp>
#include <steps/model/surfsys.hpp>
#include <steps/model/sreac.hpp>
#include <steps/model/spec.hpp>

////////////////////////////////////////////////////////////////////////////////

USING_NAMESPACE(std);
USING_NAMESPACE(steps::model);

////////////////////////////////////////////////////////////////////////////////

SReac::SReac(string const & id, Surfsys * surfsys,
             vector<Spec *> const & vlhs, vector<Spec *> const & slhs,
             vector<Spec *> const & irhs, vector<Spec *> const & srhs,
             vector<Spec *> const & orhs, double kcst)
: pID(id)
, pModel(0)
, pSurfsys(surfsys)
, pOuter(true)
, pVLHS()
, pSLHS()
, pIRHS()
, pSRHS()
, pORHS()
, pOrder(0)
, pKcst(kcst)
{
    if (pSurfsys == 0)
    {
        ostringstream os;
        os << "No surfsys provided to SReac initializer function";
        throw steps::ArgErr(os.str());
    }
	if (pKcst < 0.0)
	{
		ostringstream os;
		os << "Surface reaction constant can't be negatove";
		throw steps::ArgErr(os.str());
	}

	pModel = pSurfsys->getModel();
	assert (pModel != 0);

    setVLHS(vlhs);
    setSLHS(slhs);
    setIRHS(irhs);
    setSRHS(srhs);
    setORHS(orhs);

    pSurfsys->_handleSReacAdd(this);
}

////////////////////////////////////////////////////////////////////////////////

SReac::~SReac(void)
{
    if (pSurfsys == 0) return;
    _handleSelfDelete();
}

////////////////////////////////////////////////////////////////////////////////

void SReac::_handleSelfDelete(void)
{
	pSurfsys->_handleSReacDel(this);
	pKcst = 0.0;
	pOrder = 0;
	pORHS.clear();
	pSRHS.clear();
	pIRHS.clear();
	pSLHS.clear();
	pVLHS.clear();
	pSurfsys = 0;
	pModel = 0;
}

////////////////////////////////////////////////////////////////////////////////

void SReac::setID(string const & id)
{
    assert(pSurfsys != 0);
    // The following might raise an exception, e.g. if the new ID is not
    // valid or not unique. If this happens, we don't catch but simply let
    // it pass by into the Python layer.
    pSurfsys->_handleSReacIDChange(pID, id);
    // This line will only be executed if the previous call didn't raise
    // an exception.
    pID = id;
}

////////////////////////////////////////////////////////////////////////////////

void SReac::setInner(bool inner)
{
	assert (pSurfsys != 0);
    pOuter = (! inner);
}

////////////////////////////////////////////////////////////////////////////////

void SReac::setOuter(bool outer)
{
	assert (pSurfsys != 0);
    pOuter = outer;
}

////////////////////////////////////////////////////////////////////////////////

void SReac::setVLHS(vector<Spec *> const & vlhs)
{
    assert (pSurfsys != 0);
    pVLHS.clear();
    SpecPVecCI vl_end = vlhs.end();
    for (SpecPVecCI vl = vlhs.begin(); vl != vl_end; ++vl)
    {
		assert ((*vl)->getModel() == pModel);
        pVLHS.push_back(*vl);
    }
    pOrder = pVLHS.size() + pSLHS.size();
}

////////////////////////////////////////////////////////////////////////////////

void SReac::setSLHS(vector<Spec *> const & slhs)
{
    assert (pSurfsys != 0);
    pSLHS.clear();
    SpecPVecCI sl_end = slhs.end();
    for (SpecPVecCI sl = slhs.begin(); sl != sl_end; ++sl)
    {
		assert ((*sl)->getModel() == pModel);
        pSLHS.push_back(*sl);
    }
    pOrder = pVLHS.size() + pSLHS.size();
}

////////////////////////////////////////////////////////////////////////////////

void SReac::setIRHS(vector<Spec *> const & irhs)
{
    assert (pSurfsys != 0);
    pIRHS.clear();
    SpecPVecCI ir_end = irhs.end();
    for (SpecPVecCI ir = irhs.begin(); ir != ir_end; ++ir)
    {
		assert ((*ir)->getModel() == pModel);
        pIRHS.push_back(*ir);
    }
}

////////////////////////////////////////////////////////////////////////////////

void SReac::setSRHS(vector<Spec *> const & srhs)
{
    assert (pSurfsys != 0);
    pSRHS.clear();
    SpecPVecCI sr_end = srhs.end();
    for (SpecPVecCI sr = srhs.begin(); sr != sr_end; ++sr)
    {
        assert ((*sr)->getModel() == pModel);
        pSRHS.push_back(*sr);
    }
}

////////////////////////////////////////////////////////////////////////////////

void SReac::setORHS(vector<Spec *> const & orhs)
{
    assert (pSurfsys != 0);
    pORHS.clear();
    SpecPVecCI or_end = orhs.end();
    for (SpecPVecCI ors = orhs.begin(); ors != or_end; ++ors)
    {
		assert ((*ors)->getModel() == pModel);
        pORHS.push_back(*ors);
    }
}

////////////////////////////////////////////////////////////////////////////////

void SReac::setKcst(double kcst)
{
	assert (pSurfsys != 0);
	if(kcst < 0.0)
	{
		ostringstream os;
		os << "Surface reaction constant can't be negative";
		throw steps::ArgErr(os.str());
	}
    pKcst = kcst;
}

////////////////////////////////////////////////////////////////////////////////

vector<Spec *> SReac::getAllSpecs(void) const
{
    SpecPVec specs = SpecPVec();
	bool first_occ = true;

	SpecPVec vlhs = getVLHS();
	SpecPVecCI vl_end = vlhs.end();
	for (SpecPVecCI vl = vlhs.begin(); vl != vl_end; ++vl)
	{
		first_occ = true;
		SpecPVecCI s_end = specs.end();
		for (SpecPVecCI s = specs.begin(); s != s_end; ++s)
		{
			if ((*s) == (*vl))
			{
				first_occ = false;
				break;
			}
		}
		if (first_occ == true) specs.push_back((*vl));
	}

	SpecPVec slhs = getSLHS();
	SpecPVecCI sl_end = slhs.end();
	for (SpecPVecCI sl = slhs.begin(); sl != sl_end; ++sl)
	{
		first_occ = true;
		SpecPVecCI s_end = specs.end();
		for (SpecPVecCI s = specs.begin(); s != s_end; ++s)
		{
			if ((*s) == (*sl))
			{
				first_occ = false;
				break;
			}
		}
		if (first_occ == true) specs.push_back((*sl));
	}

	SpecPVec irhs = getIRHS();
	SpecPVecCI ir_end = irhs.end();
	for (SpecPVecCI ir = irhs.begin(); ir != ir_end; ++ir)
	{
		first_occ = true;
		SpecPVecCI s_end = specs.end();
		for (SpecPVecCI s = specs.begin(); s != s_end; ++s)
		{
			if ((*s) == (*ir))
			{
				first_occ = false;
				break;
			}
		}
		if (first_occ == true) specs.push_back((*ir));
	}

	SpecPVec srhs = getSRHS();
	SpecPVecCI sr_end = srhs.end();
	for (SpecPVecCI sr = srhs.begin(); sr != sr_end; ++sr)
	{
		first_occ = true;
		SpecPVecCI s_end = specs.end();
		for (SpecPVecCI s = specs.begin(); s != s_end; ++s)
		{
			if ((*s) == (*sr))
			{
				first_occ = false;
				break;
			}
		}
		if (first_occ == true) specs.push_back((*sr));
	}

	SpecPVec orhs = getORHS();
	SpecPVecCI ors_end = orhs.end();
	for (SpecPVecCI ors = orhs.begin(); ors != ors_end; ++ors)
	{
		first_occ = true;
		SpecPVecCI s_end = specs.end();
		for (SpecPVecCI s = specs.begin(); s != s_end; ++s)
		{
			if ((*s) == (*ors))
			{
				first_occ = false;
				break;
			}
		}
		if (first_occ == true) specs.push_back((*ors));
	}

	return specs;
}

////////////////////////////////////////////////////////////////////////////////

// END