/*    This file is part of the Gudhi Library. The Gudhi library
 *    (Geometric Understanding in Higher Dimensions) is a generic C++
 *    library for computational topology.
 *
 *    Author(s):       Pawel Dlotko
 *
 *    Copyright (C) 2015  INRIA (France)
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

#ifndef Persistence_intervals_H_
#define Persistence_intervals_H_

//gudhi include
#include <gudhi/abstract_classes/Abs_Vectorized_topological_data.h>
#include <gudhi/abstract_classes/Abs_Topological_data_with_distances.h>
#include <gudhi/abstract_classes/Abs_Real_valued_topological_data.h>
#include <gudhi/concretizations/read_persitence_from_file.h>

//standard include
#include <limits>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <climits>
#include <cmath>

namespace Gudhi 
{
namespace Gudhi_stat 
{

class Persistence_intervals : public Abs_Vectorized_topological_data , public Abs_Topological_data_with_distances, public Abs_Real_valued_topological_data
{
public:	   
	/**
	 * This is a constructor of a class Persistence_intervals from a text file. Each line of the input file is supposed to contain two numbers of a type doube (or convertable to double)
	 * representing the birth and the death of the persistence interval. If the pairs are not sorted so that birth <= death, then the constructor will sort then that way. 
	**/ 
    Persistence_intervals( const char* filename );
    
    /**
     * This is a constructor of a class Persistence_intervals from a vector of pairs. Each pair is assumed to represent a persistence interval. We assume that the first elemnets of pairs 
     * are smaller or equal the second elements of pairs.
    **/ 
    Persistence_intervals( const std::vector< std::pair< double,double > >& intervals );

	/**
	 * The procedure returns a pair the first element of which is the leftmost end of the interval, and the second element of which is the rightmost end of the interval.  
	**/
    std::pair<double,double> min_max();

	/**
	 * Procedure that compute the vector of lengths of the dominant (i.e. the longest) persistence intervals. The list is truncated at the parameter of the call where_to_cut (set by default to 100).
	**/ 
    std::vector<double> length_of_dominant_intervals( size_t where_to_cut = 100 );
    
    
    /**
	 * Procedure that compute the vector of the dominant (i.e. the longest) persistence intervals. The parameter of the procedure (set by default to 100) is the number of dominant intervals returned by the procedure.
	**/ 
    std::vector< std::pair<double,double> > dominant_intervals( size_t where_to_cut = 100 );
    
    /**
	 * Procedure to compute a histogram of interva's length. A histogram is a block plot. The number of blocks is determined by the first parameter of the function (set by default to 10). 
	 * For the sake of argument let us assume that the length of the longest interval is 1 and the number of bins is 10. In this case the i-th block correspond to a range between i-1/10 and i10. 
	 * The vale of a block supported at the interval is the number of persistence intervals of a length between x_0 and x_1.
	**/
    std::vector< size_t > histograms_of_lengths( size_t number_of_bins = 10  );
    
    /**
     * Based on a histogram of intervals lengts computed by the function histograms_of_lengths H the procedure below computes the cumulative histogram. The i-th position of the resulting histogram
     * is the sume of values of H for the positions from 0 to i. 
    **/
    std::vector< size_t > cumulative_histograms_of_lengths( size_t number_of_bins = 10 );
    
    /**
     * In this procedure we assume that each barcode is a characteristic function of a hight equal to its length. The persistence diagram is a sum of such a functions. The procedure below construct a function being a 
     * sum of the characteristic functions of persistence intervals. The first two parameters are the range in which the function is to be computed and the last parameter is the number of bins in 
     * the discretization of the interval [_min,_max].
    **/ 
    std::vector< double > characteristic_function_of_diagram( double x_min , double x_max , size_t number_of_bins = 10 );
    
    /**
     * Cumulative version of the function characteristic_function_of_diagram
    **/ 
    std::vector< double > cumulative_characteristic_function_of_diagram( double x_min , double x_max , size_t number_of_bins = 10 );
    
    /**
     * Compute the funtion of persistence Betti numbers. The returned value is a vector of pair. First element of each pair is a place where persistence Betti numbers change. 
     * Second element of each pair is the value of Persistence Betti numbers at that point.  
    **/ 
    std::vector< std::pair< double , size_t > > compute_persistent_betti_numbers();
    
    /**
     *This is a non optimal procedure that compute vector of distances from each point of diagram to its k-th nearest neighbor (k is a parameted of the program). The resulting vector is by default truncated to 10 
     *elements (this value can be changed by using the second parameter of the program). The points are returned in order from the ones which are farthest away from their k-th nearest neighbors. 
    **/
    std::vector< double > k_n_n( size_t k  , size_t where_to_cut = 10 );

	/**
     * Operator that send the diagram to a stream. 
    **/    
    friend ostream& operator << ( ostream& out , const Persistence_intervals& intervals )
    {
        for ( size_t i = 0 ; i != intervals.intervals.size() ; ++i )
        {
            out << intervals.intervals[i].first << " " << intervals.intervals[i].second << endl;
        }
        return out;
    }
    
    /**
     * Generating gnuplot script to plot the interval. 
    **/
    void plot( const char* filename ) 
    {
		//this program create a gnuplot script file that allows to plot persistence diagram.
		ofstream out;
		
		std::ostringstream nameSS;
		nameSS << filename << "_GnuplotScript";
		std::string nameStr = nameSS.str();
		out.open( (char*)nameStr.c_str() );
		std::pair<double,double> min_max_values = this->min_max();
		out << "set xrange [" << min_max_values.first - 0.1*(min_max_values.second-min_max_values.first) << " : " << min_max_values.second + 0.1*(min_max_values.second-min_max_values.first) << " ]" << endl;
		out << "set yrange [" << min_max_values.first - 0.1*(min_max_values.second-min_max_values.first) << " : " << min_max_values.second + 0.1*(min_max_values.second-min_max_values.first) << " ]" << endl;
		out << "plot '-' using 1:2 notitle \"" << filename << "\", \\" << endl;
		out << "     '-' using 1:2 notitle with lp" << endl;
		for ( size_t i = 0 ; i != this->intervals.size() ; ++i )
		{
			out << this->intervals[i].first << " " << this->intervals[i].second << endl;
		}
		out << "EOF" << endl;
		out << min_max_values.first - 0.1*(min_max_values.second-min_max_values.first) << " " << min_max_values.first - 0.1*(min_max_values.second-min_max_values.first) << endl;
		out << min_max_values.second + 0.1*(min_max_values.second-min_max_values.first) << " " << min_max_values.second + 0.1*(min_max_values.second-min_max_values.first) << endl;
			
		out.close();
		
		cout << "Gnuplot script to visualize persistence diagram written to the file: " << nameStr << ". Type load '" << nameStr << "' in gnuplot to visualize." << endl;
	}

	/**
     * Return a familly of vectors obtained from the persistence diagram. The i-th vector consist of the lenfth of i dominant persistence intervals. 
    **/
    std::vector<double> vectorize( int number_of_function )//comment: in this case, number_of_functions (a static member of Abs_Vectorized_topological_data cannot be set a priori. 
															//In this cas, maybe it is better not to make this parameter static.
    {
        return this->length_of_dominant_intervals( number_of_function );
    }

	/**
     * Retun numbr of points in the diagram.
    **/
    size_t size()const{return this->intervals.size();}
    
    /**
     * Return the persistence interval at the given position. Note that intervals are not sorted with respect to their lengths. 
    **/
    inline std::pair< double,double > operator [](size_t i)
    {
        if ( i >= this->intervals.size() )throw("Index out of range! Operator [], one_d_gaussians class\n");
        return this->intervals[i];
    }
    
    /**
     *Computations of distance from the current persistnce diagram to the persistence diagram given as a parameter of this function.
     *The last parameter, power, is here in case we would like to compute p=th Wasserstein distance. At the moment, for the bottleneck distances, it will be ignored.
    **/
    double distance( const Abs_Topological_data_with_distances* second , double power = 1) 
    {
		return 1;
		//waiting for Francois Godi for the code. We will compute here the Bottleneck distnace. 
	}
	
	/**
	 * This is a simple function projectig the persistence intervals to a real number. The function we use here is a sum of squared lendgths of intervals. It can be naturally interpreted as
	 * sum of step function, where the step hight it equal to the length of the interval.
	 **/ 
	double project_to_R( int number_of_function );
	
    
    //For visualization use output from vectorize and build histograms. 
     std::vector< std::pair< double,double > > output_for_visualization()
    {
		return this->intervals;
	}

