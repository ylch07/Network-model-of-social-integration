/* ============================================================
   Header file for the nodeList data class
   -----
   Brief Summary: The nodeList class contains a list of nodes,
                  with the simulated population model defined
                  through the functions and subroutines specifying
                  how these nodes interact and how the network
                  evolves over time.
   -----
      variables --
        <<Basic>>
          num_host : number of host nodes
          num_guest : number of guest nodes
          memberNodes : the list of nodes
	<<For the population model>>
          par : parameter values of the population model
	  adjMatrix : adjacency matrix
          num_link : the number of links of each node
          utMatrix : utility matrix
	<<For graphic display>>
          forceMatrix : force matrix
	<<For statistics>>
	  stats : statistics data of the network
          distMatrix : distance matrix
          distHistogram : histogram of distance distribution
	  dist_up2date : flag of whether distMatrix is up-to-date
   -----
       Functions and subroutines not defined explicitly here are
          defined in one of the 4 files: NodeListC.cxx, 
          ../Model/ModelC.cxx, ../Graphics/GraphModelC.cxx, 
          and ../Stats/StatC.cxx:
	  <<NodeListC.cxx>>
	     the contructors
             setNeighborConnections
	     rewireInitialConnections
	     linkGuests2RandomHosts
	     linkGuests2FractionHosts
	     RandomLinks
	     delOneNode
	     setGuestsIdling
	  <<ModelC.cxx>>
             setDefaultParameters
	     resetParametersFromFile
	     changeParameter
	     hostInitiation
	     nextTimeStep
	     createAdjMatrix
	     createUtMatrix
	     updateUtMatrix
	     updateOpinion
	     updateOpinion2
	     updateOpinionGuest
	     updateOpinion2Guest
	     utilityFunction
	     evolveAdjMatrix
	     updateConnection
	  <<GraphModelC.cxx>>
	     updateGraphData
	     createForceMatrix
	     updateForceMatrix
	     updatePosition
	     forceFunction
	     repulsionFunction
	  <<StatC.cxx>>
	     computeStats
	     updateDistMatrix
	     algorithmDijkstra
	     minDistance
	     numCluster
	     degreeConnectionSnapshot

   Author: Yao-li Chuang
   ============================================================ */
#ifndef __NodeListC_hpp_INCLUDED__
#define __NodeListC_hpp_INCLUDED__

#include"../CCommon.h"
#include"NodeC.hpp"


/**************************************************************
   The parameter values of the simulated population model
   -----------
   Note:
       iniConnection and ini_hlink_frac were introduced to specify
       the initial configuration of the network connections.
       However, they have not been actually implemented yet.
       Currently, the initial number of connections per node
       is defined by an input argument $(nLinkEach) in the contructor 
       of $(nodeList).
 **************************************************************/
struct modelParameters {
  double AH;
  double AG;
  double sigmaH;
  double sigmaG;
  double kappa;
  double alpha;
  double gamma;
  double welfare;
  bool enable_op;  // enable or disable the change of opinions
  bool enable_net; // enable or disable the change of connections
  // The next 2 are not currently implemented.
  double ini_hlink_frac; // initial fraction of host connections per node (currently not in use, may belong to the initial conditions in Main.cxx)
};

/**************************************************************
  The statistical data that we compute ---
     avg_link: average number of links per node (4 entries: overall, 
               host2host per host, host2guest per host, 
               guest2host per guest, guest2guest per guest)
     avg_op: average opinion per node (3 entries: overall, host, guest)
     avg_ut: average utility per node (3 entries: overall, host, guest)
     avg_rw: average reward per link (4 entries: overall, host2host,
             guest2guest, host2guest)
 **************************************************************/
struct modelStats {
  vector<double> avg_link; // number of links; total, h2h, h2g, g2h, g2g
  vector<double> avg_op; //opinion; total, h, g
  vector<double> avg_ut; //utility; total, h, g  
  vector<double> avg_rw; //reward; total, hh, gg, hg
};


