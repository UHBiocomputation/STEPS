####################################################################################
#
#    STEPS - STochastic Engine for Pathway Simulation
#    Copyright (C) 2007-2017 Okinawa Institute of Science and Technology, Japan.
#    Copyright (C) 2003-2006 University of Antwerp, Belgium.
#    
#    See the file AUTHORS for details.
#    This file is part of STEPS.
#    
#    STEPS is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License version 2,
#    as published by the Free Software Foundation.
#    
#    STEPS is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#    GNU General Public License for more details.
#    
#    You should have received a copy of the GNU General Public License
#    along with this program. If not, see <http://www.gnu.org/licenses/>.
#
#################################################################################   
###

# Supporting Module for generating STEPS morph sectioning file using NEURON

import sys


def hoc2morph(hoc_file):
    """
    Generate morph sectioning data from NEURON hoc file.
    """
    
    import sys
    from neuron import h
    
    sections = {}
    h.load_file(hoc_file)

    for sec in h.allsec():
        sections[sec.name()] = {}
        sections[sec.name()]["name"] = sec.name()
        sr = h.SectionRef(sec=sec)
        if sr.has_parent():
            parent = sr.parent.name()
        else:
            parent = None
        sections[sec.name()]["parent"] = parent

        children = []
        for child in sr.child:
            children.append(child.name())

        sections[sec.name()]["children"] = children
        x = []
        y = []
        z = []
        d = []
        n3d = int(h.n3d())
        sections[sec.name()]["points"] = []
        for i in range(n3d):
            sections[sec.name()]["points"].append([h.x3d(i), h.y3d(i), h.z3d(i), h.diam3d(i)])
    return sections

def swc2morph(swc_file):
    """
    Generate morph sectioning data from swc file.
    """
    
    from neuron import h
    h.load_file('stdlib.hoc')
    h.load_file('import3d.hoc')
    
    cell = h.Import3d_SWC_read()
    cell.input(swc_file)

    i3d = h.Import3d_GUI(cell, 0)
    i3d.instantiate(None)

    sections = {}

    for sec in h.allsec():
        sections[sec.name()] = {}
        sections[sec.name()]["name"] = sec.name()
        sr = h.SectionRef(sec=sec)
        if sr.has_parent():
            parent = sr.parent.name()
        else:
            parent = None
        sections[sec.name()]["parent"] = parent

        children = []
        for child in sr.child:
            children.append(child.name())

        sections[sec.name()]["children"] = children
        x = []
        y = []
        z = []
        d = []
        n3d = int(h.n3d())
        sections[sec.name()]["points"] = []
        for i in range(n3d):
            sections[sec.name()]["points"].append([h.x3d(i), h.y3d(i), h.z3d(i), h.diam3d(i)])
    return sections

################################################################################

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

################################################################################

def mapMorphTetmesh(morph_sections, mesh, morph2mesh_scale = 1e-6):
    """
    Map each tetrahedron in a mesh to morph sectioning.
    
    Parameters:
       * morph_sections    Morphology sectioning data generated by steps.utilities.morph_support.hoc2morph or swc2morph
       * mesh              STEPS Tetmesh object
       * morph2mesh_scale  Scaling factor from morph sectioning data to Tetmesh data, Default: 1e-6, that is 1 unit (usually micron) in the morph file equals 1e-6 unit (meter) in Tetmesh 
    
    Return:
       A list in format of [sec_id_tet0, sec_id_tet1, sec_id_tet_n, ...] where the n_th element stores the section id for tetrahedron n.
    """
    
    
    ntets = mesh.ntets
    mapped_list = [None] * ntets
    dsq_list = [-1] * ntets
    
    for sec in morph_sections.values():

        points = sec["points"]
        npoints = len(points)
        for i in range(npoints - 1):
            p0 = points[i]
            p1 = points[i+1]
            center = getCenter(p0, p1, morph2mesh_scale)
            intet = mesh.findTetByPoint(center)
            if intet == -1:
                print "Tracking point ", center, " of ", sec["name"], " is not in the mesh."
                print "Try to track end points instead."
                intet = mesh.findTetByPoint([p0[0] * morph2mesh_scale, p0[1] * morph2mesh_scale, p0[2] * morph2mesh_scale])
                if intet == -1:
                    print "First end point is not in the mesh, try second end point."
                    intet = mesh.findTetByPoint([p1[0] * morph2mesh_scale, p1[1] * morph2mesh_scale, p1[2] * morph2mesh_scale])
                    if intet == -1:
                        print "Second end point not in the mesh, will skip this cylinder."
                        continue

            mapped_list[intet] = sec["name"]
            dsq_list[intet] = 0.0
            
            # expend to neighbor tetrahedrons to see
            # if they also belong to this cylinder
            can_expend = True
            check_set = set([intet])
            while can_expend:
                expension = set([])
                for check_tet in check_set:
                    neighb_tets = mesh.getTetTetNeighb(check_tet)
                    for t in neighb_tets:
                        if t == -1: continue
                        center = mesh.getTetBarycenter(t)
                        dsq = isPointInCylinder(p0, p1, center, morph2mesh_scale)
                        if dsq == -1: continue
                        if mapped_list[t] == None:
                            expension.add(t)
                            mapped_list[t] = sec["name"]
                            dsq_list[t] = dsq
                            continue
                        if dsq_list[t] > dsq:
                            expension.add(t)
                            mapped_list[t] = sec["name"]
                            dsq_list[t] = dsq
            
                        
                if len(expension) == 0:
                    can_expend = False
                else:
                    check_set = expension

    for tet in range(len(mapped_list)):
        if mapped_list[tet] == None:
            can_expend = True
            unmap_set = set([tet])
            check_set = set([tet])
            found_mapping = None
            while can_expend:
                expension = set([])
                for check_tet in check_set:
                    if not can_expend: break
                    neighb_tets = mesh.getTetTetNeighb(check_tet)
                    for t in neighb_tets:
                        if t == -1: continue
                        if mapped_list[t] == None:
                            unmap_set.add(t)
                            expension.add(t)
                        else:
                            found_mapping = mapped_list[t]
                            can_expend = False
                            break
                check_set = expension
            for t in unmap_set:
                assert(mapped_list[t] == None)
                mapped_list[t] = found_mapping

    tet_part_table = {}
    for tet in range(len(mapped_list)):
        if mapped_list[tet] not in tet_part_table.keys():
            tet_part_table[mapped_list[tet]] = []
        tet_part_table[mapped_list[tet]].append(tet)
        
    for part in tet_part_table.values():
        for tet in part:
            found_neighbor_parts = set([])
            neighb_tets = mesh.getTetTetNeighb(tet)
            neighb_in_part = False
            for n_tet in neighb_tets:
                if n_tet == -1: continue
                found_neighbor_parts.add(mapped_list[n_tet])
                if n_tet in part:
                    neighb_in_part = True
                    break
            if not neighb_in_part:
                print("Tetrahedron %i has no neighbor in its partition. Try to reassign." % (tet))
                found_neighbor_parts = list(found_neighbor_parts)
                if len(found_neighbor_parts) > 1:
                    print "Multiple reassignment options: ", found_neighbor_parts, " choose the first one by default."
                mapped_list[tet] = found_neighbor_parts[0]
    return mapped_list