protected:
	
	void set_up_numbers_of_functions_for_vectorization_and_projections_to_reals()
	{
		//warning, this function can be only called after filling in the intervals vector.
		this->number_of_functions_for_vectorization = this->intervals.size();
		this->number_of_functions_for_projections_to_reals = 1;
	}

    std::vector< std::pair< double,double > > intervals;    
};


Persistence_intervals::Persistence_intervals( const char* filename )
{
    //bool dbg = false;
    //ifstream in;
    //in.open( filename );

    //if ( !in.good() )
    //{
    //    throw("File with the persistence diagram do not exist, the program will now terminate.\n");
    //}

    //while ( true )
    //{
    //    double first;
    //    double second;
    //    in >> first >> second;

    //    if ( first > second )
    //    {
    //        double buf = first;
    //        first = second;
    //        second = buf;
    //    }

    //    if ( in.eof() )break;
    //    this->intervals.push_back( std::make_pair( first,second ) );
    //    if ( dbg )
    //    {
    //        cerr << "Adding interval [ " << first << " , " << second << " ]\n";
    //        getchar();
    //    }
    //}
    //in.close();
    //standard file with barcode
    std::vector< std::pair< double , double > > barcode = read_standard_file( filename );    
    //gudhi file with barcode
    //std::vector< std::pair< double , double > > barcode = read_gudhi_file( filename , dimension );   
    
    this->intervals = barcode;
    this->set_up_numbers_of_functions_for_vectorization_and_projections_to_reals();
}//Persistence_intervals


