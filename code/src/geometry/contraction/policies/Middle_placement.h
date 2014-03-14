/*
 * Middle_placement.h
 *
 *  Created on: Feb 13, 2014
 *      Author: dsalinas
 */

#ifndef GUDHI_MIDDLE_PLACEMENT_H_
#define GUDHI_MIDDLE_PLACEMENT_H_


namespace contraction {

template< typename EdgeProfile> class Middle_placement : public Placement_policy<EdgeProfile>{

public:
	typedef typename EdgeProfile::Point Point;
	typedef typename EdgeProfile::edge_descriptor edge_descriptor;
	typedef typename EdgeProfile::Graph_vertex Graph_vertex;

	typedef typename Placement_policy<EdgeProfile>::Placement_type Placement_type;

	Placement_type operator()(const EdgeProfile& profile){
		//todo compute the middle
		return Placement_type(profile.p0());
	}
};
}  // namespace contraction


#endif /* GUDHI_MIDDLE_PLACEMENT_H_ */
