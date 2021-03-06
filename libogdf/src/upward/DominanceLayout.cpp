
#include <ogdf/upward/DominanceLayout.h>
#include <math.h>

namespace ogdf {
	
void DominanceLayout::call(ogdf::GraphAttributes &GA) {
	if (GA.constGraph().numberOfNodes() <= 1)
		return;

	//call upward planarizer
	UpwardPlanRep UPR;
	UPR.createEmpty(GA.constGraph());		
	m_upPlanarizer.get().call(UPR);	
	layout(GA, UPR);
}

void DominanceLayout::layout(GraphAttributes &GA, const ogdf::UpwardPlanRep &UPROrig) {	
	
	UpwardPlanRep UPR = UPROrig;
	
	//clear some data
	edge e;
	forall_edges(e, GA.constGraph()) {
		GA.bends(e).clear();
	}

	//compute and splite transitiv edges	
	List<edge> splitMe;
	findTransitiveEdges(UPR, splitMe);
	
	forall_listiterators(edge, it, splitMe) {
		UPR.getEmbedding().split(*it);	
	}

	// set up first-/lastout, first-/lastin
	firstout.init(UPR, 0);
	lastout.init(UPR, 0);
	firstin.init(UPR, 0);
	lastin.init(UPR, 0);
	
	node s = UPR.getSuperSource();
	node t = UPR.getSuperSink();

	firstout[t] = lastout[t] = 0;
	firstin[s] = lastin[s] = 0;
	firstin[t] = lastin[t] =t->firstAdj()->theEdge();
	adjEntry adjRun = s->firstAdj();
	while (UPR.getEmbedding().rightFace(adjRun) != UPR.getEmbedding().externalFace()) {
		adjRun = adjRun->cyclicSucc();
	}
	lastout[s] = adjRun->theEdge();
	firstout[s] = adjRun->cyclicSucc()->theEdge();

	node v;
	forall_nodes(v, UPR) {
		if (v == t || v == s) continue;

		adjEntry adj = UPR.leftInEdge(v);
		firstin[v] = adj->theEdge();
		firstout[v] = adj->cyclicSucc()->theEdge();

		adjEntry adjRightIn = adj;
		while (adjRightIn->cyclicPred()->theEdge()->source() != v)
			adjRightIn = adjRightIn->cyclicPred();
		
		lastin[v] = adjRightIn->theEdge();
		lastout[v] = adjRightIn->cyclicPred()->theEdge();	
	}

	
	//compute m_L and m_R for min. area drawing
	m_L = 0;
	m_R = 0;
	forall_edges(e, UPR) {
		node src = e->source();
		node tgt = e->target();
		if (lastin[tgt] == e && firstout[src] == e)
			m_L++;
		if (firstin[tgt] == e && lastout[src] == e)
			m_R++;
	}
	
	// compute preleminary coordinate
	xPreCoord.init(UPR);
	yPreCoord.init(UPR);	
	int count = 0; 
	labelX(UPR, s, count);
	count = 0;
	labelY(UPR, s, count);	
	
	// compaction
	compact(UPR, GA);

	// map coordinate to GA
	forall_nodes(v, GA.constGraph()) {
		node vUPR = UPR.copy(v);
		GA.x(v) = xCoord[vUPR];
		GA.y(v) = yCoord[vUPR];
	}
	// add bends to original edges
	forall_edges(e, GA.constGraph()) {
		List<edge> chain = UPR.chain(e);
		forall_nonconst_listiterators(edge, it, chain) {
			node tgtUPR = (*it)->target();
			if (tgtUPR != chain.back()->target()) {
				DPoint p(xCoord[tgtUPR], yCoord[tgtUPR]);
				GA.bends(e).pushBack(p);
			}
		}
	}
	
	
	//rotate the drawing
	forall_nodes(v, GA.constGraph()) {		
		double r = sqrt(GA.x(v)*GA.x(v) + GA.y(v)*GA.y(v));
		if (r == 0)
			continue;
		double alpha = asin(GA.y(v)/r);
		double yNew = sin(alpha + m_angle)*r;
		double xNew = cos(alpha + m_angle)*r;
		GA.x(v) = xNew;
		GA.y(v) = yNew;			
	}  

	forall_edges(e, GA.constGraph()) {
		DPolyline &poly = GA.bends(e);
		DPoint pSrc(GA.x(e->source()), GA.y(e->source()));
		DPoint pTgt(GA.x(e->target()), GA.y(e->target()));
		poly.normalize(pSrc, pTgt);
		
		forall_nonconst_listiterators(DPoint, it, poly) {
			double r = (*it).distance(DPoint(0,0));
			
			if (r == 0)
				continue;

			double alpha = asin( (*it).m_y/r);
			double yNew = sin(alpha + m_angle)*r;
			double xNew = cos(alpha + m_angle)*r;
			(*it).m_x = xNew;
			(*it).m_y = yNew;
		}
		
	}
	
}


void DominanceLayout::labelX(const UpwardPlanRep &UPR, node v, int &count) {
	xNodes.pushBack(v);
	xPreCoord[v] = count;
	count++;
	if (v != UPR.getSuperSink()) {		
		adjEntry adj = firstout[v]->adjSource();
		do {
			node w = adj->theEdge()->target();
			if (adj->theEdge() == lastin[w])
				labelX(UPR, w, count);
			adj = adj->cyclicSucc();
		} while (adj->cyclicPred()->theEdge() != lastout[v]);
	}	
}




void DominanceLayout::labelY(const UpwardPlanRep &UPR, node v, int &count) {
	yNodes.pushBack(v);
	yPreCoord[v] = count;
	count++;
	if (v != UPR.getSuperSink()) {		
		adjEntry adj = lastout[v]->adjSource();
		do {
			node w = adj->theEdge()->target();
			if (adj->theEdge() == firstin[w])
				labelY(UPR, w, count);
			adj = adj->cyclicPred();
		} while (adj->cyclicSucc()->theEdge() != firstout[v]);
	}
}


void DominanceLayout::compact(const UpwardPlanRep &UPR, GraphAttributes &GA) {
	double maxNodeSize = 0;
	node v;
	forall_nodes(v, GA.constGraph()) {
		if (GA.width(v) > maxNodeSize || GA.height(v) > maxNodeSize)
			maxNodeSize = max(GA.width(v), GA.height(v));
	}
	
	int gridDist = m_grid_dist;
	if (gridDist < maxNodeSize+1)
		gridDist = (int) maxNodeSize+1;

	xCoord.init(UPR);
	yCoord.init(UPR);

	//ASSIGN X COORDINATE
	
	OGDF_ASSERT(!xNodes.empty());

	v = xNodes.popFrontRet();	
	xCoord[v] = 0;
	while (!xNodes.empty()) {
		node u = xNodes.popFrontRet(); 	
		if ( (yPreCoord[v] > yPreCoord[u]) || (firstout[v] == lastout[v] && firstin[u] == lastin[u] && m_L <= m_R)) {
			xCoord[u] = xCoord[v] + gridDist;			
		}
		else
			xCoord[u] = xCoord[v];
		v = u;
	}
	
	//ASSIGN Y COORDINATE
	OGDF_ASSERT(!yNodes.empty());

	v = yNodes.popFrontRet();
	yCoord[v] = 0;
	while (!yNodes.empty()) {
		node u = yNodes.popFrontRet(); 		
		if ( (xPreCoord[v] > xPreCoord[u]) || (firstout[v] == lastout[v] && firstin[u] == lastin[u] && m_L > m_R)) {
			yCoord[u] = yCoord[v] + gridDist;			
		}
		else
			yCoord[u] = yCoord[v];
		v = u;
	}	
}



void DominanceLayout::findTransitiveEdges(const UpwardPlanRep &UPR, List<edge> &edges)
{
	// for st-graphs:
	// e = (u,v) transitive <=> ex. face f: e in f and u source-switch and v = sink-switch
	face f;
	forall_faces(f, UPR.getEmbedding()) {
		if (f == UPR.getEmbedding().externalFace())
			continue;
		
		adjEntry adj;
		forall_face_adj(adj, f) {
			node src = adj->theEdge()->source();
			node tgt = adj->theEdge()->target();
			if ( (adj->faceCycleSucc()->theEdge()->source() == src && adj->faceCyclePred()->theEdge()->target() == tgt)
				|| (adj->faceCycleSucc()->theEdge()->target() == tgt && adj->faceCyclePred()->theEdge()->source() == src)) {
					edges.pushBack(adj->theEdge());
					break;
			}
		}
	}	
}


}//namespace