Persistence_intervals::Persistence_intervals( const std::vector< std::pair< double , double > >& intervals_ )
{
	std::vector< std::pair< double,double > > aaa( intervals_ );
    this->intervals = aaa;
    this->set_up_numbers_of_functions_for_vectorization_and_projections_to_reals();
}


std::vector< double > Persistence_intervals::length_of_dominant_intervals( size_t where_to_cut )
{
	this->set_up_numbers_of_functions_for_vectorization_and_projections_to_reals();
    std::vector< double > result( this->intervals.size() );
    for ( size_t i = 0 ; i != this->intervals.size() ; ++i )
    {
        result[i] = this->intervals[i].second - this->intervals[i].first;
    }
    std::sort( result.begin() , result.end() ,  std::greater<double>() );


    result.resize( std::min(where_to_cut,result.size()) );
    return result;
}//length_of_dominant_intervals



bool compare( const std::pair< size_t , double >& first , const std::pair< size_t , double >& second )
{
	return first.second > second.second;
}


std::vector< std::pair<double,double> > Persistence_intervals::dominant_intervals( size_t where_to_cut )
{
	bool dbg = false;
	std::vector< std::pair< size_t , double > > position_length_vector( this->intervals.size() );
	for ( size_t i = 0 ; i != this->intervals.size() ; ++i )
    {
        position_length_vector[i] = std::make_pair( i , this->intervals[i].second - this->intervals[i].first );
    }
    
    std::sort( position_length_vector.begin() , position_length_vector.end() , compare );

	std::vector< std::pair<double,double> > result;
	result.reserve( std::min( where_to_cut , position_length_vector.size() ) );
	
	for ( size_t i = 0 ; i != std::min( where_to_cut , position_length_vector.size() ) ; ++i )
	{
		result.push_back( this->intervals[ position_length_vector[i].first ] );
		if ( dbg )cerr << "Position : " << position_length_vector[i].first << " length : " << position_length_vector[i].second << endl;
	}
	
	return result;
}//dominant_intervals


