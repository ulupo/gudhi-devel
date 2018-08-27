/*    This file is part of the Gudhi Library. The Gudhi library
 *    (Geometric Understanding in Higher Dimensions) is a generic C++
 *    library for computational topology.
 *
 *    Author(s):       Hannah Schreiber
 *
 *    Copyright (C) 2018  TU Graz (Austria)
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TOWER_CONVERTER_H
#define TOWER_CONVERTER_H

/** @file tower_converter.h
 * @brief Contains @ref Gudhi::tower_to_filtration::Tower_converter class.
 */

#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>

namespace Gudhi {
namespace tower_to_filtration {

template<class ComplexStructure>
/**
 * @brief Takes the elementary operations of a tower in order and convert them into an equivalent filtration.
 */
class Tower_converter
{
public:
    using vertex = typename ComplexStructure::vertex;	    /**< Type for vertex identifiers. */
    using index = typename ComplexStructure::index;	    /**< Type for simplex identifiers. */
    using size_type = typename ComplexStructure::size_type; /**< Type for size mesure. */
    using simplex_base = typename std::vector<vertex>;	    /**< Type for simplices. */

    /**
     * @brief Callback function model for output.
     * @param complex pointer to the stored complex.
     * @param simplex vector of vertex identifiers of the next simplex in the filtration.
     * @param timestamp filtration value of the next simplex in the filtration.
     */
    typedef void (*process_output)(ComplexStructure *complex, std::vector<vertex> &simplex, double timestamp);

    Tower_converter();
    Tower_converter(process_output outputFunction);
    ~Tower_converter();

    bool add_insertion(simplex_base &simplex, double timestamp);
    bool add_insertion(simplex_base &simplex, double timestamp, std::vector<index> *simplexBoundary, index *simplexInsertionNumber);
    bool add_insertions_via_edge_expansion(vertex u, vertex v, double timestamp, int maxExpDim = -1);
    bool add_insertions_via_edge_expansion(vertex u, vertex v, double timestamp, int maxExpDim,
					   std::vector<simplex_base> *addedSimplices, std::vector<std::vector<index>*> *boundaries, std::vector<index> *insertionNumbers);
    index add_contraction(vertex v, vertex u, double timestamp);
    index add_contraction(vertex v, vertex u, double timestamp, std::vector<std::vector<index>*> *addedBoundaries, std::vector<index> *removedIndices);

    ComplexStructure *get_complex() const;
    size_type get_filtration_size() const;
    size_type get_tower_width() const;
    void print_filtration_data();

private:
    ComplexStructure *complex_;                     /**< Current complex. */
    std::unordered_map<vertex, vertex> *vertices_;  /**< Current vertices in the complex. Keeps the coherence between vertex identifiers outside and inside the class. */
    process_output outputFunction_;		    /**< Output function. */
    size_type filtrationSize_;                      /**< Current filtration size. */
    size_type towerWidth_;                          /**< Current tower width. */