/**************************************************************
   nodeList data class
 **************************************************************/
class nodeList {

public:
  // Constructors & destructor
  // The difference between the 2 constructors is whether the guest nodes are specified by a ratio or by a number
  nodeList(int totalN, double guest_ratio, int nLinkEach, double iniOp=1.0);
  nodeList(int totalN, int guestN, int nLinkEach, double iniOp=1.0);
  ~nodeList(void) { memberNodes.clear(); adjMatrix.clear(); num_link.clear();
    utMatrix.clear(); forceMatrix.clear(); distMatrix.clear();}
  // Getters
  vector<node> getMemberNodes(void) {return memberNodes;}
  int getNumMemberNodes(void) {return memberNodes.size();}
  int getNumHost(void) {return num_host; }
  int getNumGuest(void) {return num_guest; }
  // Adding or deleting nodes
  void addOneNode(node value) {memberNodes.push_back(value);}
  void delOneNode(int i); // in NodeListC.cxx
  // For initiating connections
  void linkGuests2FractionHosts(int nLinkEach, int n_host, double hfrac); // in NodeListC.cxx
  double hostInitiation(void); // in ModelC.cxx
  // For model parameters (ModelC.cxx)
  void resetParametersFromFile(string file_name);
  void changeParameter(string pname, double value);
  void changeParameter(string pname, bool value);
  // For running the model simulation (ModelC.cxx)
  void nextTimeStep(void);
  vector<double> utilityFunction(int ntype1, double x1, int ntype2, double x2);
  // For graphic display (GraphModelC.cxx)
  void updateGraphData(void);
  // For statistics (StatC.cxx)
  void computeStats(void);
  struct modelStats getStats(void) {return stats;}
  bool IsDistMatrixUpdated(void) {return dist_up2date;}
  vector<int> getDistHistogram(void) {
    if(!IsDistMatrixUpdated())
      updateDistMatrix();
    return distHistogram;
  }
  int numCluster(void);
  vector<int> degreeConnectionSnapshot(int n_degree);

private:
  int num_host, num_guest;
  vector<node> memberNodes;
  struct modelParameters par;
  vector<int> adjMatrix, num_link, distMatrix, distHistogram;
  vector<double> utMatrix, forceMatrix;
  bool dist_up2date;
  struct modelStats stats;
  // For initiating connections (NodeListC.cxx)
  void setNeighborConnections(int nLinkEach, int n_host);
  void rewireInitialConnections(int ifirst, int n_node,
				double rw_prob);
  void linkGuests2RandomHosts(int nLinkEach, int n_host);
  void RandomLinks(int nLinkEach);
  // For setting the status of nodes (NodeListC.cxx)
  void setGuestsIdling(bool value);
  // For model parameters (ModelC.cxx)
  void setDefaultParameters(void);
  // For running the model simulation (ModelC.cxx)
  void createAdjMatrix(void);
  void createUtMatrix(void);
  void updateUtMatrix(void);
  void updateOpinion(void);  // See ModelC.cxx for the difference
  void updateOpinion2(void); //  between updateOpinion & updateOpinion2
  void updateOpinionGuest(void);
  void updateOpinion2Guest(void);
  void evolveAdjMatrix(void);
  void updateConnection(void);
  // For graphic display (GraphModelC.cxx)
  void createForceMatrix(void);
  void updateForceMatrix(void);
  void updatePosition(void);
  vector<double> forceFunction(int ntype1, double x1, vector<double> p1,
			       int ntype2, double x2, vector<double> p2);
  vector<double> repulsionFunction(int ntype1, double x1, vector<double> p1,
				   int ntype2, double x2, vector<double> p2);
  // For statistics (StatC.cxx)
  void updateDistMatrix(void);
  vector<int> algorithmDijkstra(int n, int src);
  int minDistance(int m, vector<int> dist, vector<bool> spt_set);
};


#endif
