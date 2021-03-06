/*
 * $Revision: 1.1.1.1 $
 * 
 * last checkin:
 *   $Author: wong $ 
 *   $Date: 2008-09-22 23:55:21 +1000 (Mon, 22 Sep 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of interface for planar layout algorithms for
 *        UML diagrams (used in planarization approach).
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

#ifndef OGDF_UML_PLANAR_LAYOUT_MODULE_H
#define OGDF_UML_PLANAR_LAYOUT_MODULE_H



#include <ogdf/planarity/PlanRepUML.h>
#include <ogdf/basic/Layout.h>



namespace ogdf {

	
enum UMLOpt {umlOpAlign = 0x0001, umlOpScale = 0x0002, umlOpProg = 0x0004};


/**
 * \brief Interface for planar UML layout algorithms.
 *
 * \see PlanarizationLayout
 */
class OGDF_EXPORT LayoutPlanRepModule {
public:
	//! Initializes a UML planar layout module.
	LayoutPlanRepModule() { }

	virtual ~LayoutPlanRepModule() { }

	/** \Brief Computes a planar layout of \a PG in \a drawing.
	 *
	 * Must be overridden by derived classes.
	 * @param PG is the input planarized representation which may be modified.
	 * @param adjExternal is an adjacenty entry on the external face.
	 * @param drawing is the computed layout of \a PG.
	 */
	virtual void call(PlanRepUML &PG,
		adjEntry adjExternal,
		Layout &drawing) = 0;

	//! Computes a planar layout of \a PG in \a drawing.
	void operator()(PlanRepUML &PG, adjEntry adjExternal, Layout &drawing) {
		call(PG,adjExternal,drawing);
	}

	//! Returns the bounding box of the computed layout.
	const DPoint &getBoundingBox() const {
		return m_boundingBox;
	}

	//! Sets the (generic) options; derived classes have to cope with the interpretation)
	virtual void setOptions(int /* optionField */) { } //don't make it abstract

	//! Returns the (generic) options.
	virtual int getOptions() { return 0; } //don't make it abstract

	//! Returns the minimal allowed distance between edges and vertices.
	virtual double separation() const = 0;

	//! Sets the minimal allowed distance between edges and vertices to \a sep.
	virtual void separation(double sep) = 0;

protected:
	/**
	 * \brief Stores the bounding box of the computed layout.
	 * <b>Must be set by derived algorithms!</b>
	 */
	DPoint m_boundingBox;

	/**
	 * \brief Computes and sets the bounding box variable \a m_boundingBox.
	 * An algorithm can call setBoundingBox() for setting the
	 * m_boundingBox variable if no faster implementation is available.
	 */
	void setBoundingBox(PlanRepUML &PG, Layout &drawing);

	OGDF_MALLOC_NEW_DELETE
};


} // end namespace ogdf


#endif
