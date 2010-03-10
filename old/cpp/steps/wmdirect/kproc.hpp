////////////////////////////////////////////////////////////////////////////////
// STEPS - STochastic Engine for Pathway Simulation
// Copyright (C) 2007-2009�Okinawa Institute of Science and Technology, Japan.
// Copyright (C) 2003-2006�University of Antwerp, Belgium.
//
// See the file AUTHORS for details.
//
// This file is part of STEPS.
//
// STEPS�is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// STEPS�is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.�If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef STEPS_WMDIRECT_KPROC_HPP
#define STEPS_WMDIRECT_KPROC_HPP 1

// Autotools definitions.
#ifdef HAVE_CONFIG_H
#include <steps/config.h>
#endif

// STL headers.
#include <vector>

// STEPS headers.
#include <steps/common.h>
#include <steps/solver/types.hpp>
#include <steps/solver/reacdef.hpp>
#include <steps/solver/sreacdef.hpp>

////////////////////////////////////////////////////////////////////////////////

START_NAMESPACE(steps)
START_NAMESPACE(wmdirect)

//Forward declaration
class Comp;
class Patch;
class KProc;

////////////////////////////////////////////////////////////////////////////////

typedef steps::wmdirect::KProc *        KProcP;
typedef std::vector<KProcP>             KProcPVec;
typedef KProcPVec::iterator             KProcPVecI;
typedef KProcPVec::const_iterator       KProcPVecCI;

////////////////////////////////////////////////////////////////////////////////

class KProc

{
public:

    ////////////////////////////////////////////////////////////////////////
    // OBJECT CONSTRUCTION & DESTRUCTION
    ////////////////////////////////////////////////////////////////////////

    KProc(void);
    virtual ~KProc(void);

    ////////////////////////////////////////////////////////////////////////
    // DATA ACCESS
    ////////////////////////////////////////////////////////////////////////

    /*
    static const int INACTIVATED = 1;

    inline bool active(void) const
    { return !(pFlags & INACTIVATED); }
    inline bool inactive(void) const
    { return (pFlags & INACTIVATED); }
    void setActive(bool active);

    inline uint flags(void) const
    { return pFlags; }
	*/
    ////////////////////////////////////////////////////////////////////////

    uint schedIDX(void) const
    { return pSchedIDX; }

    void setSchedIDX(uint idx)
    { pSchedIDX = idx; }

    ////////////////////////////////////////////////////////////////////////
    // VIRTUAL INTERFACE METHODS
    ////////////////////////////////////////////////////////////////////////

    /// This function is called when all kproc objects have been created,
    /// allowing the kproc to pre-compute its SchedIDXVec.
    ///
    virtual void setupDeps(void) = 0;

    virtual bool depSpecComp(uint gidx, Comp * comp) = 0;
    virtual bool depSpecPatch(uint gidx, Patch * patch) = 0;

    /// Reset this Kproc.
    ///
    virtual void reset(void) = 0;

    // Recompute the Ccst for this KProc
    virtual void resetCcst(void) = 0;

    /// Compute the rate for this kproc (its propensity value).
    ///
    virtual double rate(void) const = 0;

    // Return the ccst for this kproc
    virtual double c(void) const = 0;

    // Return the h value for this kproc (number of available reaction channels)
    virtual double h(void) const = 0;

    /// Apply a single discrete instance of the kinetic process, returning
    /// a vector of kproc schedule indices that need to be updated as a
    /// result.
    ///
    virtual std::vector<uint> const & apply(void) = 0;

	virtual uint updVecSize(void) const = 0;

    ////////////////////////////////////////////////////////////////////////

    uint getExtent(void) const;
    void resetExtent(void);

    ////////////////////////////////////////////////////////////////////////

    // Return a pointer to the corresponding Reacdef or SReacdef object
    // Separate methods to avoid makeing a base KProcdef class
    //
    virtual steps::solver::Reacdef * defr(void) const;
    virtual steps::solver::SReacdef * defsr(void) const;

    ////////////////////////////////////////////////////////////////////////

protected:

    uint                                rExtent;

private:

    ////////////////////////////////////////////////////////////////////////
	/*
    uint                                pFlags;
    */
    uint                                pSchedIDX;

    ////////////////////////////////////////////////////////////////////////
};

////////////////////////////////////////////////////////////////////////////////

END_NAMESPACE(wmdirect)
END_NAMESPACE(steps)

#endif
// STEPS_WMDIRECT_KPROC_HPP

// END