std::vector< size_t > Persistence_intervals::histograms_of_lengths( size_t number_of_bins )
{
    bool dbg = false;

    if ( dbg )cerr << "this->intervals.size() : " << this->intervals.size() << endl;
    //first find the length of the longest interval:
    double lengthOfLongest = 0;
    for ( size_t i = 0  ; i != this->intervals.size() ; ++i )
    {
        if ( (this->intervals[i].second - this->intervals[i].first) > lengthOfLongest )
        {
            lengthOfLongest = this->intervals[i].second - this->intervals[i].first;
        }
    }

    if ( dbg ){cerr << "lengthOfLongest : " << lengthOfLongest << endl;}

	//this is a container we will use to store the resulting histogram
    std::vector< size_t > result( number_of_bins + 1 , 0 );
    
    //for every persistence interval in our collection.
    for ( size_t i = 0  ; i != this->intervals.size() ; ++i )
    {
		//compute its length relative to the length of the dominant interval:
        double relative_length_of_this_interval = (this->intervals[i].second - this->intervals[i].first)/lengthOfLongest;
        
        //given the relative length (between 0 and 1) compute to which bin should it contribute. 
        size_t position = (size_t)(relative_length_of_this_interval*number_of_bins);
        
        
        ++result[position];
        
        if ( dbg )
        {
            cerr << "i : " << i << endl;
            cerr << "Interval : [" << this->intervals[i].first << " , " << this->intervals[i].second << " ] \n";
            cerr << "relative_length_of_this_interval : " << relative_length_of_this_interval << endl;
            cerr << "position : " << position << endl;
            getchar();
        }
    }

    
    if ( dbg ){for ( size_t i = 0 ; i != result.size() ; ++i )cerr << result[i] << endl;}
    return result;
}


std::vector< size_t > Persistence_intervals::cumulative_histograms_of_lengths( size_t number_of_bins )
{
    std::vector< size_t > histogram = this->histograms_of_lengths( number_of_bins );
    std::vector< size_t > result( histogram.size() );

    size_t sum = 0;
    for ( size_t i = 0 ; i != histogram.size() ; ++i )
    {
        sum += histogram[i];
        result[i] = sum;
    }
    return result;
}


std::vector< double > Persistence_intervals::characteristic_function_of_diagram( double x_min , double x_max , size_t number_of_bins )
{
    bool dbg = false;

    std::vector< double > result( number_of_bins );
    std::fill( result.begin() , result.end() , 0 );

    for ( size_t i = 0 ; i != this->intervals.size() ; ++i )
    {
        if ( dbg )
        {
            cerr << "Interval : " << this->intervals[i].first << " , " << this->intervals[i].second << endl;
        }

        size_t beginIt = 0;
        if ( this->intervals[i].first < x_min )beginIt = 0;
        if ( this->intervals[i].first >= x_max )beginIt = result.size();
        if ( ( this->intervals[i].first > x_min ) && ( this->intervals[i].first < x_max ) )
        {
            beginIt = number_of_bins*(this->intervals[i].first-x_min)/(x_max - x_min);
        }

        size_t endIt;
        if ( this->intervals[i].second < x_min )endIt = 0;
        if ( this->intervals[i].second >= x_max )endIt = result.size();
        if ( ( this->intervals[i].second > x_min ) && ( this->intervals[i].second < x_max ) )
        {
            endIt = number_of_bins*( this->intervals[i].second - x_min )/(x_max - x_min);
        }

        if ( beginIt > endIt ){beginIt = endIt;}

        if ( dbg )
        {
            cerr << "beginIt : " << beginIt << endl;
            cerr << "endIt : " << endIt << endl;
        }


        for ( size_t pos = beginIt ; pos != endIt ; ++pos )
        {
            result[pos] += ( (x_max - x_min)/(double)number_of_bins ) * ( this->intervals[i].second - this->intervals[i].first );
        }
        //cerr << "x_max : " << x_max << " x_min : " << x_min << " , number_of_bins : " << number_of_bins << " this->intervals[i].second : " << this->intervals[i].second <<  " this->intervals[i].first : " << this->intervals[i].first << endl;
        if ( dbg )
        {
            cerr << "Result at this stage \n";
            for ( size_t aa = 0 ; aa != result.size() ; ++aa )
            {
                cerr << result[aa] << " ";
            }
            cerr << endl;
            //getchar();
        }
    }
    return result;
}//characteristic_function_of_diagram



