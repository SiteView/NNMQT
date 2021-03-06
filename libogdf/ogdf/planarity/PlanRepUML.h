/*
 * $Revision: 1.9 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-01-17 22:56:41 +0100 (Thu, 17 Jan 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class PlanRepUML.
 * 
 * \author Carsten Gutwenger
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 * Copyright (C) 2005-2007
 * 
 * \par
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 or 3 as published by the Free Software Foundation
 * and appearing in the files LICENSE_GPL_v2.txt and
 * LICENSE_GPL_v3.txt included in the packaging of this file.
 *
 * \par
 * In addition, as a special exception, you have permission to link
 * this software with the libraries of the COIN-OR Osi project
 * (http://www.coin-or.org/projects/Osi.xml), all libraries required
 * by Osi, and all LP-solver libraries directly supported by the
 * COIN-OR Osi project, and distribute executables, as long as
 * you follow the requirements of the GNU General Public License
 * in regard to all of the software in the executable aside from these
 * third-party libraries.
 * 
 * \par
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * \par
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 * 
 * \see  http://www.gnu.org/copyleft/gpl.html
 ***************************************************************/


#ifdef _MSC_VER
#pragma once
#endif

#ifndef OGDF_PLAN_REP_UML_H
#define OGDF_PLAN_REP_UML_H



#include <ogdf/planarity/PlanRep.h>
#include <ogdf/basic/UMLGraph.h>

#include <ogdf/planarity/EdgeTypePatterns.h> 
#include <ogdf/planarity/NodeTypePatterns.h>



namespace ogdf {

	class OGDF_EXPORT Layout;
	class OGDF_EXPORT GridLayoutMapped;
	class OGDF_EXPORT OrthoRep;

 
//---------------------------------------------------------
// PlanRepUML
// Planarized representation (of a connected component)
// of a UMLGraph; allows special handling of hierarchies 
// in the graph
//---------------------------------------------------------
class OGDF_EXPORT PlanRepUML : public PlanRep {
public:

	// construction
	PlanRepUML(const UMLGraph &umlGraph);
	PlanRepUML(const GraphAttributes &GA);

	// deconstruction
	~PlanRepUML() {}

	void initCC(int i);	

	// Returns true if an edge splits a face into two subfaces to 
	// guarantee generalizations to be on opposite sides of a node. 
	bool faceSplitter(edge e) const{
		return m_faceSplitter[e];
	}

	// Removes all face splitting edges.
	void removeFaceSplitter(){
		edge e;
		forall_edges(e,(*this))
			if (m_faceSplitter[e])
				delEdge(e);
	}

	//-------------------
	//incremental drawing
	//-------------------
	//initialize incremental stuff, e.g. insert inremental mergers
	void setupIncremental(int indexCC, CombinatorialEmbedding &E);
	//Return the list of inserted incremental mergers
	const SList<node>&  incrementalMergers(int indexCC) const {return m_incMergers[indexCC];};
	

	//**********************************************************
	//set generic types
  
	//the edges that are embedded next to outgoing generalizations if alignment set
	//attention: this information is NOT updated during graph changes and only
	//to be used during the embedding phase
	bool alignUpward(adjEntry ae) {return m_alignUpward[ae];}
	void alignUpward(adjEntry ae, bool b) {m_alignUpward[ae] = b;}

	
	//*************************************************************************


	const UMLGraph &getUMLGraph() const {
		return *m_pUmlGraph;
	}

	//*************************************************************************
	//structural alterations

	// inserts a generalization merge node for all incoming
	// generalizations of v and returns it
	//conserving embedding
	node insertGenMerger(node v, const SList<edge> &inGens,
								 CombinatorialEmbedding &E);

	// Expands nodes with degree > 4 and merge nodes for generalizations
	void expand(bool lowDegreeExpand = false);

	//expands nodes with degree <= 4 and aligns opposite edges at degree 2 nodes
	void expandLowDegreeVertices(OrthoRep &OR, bool alignSmallDegree = false);

	void collapseVertices(const OrthoRep &OR, Layout &drawing);

	//*************************************************************************


	//
	// extension of methods defined by GraphCopy/PlanRep
	//

	// splits edge e
	virtual edge split(edge e) {
        
		edge eNew = PlanRep::split(e);
		
		//check this
		if (m_alignUpward[e->adjSource()]) m_alignUpward[eNew->adjSource()] = true;
		if (m_alignUpward[e->adjTarget()]) m_alignUpward[eNew->adjTarget()] = true;
        
		return eNew;
	}


	// writes attributed graph in GML format to file fileName
	// For Debugging only
	void writeGML(const char *fileName, const Layout &drawing);
    void writeGML(const char *fileName);
	void writeGML(const char *fileName, GraphAttributes &AG);

	// writes attributed graph in GML format to output stream os
	// For Debugging only
	void writeGML(ostream &os, const Layout &drawing);
	void writeGML(const char *fileName, const OrthoRep &OR, const Layout &drawing);
	void writeGML(ostream &os, const OrthoRep &OR, const Layout &drawing);
	void writeGML(const char *fileName, const OrthoRep &OR, const GridLayoutMapped &drawing);
	void writeGML(ostream &os, const OrthoRep &OR, const GridLayoutMapped &drawing);

        
protected:
	//insert mergers of generalizations in copy
	void prepareIncrementalMergers(int indexCC, CombinatorialEmbedding &E);

   
protected:
	//still some AdjEntry type: used by alignment procedures
	//attention: this information is NOT updated during graph changes and only
	//to be used during the embedding phase
	AdjEntryArray<bool> m_alignUpward;

private:
	const UMLGraph *m_pUmlGraph;
	
	EdgeArray<bool>     m_faceSplitter;

	SListPure<edge>       m_mergeEdges;
	Array< SList<node> >  m_incMergers; //stores all incremental mergers in CC
};


} // end namespace ogdf

#endif
