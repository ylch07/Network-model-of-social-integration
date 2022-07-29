/* ============================================================
   Source codes for the nodeList data class (part IV)
   This file contains subroutines and functions related to
     the calculation of statistics data.
	    void computeStats
	    void updateDistMatrix
	    vector<int> algorithmDijkstra
	    int minDistance
	    int numCluster
	    vector<int> degreeConnectionSnapshot

   Author: Yao-li Chuang
   ============================================================ */
#include"../Node/NodeListC.hpp"

/************************************************************************
   This subroutine computes the average opinion $(stats.avg_op) and 
      the average utility $(stats.avg_ut) per node. 
      (Here the utility includes the cost of maintaining the social 
       connections.)
   No input and return values.
 ***********************************************************************/
void nodeList::computeStats(void) {
  int n=memberNodes.size();
  double op_tot=0., op_h=0., op_g=0.;
  double ut_tot=0., ut_h=0., ut_g=0.;
  for(int i=0; i<n; i++) {
    if(memberNodes.at(i).getNodeType()==1) {
      op_h += memberNodes.at(i).getOpinion();
      ut_h += memberNodes.at(i).getUtCost();
    } else if(memberNodes.at(i).getNodeType()==-1) {
      op_g += memberNodes.at(i).getOpinion();
      ut_g += memberNodes.at(i).getUtCost();
    }
  }
  op_tot = op_h + op_g;
  op_h /= static_cast<double>(num_host);
  op_g /= static_cast<double>(num_guest);
  op_tot /= static_cast<double>(num_host+num_guest);
  double tmp_op[] = {op_tot, op_h, op_g};
  stats.avg_op.assign(tmp_op, tmp_op+3);

  ut_tot = ut_h + ut_g;
  ut_h /= static_cast<double>(num_host);
  ut_g /= static_cast<double>(num_guest);
  ut_tot /= static_cast<double>(num_host+num_guest);
  double tmp_ut[] = {ut_tot, ut_h, ut_g};
  stats.avg_ut.assign(tmp_ut, tmp_ut+3);
}

/************************************************************************
   This subroutine updates the distance matrix $(distMatrix) and the
     histogram of the distance distribution $(distHistogram).
   The distance between each pair of nodes is calculated using
     Dijkstra's algorithm. 
   No input and return values.
 ***********************************************************************/
void nodeList::updateDistMatrix(void) {

  // Here we check if adjMatrix is filled or empty. 
  // If it is already filled, we will use it directly.
  // If it is empty, we will create it for this subroutine but then
  //    clear it before we exit this subroutine.
  bool adj_empty=adjMatrix.empty();
  if(adj_empty)
    createAdjMatrix();

  int n = num_host + num_guest;
  distMatrix.assign(n*n, INT_MAX);
  distHistogram.clear(); distHistogram.assign(50,0);
  for(int src=0; src<n; src++) {
    vector<int> dist = algorithmDijkstra(n, src); // Dijkstra's algorithm
    for(int v=0; v<n; v++) {
      int dist_value = dist.at(v);
      distMatrix.at(src*n+v) = dist_value;
      if(dist_value<50)
	distHistogram.at(dist_value)++;
    }
  }
  dist_up2date=true;

  if(adj_empty) // if adjMatrix is originally empty, clear it.
    adjMatrix.clear();
}

/*********************************************************************
  This function computes the shortest distances from one node to
     all the other nodes using Dijkstra's algorithm.
   Input values: 
       $(n) is the number of nodes.
       $(src) is the index of the node from which the distances to
              all the other nodes are calculated.
   The return value is an integer vector of length $(n), representing
       the distances from $(src) to all the nodes.
  -----
   The distance between two nodes means the number of edges to traverse
       from one node to the other. Usually it implies the shortest path,
       i.e., the path through the fewest edges (assuming that the edges
       are not weighted). 
  -----
   If there exists no path from one node to the other, then the distance
       is infinity, which is assigned the value of maximal integer
       $(INT_MAX) in the codes. 
  -----
   The distance from a node to itself is zero.
 *********************************************************************/
