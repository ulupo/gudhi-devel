#ifndef DOLPHINN_HYPERCUBE_H
#define DOLPHINN_HYPERCUBE_H

#include <vector>
#include <gudhi/Stable_hash_function.h>
#include <thread>
#include <iterator>
#include <utility>

namespace Gudhi {

namespace dolphinn
{
/**
 * \class Hypercube Hypercube.h gudhi/Hypercube.h
 * \brief Data structure for Dolphinn.
 * 
 * \ingroup dolphinn
 * 
 * \details
 * This class is the hypercube used in Dolphinn. It contains functions for quering and so far 
 * unused parallelisation code. Among its members, there is a vector of hash functions, every 
 * vertex of the hypercube corresponds to a possible output of this vector (each function hash 
 * a points and returns a bit). One of these hash functions contains a map corresponding to the 
 * data structure. 
 * 
 */



  template <typename Point, typename T, typename bitT>
  class Hypercube
  {
  	// original dimension of points
    const int D;
    // mapped dimension of points (dimension of the Hypercube)
    const int K;
    // Parameter of the LSH functions
    const float R;
  	// The 'K' hash-functions that we are going to use. Only the last one will be used to query,
    // but we need all of them to map the query on arrival, first.
    std::vector<Stable_hash_function<T,Point>> H;
    // Reference of an 1D vector of points, emulating a 2D, with N rows and D columns per row.
    const std::vector<Point>& pointset;
    public:
    
    //accessors
    int get_D() {
  		return D;
  	}
  	int get_K() {
  		return K;
  	}
  	float get_R(){
  		return R;
  	}
  	std::vector<Point> get_pointset(){
  		return pointset;
  	}
  	std::vector<Stable_hash_function<T,Point>> get_H(){
  		return H;
  	}
  	
    
    /** \brief Constructor that creates in parallel a 
      * vector from a stable distribution.
      *
      * Assign 'D' random values from a normal distribution
      * N(0,1/sqrt(D)).
      *
      * @param pointset    - 1D vector of points, emulating a 2D, with N rows and D columns per row.
      * @param N           - number of points
      * @param D           - dimension of points
      * @param K           - dimension of Hypercube (and of the mapped points)
      * @param threads_no  - number of threads to be created. Default value is 'std::thread::hardware_concurrency()'.
      * @param r           - parameter of Stable Distribution. Default value is 4. Should be modified for Nearest 
      *                      Neighbor Search, to adapt to the average distance of the NN, 'r' is the hashing window.
   */
    Hypercube(const std::vector<Point>& pointset, const int N, const int D, const int K, const int threads_no = 1 /*std::thread::hardware_concurrency()*/, const float r = 4/*3 or 8*/)
      : D(D), K(K), R(r), pointset(pointset)
    {
      if(N<1){
      	std::cerr << "N<1 aborting construction\n";
      	exit(-1); 
      }
      {
      	if(r!=0){
      		std::vector<bitT> mapped_pointset(N * K);
      		for(int k = 0; k < K - 1; ++k)
		      {
		        H.emplace_back(D, r);
		        //H[k].print_a();
		        H[k].hash(pointset, N, D);
		        //H[k].print_stats();

		        H[k].assign_random_bit(mapped_pointset, k, K);
		      }
		      H.emplace_back(D, r);
		      H[K - 1].hash(pointset, N, D);
		      H[K - 1].assign_random_bit_and_fill_hashtable_cube(mapped_pointset, K);

		      //H[K - 1].print_hashtable_cube();
      	} else {
      		H.emplace_back(K, D, r);
      		H[0].hyperplane_hash(pointset);
      		//H[0].print_hashtable_cube();
      	}
        
      }
     /* else
      {
        std::vector<std::thread> threads;

        std::vector<std::vector<Stable_hash_function<T,Point>>> subvectors(threads_no);
        const int subvector_size = (K - 1)/threads_no;
        //std::cout << "subvector_size = " << subvector_size << std::endl;
        for (int i = 0; i < threads_no; ++i)
          threads.push_back(std::thread(populate_vector_of_hash_functions, std::ref(subvectors[i]), subvector_size, D, r, std::ref(pointset), N, std::ref(mapped_pointset), i * subvector_size, K));

        for (auto& th : threads)
          th.join();

        for(auto& subv: subvectors)
        {
          //for(auto& s: subv)
          //{
            H.insert(
              H.end(),
              std::make_move_iterator(subv.begin()),
              std::make_move_iterator(subv.end())
            );
          //}
        }
        H.emplace_back(D, r);
        H[K - 1].hash(pointset, N, D);
        H[K - 1].assign_random_bit_and_fill_hashtable_cube(mapped_pointset, K);

        //H[K - 1].print_hashtable_cube();
      }*/
    } 