std::vector< double > Persistence_intervals::cumulative_characteristic_function_of_diagram( double x_min , double x_max , size_t number_of_bins )
{
    std::vector< double > intsOfBars = this->characteristic_function_of_diagram( x_min , x_max , number_of_bins );    
    std::vector< double > result( intsOfBars.size() );
    double sum = 0;
    for ( size_t i = 0 ; i != intsOfBars.size() ; ++i )
    {
        sum += intsOfBars[i];
        result[i] = sum;
    }
    return result;
}//cumulative_characteristic_function_of_diagram


template <typename T>
bool compare_first_element_of_pair( const std::pair< T , bool >& f, const std::pair< T , bool >& s )
{
    return (f.first < s.first);
}


std::vector< std::pair< double , size_t > > Persistence_intervals::compute_persistent_betti_numbers()
{	
    std::vector< std::pair< double , bool > > places_where_pbs_change( 2*this->intervals.size() );

    for ( size_t i = 0 ; i != this->intervals.size() ; ++i )
    {
        places_where_pbs_change[2*i] = std::make_pair( this->intervals[i].first , true );
        places_where_pbs_change[2*i+1] = std::make_pair( this->intervals[i].second , false );
    }

    std::sort( places_where_pbs_change.begin() , places_where_pbs_change.end() , compare_first_element_of_pair<double> );
    size_t pbn = 0;
    std::vector< std::pair< double , size_t > > pbns( places_where_pbs_change.size() );
    for ( size_t i = 0 ; i != places_where_pbs_change.size() ; ++i )
    {
        if ( places_where_pbs_change[i].second == true )
        {
            ++pbn;
        }
        else
        {
            --pbn;
        }
        pbns[i] = std::make_pair( places_where_pbs_change[i].first , pbn );
    }    
    return pbns;
}






inline double compute_euclidean_distance( const std::pair< double,double > & f , const std::pair< double,double > & s )
{
    return sqrt( (f.first-s.first)*(f.first-s.first) + (f.second-s.second)*(f.second-s.second) );
}