vector<int> nodeList::algorithmDijkstra(int n, int src) {
  vector<int> dist(n, INT_MAX);   // set all distances to infinity
  vector<bool> spt_set(n, false); // mark all nodes unvisited
  dist.at(src) = 0;               // set distance=0 to itself
  int cnt_total = n - 1;
  for(int cnt = 0; cnt<cnt_total; cnt++) {
    // Look for the node with the shortest distance among univisted nodes.
    int u = minDistance(n, dist, spt_set);
    if(u == -1)
      cout << "minDistance not found in Dijkstra" << endl;
    else
      spt_set.at(u) = true;  // mark the node visited.
    
    // Update the tentative distances of the other unvisited nodes.
    // Given the shortest distance to node $(u) we just found, the
    //    distance to an unvisited node with a direct link to $(u) 
    //    would tentatively be this shortest distance + 1, unless
    //    it is already shorter (through another visited node). 
    // The other tentative distances remain unchanged.
    for(int v=0; v<n; v++) {
      int new_dist = dist.at(u)+adjMatrix.at(u*n+v);
      if(!spt_set.at(v) && adjMatrix.at(u*n+v)!=0 && dist.at(u)!=INT_MAX
	 && new_dist<dist.at(v))
	dist.at(v) = new_dist;
    } // end of v loop for updating dist vector
  } // end of cnt loop for going through the nodes;

  return dist;
}

/*********************************************************************
  This function is a part of Dijkstra's algorithm.
  It searches for a node among unvisited nodes of which 
      the distance (to $(src) in the function above) is
      the shortest.
   Input values: 
       $(m) is the number of nodes.
       $(dist) is a list of distances already calculated for these nodes.
       $(spt_set) is a list of flags indicating whether each node is
                  already visited or still unvisited.
   The return value is an integer index of the node found. 
   If such a node is not found, it returns -1, indicating there may be
      an error somewhere.
 *********************************************************************/
int nodeList::minDistance(int m, vector<int> dist, vector<bool> spt_set) {
  int min_dist = INT_MAX, min_index = -1;

  for(int v=0; v<m; v++)
    if(spt_set.at(v)==false && dist.at(v) <= min_dist) {
      min_dist = dist.at(v);
      min_index = v;
    }

  return min_index;
}

/**************************************************************
   This function returns the number of clusters at the moment.
   Return value ----
       An integer, representing the number of clusters
 **************************************************************/
int nodeList::numCluster(void) {
  // Need the updated distance matrix to count the number of clusters
  if(!IsDistMatrixUpdated())
    updateDistMatrix();

  int n=num_host+num_guest;
  vector<int> idx;
  for(int i=0; i<n; i++)
    idx.push_back(i);    // putting all nodes in vector $(idx)
  vector<vector<int> > clusters;
  // The loop below goes through the vector $(idx) to look for nodes
  //    belonging to the same cluster.
  // If nodes are found to be in the same cluster as the first node in
  //    $(idx), these nodes and the first node are put in vector $(group).
  // Those not in the same group are left in $(idx) for the next loop.
  for(;!idx.empty();) { 
    vector<int> tmpid(idx), group;
    group.clear(); idx.clear();
    int m=tmpid.size(), src=tmpid.at(0); group.push_back(src);
    // The loop below checks the distance between two nodes: 
    //    tmpid.at(0) (i.e., src) and tmpid.at(i), which 
    //    is already calculated and stored in distMatrix.
    // If the distance is finite, the two nodes are directly
    //    or indirectly connected, thus belonging to the same cluster.
    // If the distance is infinite (INT_MAX), the two nodes 
    //    belongs to different clusters.
    for(int i=1; i<m; i++)
      if(distMatrix.at(src*n+tmpid.at(i))<INT_MAX)
	group.push_back(tmpid.at(i)); // putting those in the same cluster in $(group)
      else
	idx.push_back(tmpid.at(i)); // putting those not in the same cluster back in $(idx)

    clusters.push_back(group); // collecting the groups in vector $(clusters)
  }
  return clusters.size(); // the length of vector $(clusters) is the number of distinct clusters
}

/**************************************************************
   This function returns the degree distribution of the network.
   Input ---
      n_degree : the upper bound of the returned degree.
   Return value ----
      An integer vector, degree[u], representing the counts
      of nodes having u edges (u < n_degree).
 **************************************************************/
vector<int> nodeList::degreeConnectionSnapshot(int n_degree) {
  vector<int> degree(n_degree,0);
  int n= num_host+num_guest;
  for(int i=0; i<n; i++) {
    int u=memberNodes.at(i).getNumConnections();
    if(u<n_degree)
      degree.at(u)++;
  }

  return degree;
}