    void get_union(vertex v, std::vector<simplex_base*> *simplices);
    void stream_simplex(simplex_base &simplex, double timestamp);
};

template<class ComplexStructure>
/**
 * @brief Constructor without parameters.
 *
 * Initializes the members. The output option is set as "no output".
 */
Tower_converter<ComplexStructure>::Tower_converter() : outputFunction_(nullptr), filtrationSize_(0), towerWidth_(0)
{
    vertices_ = new std::unordered_map<vertex, vertex>();
    complex_ = new ComplexStructure();
}

template<class ComplexStructure>
/**
 * @brief Full constructor.
 *
 * Initializes the members. The output stream will be redirected to @p outputFunction.
 *
 * @param outputFunction pointer to a callback function which process the filtration output stream.
 */
Tower_converter<ComplexStructure>::Tower_converter(process_output outputFunction) : outputFunction_(outputFunction), filtrationSize_(0), towerWidth_(0)
{
    vertices_ = new std::unordered_map<vertex, vertex>();
    complex_ = new ComplexStructure();
}

template<class ComplexStructure>
/**
 * @brief Destructor
 */
Tower_converter<ComplexStructure>::~Tower_converter()
{
    delete vertices_;
    delete complex_;
}

template<class ComplexStructure>
/**
 * @brief Add an elementary insertion as the next tower operation and convert it into the output stream.
 * @param simplex simplex to be inserted, represented as a vector of its vertex identifiers in increasing order.
 * @param timestamp time value or filtration value which will be associated to the operation in the filtration. Has to be equal or higher to the precedent ones.
 * @return true if the simplex was not already inserted in the complex, false otherwise.
 */
inline bool Tower_converter<ComplexStructure>::add_insertion(simplex_base &simplex, double timestamp)
{
    return add_insertion(simplex, timestamp, nullptr, nullptr);
}

template<class ComplexStructure>
/**
 * @brief Add an elementary insertion as the next tower operation and convert it into the output stream.
 * @param simplex simplex to be inserted, represented as a vector of its vertex identifiers in increasing order.
 * @param timestamp time value or filtration value which will be associated to the operation in the filtration. Has to be equal or higher to the precedent ones.
 * @param simplexBoundary pointer to an (empty) vector, where the identifiers of the boundary of the inserted simplex will be stored.
 * @param simplexInsertionNumber pointer to an identifier, which will be replaced by the one of the inserted simplex.
 * @return true if the simplex was not already inserted in the complex, false otherwise.
 */
bool Tower_converter<ComplexStructure>::add_insertion(simplex_base &simplex, double timestamp, std::vector<index> *simplexBoundary, index *simplexInsertionNumber)
{
    simplex_base transSimplex;

    if (simplex.size() == 1){
	vertices_->emplace(simplex.at(0), simplex.at(0));
	transSimplex.push_back(simplex.at(0));
    } else {
	for (typename simplex_base::size_type i = 0; i < simplex.size(); i++){
	    transSimplex.push_back(vertices_->at(simplex.at(i)));
        }
        std::sort(transSimplex.begin(), transSimplex.end());
    }

    if (complex_->insert_simplex(transSimplex)) {
	stream_simplex(transSimplex, timestamp);
        if (complex_->get_size() > towerWidth_) towerWidth_ = complex_->get_size();
	if (simplexBoundary != nullptr) *simplexInsertionNumber = complex_->get_boundary(transSimplex, simplexBoundary);
        return true;
    }
    return false;
}

template<class ComplexStructure>
bool Tower_converter<ComplexStructure>::add_insertions_via_edge_expansion(vertex u, vertex v, double timestamp, int maxExpDim)
{
    return add_insertions_via_edge_expansion(u, v, timestamp, maxExpDim, nullptr, nullptr, nullptr);
}

template<class ComplexStructure>
bool Tower_converter<ComplexStructure>::add_insertions_via_edge_expansion(vertex u, vertex v, double timestamp, int maxExpDim, std::vector<simplex_base> *addedSimplices,
									  std::vector<std::vector<index>*> *boundaries, std::vector<index> *insertionNumbers)
{
    std::vector<simplex_base> *insertedSimplices;
    vertex first = u;
    vertex second = u;
    if (u < v) second = v;
    else first = v;

    vertices_->emplace(u, u);
    vertices_->emplace(v, v);

    if (addedSimplices == nullptr) insertedSimplices = new std::vector<simplex_base>();
    else insertedSimplices = addedSimplices;

    if (complex_->insert_edge_and_expand(first, second, maxExpDim, insertedSimplices, insertionNumbers, boundaries)){
	for (simplex_base simplex : *insertedSimplices){
	    stream_simplex(simplex, timestamp);
	}

	if (complex_->get_size() > towerWidth_) towerWidth_ = complex_->get_size();

	if (addedSimplices == nullptr) delete insertedSimplices;
	return true;
    }

    if (addedSimplices == nullptr) delete insertedSimplices;
    return false;
}

template<class ComplexStructure>
/**
 * @brief Add an elementary contraction as the next tower operation and convert it into a equivalent sequence of insertions into the output stream.
 * @param v identifier of the contracted vertex which disappears from the complex.
 * @param u identifier of the contracted vertex which remains in the complex.
 * @param timestamp time value or filtration value which will be associated to the operation in the filtration. Has to be equal or higher to the precedent ones.
 * @exception std::out_of_range If @p v or @p u is not an existing identifier in the current complex ;
 *	Therefore be careful with the order of @p v and @p u to keep coherence with futur contractions.
 * @return The identifier of the first new simplex in the equivalent insertion ; the remaining new simplices will take the identifiers which follows continuously.
 */
inline typename Tower_converter<ComplexStructure>::index Tower_converter<ComplexStructure>::add_contraction(vertex v, vertex u, double timestamp)
{
    return add_contraction(v, u, timestamp, nullptr, nullptr);
}

template<class ComplexStructure>
/**
 * @brief Add an elementary contraction as the next tower operation and convert it into a equivalent sequence of insertions into the output stream.
 * @param v identifier of the contracted vertex which disappears from the complex.
 * @param u identifier of the contracted vertex which remains in the complex.
 * @param timestamp time value or filtration value which will be associated to the operation in the filtration. Has to be equal or higher to the precedent ones.
 * @param addedBoundaries pointer to an (empty) vector, where the boundaries of the inserted simplices will be stored.
 * @param removedIndices pointer to an (empty) vector, where the identifiers of the simplices which become inactive will be stored.
 * @exception std::out_of_range If @p v or @p u is not an existing identifier in the current complex ;
 *	Therefore be careful with the order of @p v and @p u to keep coherence with futur contractions.
 * @return The identifier of the first new simplex in the equivalent insertion ; the remaining new simplices will take the identifiers which follows continuously.
 */
typename Tower_converter<ComplexStructure>::index Tower_converter<ComplexStructure>::add_contraction(vertex v, vertex u, double timestamp,
									  std::vector<std::vector<index>*> *addedBoundaries, std::vector<index> *removedIndices)
{
    std::vector<simplex_base*> closedStar;
    vertex tv = vertices_->at(v), tu = vertices_->at(u);
    vertex dis = complex_->get_smallest_closed_star(tv, tu, &closedStar);
    simplex_base vdis(1, dis);
    index first = -1;

    vertices_->erase(v);
    if (dis == tu){
        vertices_->at(u) = tv;
        get_union(tv, &closedStar);
    } else {
        get_union(tu, &closedStar);
    }

    for (auto it = closedStar.begin(); it != closedStar.end(); it++){
	if (complex_->insert_simplex(**it)) {
	    stream_simplex(**it, timestamp);
	    if (addedBoundaries != nullptr){
                std::vector<index> *boundary = new std::vector<index>();
                if (first == -1) first = complex_->get_max_index();
		complex_->get_boundary(**it, boundary);
                addedBoundaries->push_back(boundary);
            }
        }
        delete *it;
    }
    complex_->remove_simplex(vdis, removedIndices);

    if (complex_->get_size() > towerWidth_) towerWidth_ = complex_->get_size();

    return first;
}

template<class ComplexStructure>
/**
 * @brief Returns the current size of the filtration.
 * @return The current size of the filtration.
 */
inline typename Tower_converter<ComplexStructure>::size_type Tower_converter<ComplexStructure>::get_filtration_size() const
{
    return filtrationSize_;
}

template<class ComplexStructure>
/**
 * @brief Returns the maximal size of the complex until now.
 * @return The maximal size of the complex until now.
 */
inline typename Tower_converter<ComplexStructure>::size_type Tower_converter<ComplexStructure>::get_tower_width() const
{
    return towerWidth_;
}

template<class ComplexStructure>
/**
 * @brief Prints various information about the filtration in the terminal.
 *
 * Those are: filtration size, maximal size of a complex, maximal dimension of a simplex, tower width.
 */
inline void Tower_converter<ComplexStructure>::print_filtration_data()
{
    std::cout << std::setprecision(std::numeric_limits<double>::digits10 + 1) << "Filtration Size: " << filtrationSize_ << "\n";
    std::cout << "Max Size: " << complex_->get_max_size() << "\n";
    std::cout << "Max Dimension: " << complex_->get_max_dimension() << "\n";
    std::cout << "Tower Width: " << towerWidth_ << "\n";
}

template<class ComplexStructure>
/**
 * @brief Return pointer to the stored complex.
 * @return The pointer to the stored complex.
 */
inline ComplexStructure *Tower_converter<ComplexStructure>::get_complex() const
{
    return complex_;
}

template<class ComplexStructure>
/**
 * @brief Make the union of a set of simplices and a vertex.
 * @param v vertex identifier to unify.
 * @param simplices vector of simplices to unify with @p v. The simplices will be replaced by the resulting simplices.
 */
void Tower_converter<ComplexStructure>::get_union(vertex v, std::vector<simplex_base*> *simplices)
{
    for (auto itSimplices = simplices->begin(); itSimplices != simplices->end(); itSimplices++){
        auto itVertices = (*itSimplices)->begin();
        while (itVertices != (*itSimplices)->end() && *itVertices < v) itVertices++;
        if ((itVertices != (*itSimplices)->end() && *itVertices != v) || itVertices == (*itSimplices)->end()){
            (*itSimplices)->insert(itVertices, v);
        }
    }
}

template<class ComplexStructure>
/**
 * @brief Writes the simplex as an insertion in the output stream.
 * @param simplex simplex to be inserted.
 * @param timestamp filtration value of the insertion.
 */
void Tower_converter<ComplexStructure>::stream_simplex(simplex_base &simplex, double timestamp)
{
    filtrationSize_++;
    if (outputFunction_ != nullptr) outputFunction_(complex_, simplex, timestamp);
}

}
}

#endif // TOWER_CONVERTER_H