std::vector< double > Persistence_intervals::k_n_n( size_t k , size_t where_to_cut )
{
    bool dbg = false;
    if ( dbg )
    {
        cerr << "Here are the intervals : \n";
        for ( size_t i = 0 ; i != this->intervals.size() ; ++i )
        {
            cerr << "[ " << this->intervals[i].first << " , " << this->intervals[i].second << "] \n";
        }
        getchar();
    }

    std::vector< double > result;
    //compute all to all distance between point in the diagram. Also, consider points in the diagonal with the infinite multiplicity.
    std::vector< std::vector< double > > distances( this->intervals.size() );
    for ( size_t i = 0 ; i != this->intervals.size() ; ++i )
    {
        std::vector<double> aa(this->intervals.size());
        std::fill( aa.begin() , aa.end() , 0 );
        distances[i] = aa;
    }
    std::vector< double > distances_from_diagonal( this->intervals.size() );
    std::fill( distances_from_diagonal.begin() , distances_from_diagonal.end() , 0 );

    for ( size_t i = 0 ; i != this->intervals.size() ; ++i )
    {
        std::vector< double > distancesFromI;
        for ( size_t j = i+1 ; j != this->intervals.size() ; ++j )
        {
            distancesFromI.push_back( compute_euclidean_distance( this->intervals[i] , this->intervals[j] ) );
        }
        //distances.push_back( distancesFromI );
        //also add a distance from this guy to daigonal:
        double distanceToDiagonal = compute_euclidean_distance( this->intervals[i] , std::make_pair( 0.5*(this->intervals[i].first + this->intervals[i].second) , 0.5*(this->intervals[i].first + this->intervals[i].second) ) );
        distances_from_diagonal[i] = distanceToDiagonal;

        if ( dbg )
        {
            cerr << "Here are the distances form the point : [" << this->intervals[i].first << " , " << this->intervals[i].second << "] in the diagram \n";
            for ( size_t aa = 0 ; aa != distancesFromI.size() ; ++aa )
            {
                cerr << "To : " << i+aa << " : " << distancesFromI[aa] << " ";
            }
            cerr << endl;
            getchar();
        }

        //filling in the distances matrix:
        for ( size_t j = i+1 ; j != this->intervals.size() ; ++j  )
        {
            distances[i][j] = distancesFromI[j-i-1];
            distances[j][i] = distancesFromI[j-i-1];
        }
    }
    if ( dbg )
    {
        cerr << "Here is the distance matrix : \n";
        for ( size_t i = 0 ; i != distances.size() ; ++i )
        {
            for ( size_t j = 0 ; j != distances.size() ; ++j )
            {
                cerr << distances[i][j] << " ";
            }
            cerr << endl;
        }
        cerr << endl << endl << "And here are the distances to the diagonal : " << endl;
        for ( size_t i = 0 ; i != distances_from_diagonal. size() ; ++i )
        {
            cerr << distances_from_diagonal[i] << " ";
        }
        cerr << endl << endl;
        getchar();
    }

    for ( size_t i = 0 ; i != this->intervals.size() ; ++i )
    {
        std::vector< double > distancesFromI = distances[i];
        distancesFromI.push_back( distances_from_diagonal[i] );

        //sort it:
        std::sort( distancesFromI.begin() , distancesFromI.end() , std::greater<double>() );

        if ( k > distancesFromI.size() )
        {
            if ( dbg )
            {
                cerr << "There are not enough neighbors in your set. We set the result to plus infty \n";
            }
            result.push_back( std::numeric_limits<double>::max() );
        }
        else
        {
            if ( distances_from_diagonal[i] > distancesFromI[k] )
            {
                if ( dbg )
                {
                    cerr << "The k-th n.n. is on a diagonal. Therefore we set up a distance to diagonal \n";
                }
                result.push_back( distances_from_diagonal[i] );
            }
            else
            {
                result.push_back( distancesFromI[k] );
            }
        }
    }
    std::sort( result.begin() , result.end() , std::greater<double>() );
    result.resize( std::min( result.size() , where_to_cut ) );

    return result;
}


std::pair<double,double> Persistence_intervals::min_max()
{
    double min_ = std::numeric_limits<int>::max();
    double max_ = -std::numeric_limits<int>::max();
    for ( size_t i = 0 ; i != this->intervals.size() ; ++i )
    {
        if ( this->intervals[i].first < min_ )min_ = this->intervals[i].first;
        if ( this->intervals[i].second > max_ )max_ = this->intervals[i].second;
    }
    return std::make_pair( min_ , max_ );
}

double Persistence_intervals::project_to_R( int number_of_function )
{
	double result = 0;
	
	for ( size_t i = 0 ; i != this->intervals.size() ; ++i )
	{
		result += ( this->intervals[i].second - this->intervals[i].first )*( this->intervals[i].second - this->intervals[i].first );
	}
	
	return result;
}


}//namespace gudhi stat
}//namespace gudhi

#endif