    /** \brief Populate the vector of hash functions.
      * Helper function for the Constructor in a parallel environment.
      *
      * @param H                 - vector of Hash Functions
      * @param n_vec             - nubmer of hash function to be inserted
      * @param D                 - dimension of the original points
      * @param r                 - Stable Distirbution parameter
      * @param pointset          - original points
      * @param N                 - number of origial points
      * @param mapped_pointset   - vector of mapped points (to be poppulated)
      * @param k_start           - starting index of mapped_pointset to be poppulated in parallel
      * @param K                 - dimension of Hypercube
    */
    static void populate_vector_of_hash_functions(std::vector<Stable_hash_function<T,Point>>& H, const int n_vec, const int D, const int r, const std::vector<Point>& pointset, const int N, std::vector<bitT>& mapped_pointset, const int k_start, const int K)
    {
      for (int i = 0; i < n_vec; ++i)
      {
        H.emplace_back(D, r, k_start + i);
        H[i].hash(pointset, N, D);
        //std::cout << k_start << " " << i << std::endl;
        H[i].assign_random_bit(mapped_pointset, k_start + i, K);
      }
    }

    /** \brief Radius query the Hamming cube.
      *
      * @param query               - vector of queries
      * @param Q                   - number of queries
      * @param radius              - find a point within r with query
      * @param MAX_PNTS_TO_SEARCH  - threshold
      * @param results_idxs        - indices of Q points, where Eucl(point[i], query[i]) <= r
      * @param threads_no          - number of threads to be created. Default value is 'std::thread::hardware_concurrency()'.
    */
    void radius_query(const std::vector<Point>& query, const int Q, const float radius, const int MAX_PNTS_TO_SEARCH, std::vector<int>& results_idxs, const int threads_no = std::thread::hardware_concurrency())
    {
    	if(R>0){
		    std::vector<bitT> mapped_query(Q * K);
		    //if(threads_no == 1){
	      for(int q = 0; q < Q; ++q)
	      {
		    	for(int k = 0; k < K; ++k)
		      {
		        H[k].assign_random_bit_query((query[q]), (std::begin(mapped_query) + q * K), k);
		      }
		      results_idxs[q] = H[K - 1].radius_query(std::string(mapped_query.begin() + q * K, mapped_query.begin() + (q + 1) * K), radius, K, MAX_PNTS_TO_SEARCH, pointset.begin(), query.begin() + q);
	    	}
		  } else {
		  	std::vector<bitT> key(K);
				for(int q = 0; q < Q; ++q){
					H[0].hyperplane_hash(query[q], key);
					results_idxs[q] = H[0].radius_query(std::string(key.begin(), key.end()), radius, K, MAX_PNTS_TO_SEARCH, pointset.begin(), query.begin() + q);
				}
		  }
      /*}
      else
      {
        std::vector<std::thread> threads;

        const int batch = Q/threads_no;
        //std::cout << "subvector_size = " << subvector_size << std::endl;
        for (int i = 0; i < threads_no - 1; ++i)
          threads.push_back(std::thread(execute_radius_queries, std::ref(H), std::ref(query), std::ref(mapped_query), i * batch, (i + 1) * batch, K, D, std::ref(pointset), radius, MAX_PNTS_TO_SEARCH, std::ref(results_idxs)));
        threads.push_back(std::thread(execute_radius_queries, std::ref(H), std::ref(query), std::ref(mapped_query), (threads_no - 1) * batch, Q, K, D, std::ref(pointset), radius, MAX_PNTS_TO_SEARCH, std::ref(results_idxs)));
    
        for (auto& th : threads)
          th.join();
      }*/
      // per query, find candidates
      /*for(int q = 0; q < Q; ++q)
      {
        //std::cout << "Query no. " << q << std::endl;
        results_idxs[q] = H[K - 1].radius_query(std::string(mapped_query.begin() + q * K, mapped_query.begin() + (q + 1) * K), radius, K, MAX_PNTS_TO_SEARCH, pointset.begin(), query.begin() + q * D);
        //std::cout << "Query no. " << q << " completed" << std::endl;
      }*/
    }

    /** \brief Execute specified portion of Radius Queries.
      * Helper function for the Constructor in a parallel environment.
      *
      * @param H                    - vector of Hash Functions
      * @param query                - vector of all queries
      * @param mapped_query         - vector of all (to be) mapped queries
      * @param q_start              - starting index of query to execute
      * @param q_end                - ending index of query to execute
      * @param K                    - dimension of Hypercube
      * @param D                    - dimension of original points and queries
      * @param pointset             - original points
      * @param radius               - radius to query with
      * @param MAX_PNTS_TO_SEARCH   - threshold when searching
      * @param results_idxs         - The index of the point-answer in i-th posistion, for i-th query, -1 if not found.
    */
    static void execute_radius_queries(std::vector<Stable_hash_function<T,Point>>& H, const std::vector<Point>& query, std::vector<bitT>& mapped_query, const int q_start, const int q_end, const int K, const int D, const std::vector<Point>& pointset, const float radius, const int MAX_PNTS_TO_SEARCH, std::vector<int>& results_idxs)
    {
      for(int q = q_start; q < q_end; ++q)
      {
        for(int k = 0; k < K; ++k)
        {
          H[k].assign_random_bit_query(query[q], (std::begin(mapped_query) + q * K), k);
        }
        results_idxs[q] = H[K - 1].radius_query(std::string(mapped_query.begin() + q * K, mapped_query.begin() + (q + 1) * K), radius, K, MAX_PNTS_TO_SEARCH, pointset.begin(), query.begin() + q);
      }
    }

