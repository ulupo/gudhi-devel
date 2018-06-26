/*    This file is part of the Gudhi Library. The Gudhi library
 *    (Geometric Understanding in Higher Dimensions) is a generic C++
 *    library for computational topology.
 *
 *    Author(s):       Clément Maria
 *
 *    Copyright (C) 2018 Inria
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

#include <gudhi/Rips_complex.h>
#include <gudhi/distance_functions.h>
#include <gudhi/Simplex_tree.h>
#include <gudhi/Persistent_cohomology.h>
#include <gudhi/Points_off_io.h>

#include <string>
#include <vector>
#include <limits>  // infinity

using namespace Gudhi;

typedef int Vertex_handle;
typedef double Filtration_value;
using Rips_complex = Gudhi::rips_complex::Rips_complex<Filtration_value>;
using Point = std::vector<double>;
typedef Simplex_tree<Simplex_tree_options_morse> opt_ST;
typedef Simplex_tree<> brut_ST;

/* 
 * Display on std::cout the coboundaries of all simplices in a Rips complex.
 */
int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " path_to_file_points Rips_threshold max_dim \n";
    return 0;
  }
  std::string off_file_points = argv[1];
  Filtration_value threshold = atof(argv[2]);
  int dim_max = atoi(argv[3]);

  clock_t start, end;
  // Construct the Simplex Tree
  std::cout << "---- Simplex tree with FAST cofaces option: \n";
  Gudhi::Points_off_reader<Point> off_reader(off_file_points);
  Rips_complex rips_complex_from_file(off_reader.get_point_cloud(), threshold, Euclidean_distance());

  start = clock();
  opt_ST opt_st;
  rips_complex_from_file.create_complex(opt_st, dim_max);
  end = clock();
  std::cout << "     Construct Rips complex in " << static_cast<double>(end - start) / CLOCKS_PER_SEC << " s. \n";

  std::cout << "     Information of the Simplex Tree: " << std::endl;
  std::cout << "        Number of vertices = " << opt_st.num_vertices() << "\n";
  std::cout << "        Number of simplices = " << opt_st.num_simplices() << std::endl;

  std::cout << "     Compute the coboundary of every simplex in the Simplex_tree:" << std::endl;
  start = clock();  

  for (auto f_simplex : opt_st.filtration_simplex_range()) {  // for every simplex
    for(auto v : opt_st.simplex_vertex_range(f_simplex)) {std::cout << v <<" ";}
    std::cout << std::endl;
    for (auto c_simplex : opt_st.coboundary_simplex_range(f_simplex)) {
      std::cout << "     ";
      for(auto v : opt_st.simplex_vertex_range(c_simplex)) {std::cout << v <<" ";}
      std::cout << std::endl;
    }  // traverse the coboundary
  }
 
  for (auto f_simplex : opt_st.filtration_simplex_range()) {  // for every simplex
    for(auto v : opt_st.simplex_vertex_range(f_simplex)) {std::cout << v <<" ";}
    std::cout << std::endl;
    for (auto c_simplex : opt_st.boundary_simplex_range(f_simplex)) {
      std::cout << "     ";
      for(auto v : opt_st.simplex_vertex_range(c_simplex)) {std::cout << v <<" ";}
      std::cout << std::endl;
    }  // traverse the boundary
  }
  return 0;
}