    /** \brief Nearest Neighbor query in the Hamming cube.
      *
      * @param query               - vector of queries
      * @param Q                   - number of queries
      * @param m                   - number of neigbours to be searched
      * @param MAX_PNTS_TO_SEARCH  - threshold
      * @param results_idxs_dists  - indices and distances of Q points, where the (Approximate) Nearest Neighbors are stored.
      * @param threads_no          - number of threads to be created. Default value is 'std::thread::hardware_concurrency()'.
    */
    void m_nearest_neighbors_query(const std::vector<Point>& query, const int Q, const int m, const int MAX_PNTS_TO_SEARCH, std::vector<std::vector<std::pair<int, float>>>& results_idxs_dists, const int threads_no = std::thread::hardware_concurrency())
    {
    	if(R>0){
		    std::vector<bitT> mapped_query(Q * K);
		    //if(threads_no == 1){
	      for(int q = 0; q < Q; ++q)
	      {
	        for(int k = 0; k < K; ++k)
	        {
	          H[k].assign_random_bit_query(query[q], (std::begin(mapped_query) + q * K), k);
	        }
	        results_idxs_dists[q] = H[K - 1].m_nearest_neighbors_query(std::string(mapped_query.begin() + q * K, mapped_query.begin() + (q + 1) * K), K, m, MAX_PNTS_TO_SEARCH, pointset.begin(), query.begin() + q);
	      }
			} else {
				std::vector<bitT> key(K);
				for(int q = 0; q < Q; ++q){
					H[0].hyperplane_hash(query[q], key);
					results_idxs_dists[q] = H[0].m_nearest_neighbors_query(std::string(key.begin(), key.end()), K, m, MAX_PNTS_TO_SEARCH, pointset.begin(), query.begin() + q);
				}
			}
      /*}
      else
      {
        std::vector<std::thread> threads;

        const int batch = Q/threads_no;
        //std::cout << "subvector_size = " << subvector_size << std::endl;
        for (int i = 0; i < threads_no - 1; ++i)
          threads.push_back(std::thread(execute_nearest_neighbor_queries, std::ref(H), std::ref(query), std::ref(mapped_query), i * batch, (i + 1) * batch, K, D, std::ref(pointset), MAX_PNTS_TO_SEARCH, std::ref(results_idxs_dists)));
        threads.push_back(std::thread(execute_nearest_neighbor_queries, std::ref(H), std::ref(query), std::ref(mapped_query), (threads_no - 1) * batch, Q, K, D, std::ref(pointset), MAX_PNTS_TO_SEARCH, std::ref(results_idxs_dists)));
    
        for (auto& th : threads)
          th.join();
      }*/
    }

    /** \brief Execute specified portion of Nearest Neighbor Queries.
      * Helper function for the Constructor in a parallel environment.
      *
      * @param H                    - vector of Hash Functions
      * @param query                - vector of all queries
      * @param mapped_query         - vector of all (to be) mapped queries
      * @param q_start              - starting index of query to execute
      * @param q_end                - ending index of query to execute
      * @param K                    - dimension of Hypercube
      * @param D                    - dimension of original points and queries
      * @param pointset             - original points
      * @param MAX_PNTS_TO_SEARCH   - threshold when searching
      * @param results_idxs_dists  - indices and distances of Q points, where the (Approximate) Nearest Neighbors are stored.
    */
    static void execute_nearest_neighbor_queries(std::vector<Stable_hash_function<T,Point>>& H, const std::vector<Point>& query, std::vector<bitT>& mapped_query, const int q_start, const int q_end, const int K, const int D, const std::vector<Point>& pointset, const int MAX_PNTS_TO_SEARCH, std::vector<std::pair<int, float>>& results_idxs_dists)
    {
      for(int q = q_start; q < q_end; ++q)
      {
        for(int k = 0; k < K; ++k)
        {
          H[k].assign_random_bit_query(query[q], (std::begin(mapped_query) + q * K), k);
        }
        results_idxs_dists[q] = H[K - 1].nearest_neighbor_query(std::string(mapped_query.begin() + q * K, mapped_query.begin() + (q + 1) * K), K, MAX_PNTS_TO_SEARCH, pointset.begin(), query.begin() + q);
      }
    }

    /** \brief Print how many points are assigned to every vertex.
      * Empty vertices (if any) are not printed (because we do not store them).
      *
    */
    void print_no_of_assigned_points_per_vertex()
    {
    	if(R>0){
      	H[K - 1].print_hashtable_cube();
      } else {H[0].print_hashtable_cube();}
    }

  };
}
}
#endif /* DOLPHINN_HYPERCUBE_H */
