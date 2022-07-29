/* ============================================================
   Source codes for the nodeList data class (part I)
   This file contains subroutines and functions related to
     setting and manupulating the list of nodes:
             the constructors
             void setNeighborConnections
	     void rewireInitialConnections
	     void linkGuests2RandomHosts
	     void linkGuests2FractionHosts
	     void RandomLinks
	     void delOneNode
	     void setGuestsIdling

   Author: Yao-li Chuang
   ============================================================ */
#include"NodeListC.hpp"

/************************************************************************
  Constructor of a node list
  Inputs:
     totalN - total number of nodes
     guest_ratio - ratio of guest nodes
     nLinkEach - average number of social connections per node
     ini_Op - intensity of opinions (default value=1.0, set in the header file)
*************************************************************************/
nodeList::nodeList(int totalN, double guest_ratio, int nLinkEach, double iniOp)
{
  // set model parameters
  setDefaultParameters();

  // Reset the random seed using the current time.
  time_t current_time;
  srand(static_cast<unsigned>(time(&current_time)));

  // Creating $(totalN) nodes.
  if(!memberNodes.empty()) memberNodes.clear(); // first clear the node list
  int i=0, n_host = totalN*(1.0-guest_ratio);
  num_host = n_host; num_guest = totalN-n_host; // numbers of hosts and guests
  // Now we begin to initiate a list of host and guest nodes.
  // double dtheta = M_PI*2.0/totalN, theta=0.0; // for putting agents on a circle.
  for(; i<n_host; i++) { // initiating host nodes
    double tmp_op = iniOp; // unified initial opinion
    //double tmp_op = static_cast<double>(rand())
    //               /static_cast<double>(RAND_MAX); // random initial opinion
    node tmp(1, tmp_op); // creating a host node 

    // Here we initial the positions of each node on the graphic display.
    // For further details of how the initial position is set, please see
    //   the comments for the class constructor in AgentC.cxx
    // Simply speaking, the node will be placed randomly on a circular disc.
    vector<double> cxy; // cxy contains the coordinates of the disc center
    cxy.clear(); cxy.push_back(-60.0); cxy.push_back(0.0); 
    agent tmp_agent(cxy, 60.0); // for putting agents on a circular disc
    //agent tmp_agent(theta); // for putting agents on a circle
    //theta += dtheta;
    tmp.setGraphAgent(tmp_agent); // assigning the graphic agent to the node
    memberNodes.push_back(tmp); // adding the node to the NodeList
  }
  for(; i<totalN; i++) { // initiating guest nodes
    double tmp_op = -iniOp; // unified initial opinion
    //double tmp_op = - static_cast<double>(rand())
    //                 /static_cast<double>(RAND_MAX); // random initial opinion
    node tmp(-1, tmp_op); // creating a guest node
    // Same as the hosts above, here we put the guests on another circular
    //    disc; note that the host disc centers at (-60, 0), whereas the
    //    guest disc at (60, 0). 
    vector<double> cxy; // cxy contains the coordinates of the disc center
    cxy.clear(); cxy.push_back(60.0); cxy.push_back(0.0);
    agent tmp_agent(cxy, 60.0); // for putting agents on a circular disc
    //agent tmp_agent(theta); // for putting agents on a circle
    //theta += dtheta;
    tmp.setGraphAgent(tmp_agent); // assigning the graphic agent to the node
    memberNodes.push_back(tmp); // adding the node to the NodeList
  }

  // Here we make the initial social connections for the nodes that we
  //  have just created.   
  if(nLinkEach != 0) {
    // Make a randomly connected network
    //RandomLinks(nLinkEach); 

    // Make a small world network among the host nodes,
    //    guest nodes unconnected.
    setNeighborConnections(nLinkEach, n_host);
    rewireInitialConnections(0, n_host, 0.1); // rewire only among hosts

    // Make a small world network among all nodes (hosts+guests).
    //setNeighborConnections(nLinkEach, totalN);
    //rewireInitialConnections(0, totalN, 0.1); // rewire among all nodes

    // Randomly link guest nodes to host nodes.
    //linkGuests2RandomHosts(nLinkEach, n_host);

    // Rewire links among the guest nodes (only if the number of guest
    //   nodes is sufficient large.)
    //int n_guest = totalN-n_host;
    //if(n_guest>=5*nLinkEach) 
    //rewireInitialConnections(n_host, n_guest, 0.1); // rewire guest nodes
  }

  // Set the average number of links for the initial statistics
  double nlink = static_cast<double>(nLinkEach);
  double tmp_link[] = {nlink*n_host/totalN, nlink, 0, 0, 0}; 
  stats.avg_link.assign(tmp_link, tmp_link+5);

  createAdjMatrix();
  createUtMatrix();
  updateConnection();

  // Clear distance Matrix in case it is not empty
  //   and set the up2date flag to false
  distMatrix.clear();
  dist_up2date=false;
}

/************************************************************************
  Constructor of a node list
  Inputs:
     totalN - total number of nodes
     guestN - number of guest nodes 
     nLinkEach - average number of social connections per node
     ini_Op - intensity of opinions (default value=1.0, set in the header file)
*************************************************************************/
nodeList::nodeList(int totalN, int guestN, int nLinkEach, double iniOp) {

  // set model parameters
  setDefaultParameters();

  // Reset the random seed using the current time.
  time_t current_time;
  srand(static_cast<unsigned>(time(&current_time)));

  // Creating $(totalN) nodes.
  if(!memberNodes.empty()) memberNodes.clear();
  int i=0, n_host = totalN - guestN;
  num_host = n_host; num_guest = guestN;

  // Now we begin to initiate a list of host and guest nodes.
  //double dtheta = M_PI*2.0/totalN, theta=0.0; // for putting agents on a circle.
  for(; i<n_host; i++) { // initiating host nodes
    double tmp_op = iniOp; // unified initial opinion
    //double tmp_op = static_cast<double>(rand())
    //               /static_cast<double>(RAND_MAX); // random initial opinion
    node tmp(1, tmp_op);
    vector<double> cxy;
    cxy.clear(); cxy.push_back(-50.0); cxy.push_back(0.0);
    agent tmp_agent(cxy, 60.0);
    //agent tmp_agent(theta); // for putting agents on a circle.
    //theta += dtheta;
    tmp.setGraphAgent(tmp_agent);
    memberNodes.push_back(tmp);
  }
  for(; i<totalN; i++) { // initiating guest nodes
    double tmp_op = -iniOp; // unified initial opinion
    //double tmp_op = - static_cast<double>(rand())
    //                 /static_cast<double>(RAND_MAX); // random initial opinion
    node tmp(-1, tmp_op);
    vector<double> cxy;
    cxy.clear(); cxy.push_back(50.0); cxy.push_back(0.0);
    agent tmp_agent(cxy, 20.0);
    //agent tmp_agent(theta); // for putting agents on a circle.
    //theta += dtheta;
    tmp.setGraphAgent(tmp_agent);
    memberNodes.push_back(tmp);
  }
  
  if(nLinkEach != 0) {
    // Make a randomly connected network
    //RandomLinks(nLinkEach); 

    // Make a small world network among the host nodes,
    //    guest nodes unconnected.
    setNeighborConnections(nLinkEach, n_host);
    rewireInitialConnections(0, n_host, 0.1); // rewire only among hosts

    // Make a small world network among all nodes (hosts+guests).
    //setNeighborConnections(nLinkEach, totalN);
    //rewireInitialConnections(0, totalN, 0.1); // rewire among all nodes

    // Randomly link guest nodes to host nodes.
    //linkGuests2RandomHosts(nLinkEach, n_host);

    // Rewire links among the guest nodes (only if the number of guest
    //   nodes is sufficient large.)
    //int n_guest = totalN-n_host;
    //if(n_guest>=5*nLinkEach) 
    //rewireInitialConnections(n_host, n_guest, 0.1); // rewire guest nodes
  }

  // Set the average number of links for the initial statistics
  double nlink = static_cast<double>(nLinkEach);
  double tmp_link[] = {nlink*n_host/totalN, nlink, 0, 0, 0};
  stats.avg_link.assign(tmp_link, tmp_link+5);
  createAdjMatrix();
  createUtMatrix();
  updateConnection();

  distMatrix.clear();
  dist_up2date=false;
}

/**********************************************************************
  This function connects node i to $(nLinkEach) of its immediate neighbors.
  By neighbors, it means the nodes ranged from index i-p to i+p, where
     p = $(nLinkEach)/2. 
  This is usually used as the first step to construct a small world network
   through the Watts-Strogatz model.
  Moreover, by our assumptions, host nodes connect initially only to other
      host nodes, whereas guest nodes are not connected. 
  Input values: 
       $(nLinkEach) is the number of links each guest node will make.
       $(n_host) is the number of host nodes. 
  No return values.
**********************************************************************/
void nodeList::setNeighborConnections(int nLinkEach, int n_host)
{
  int n = memberNodes.size();
  int st = -nLinkEach/2;
  int ed = st+nLinkEach;
  // Connect host nodes with neighboring host nodes
  int i=0;
  for(; i < n_host; i++)
    for(int j=(i+st); j<(i+ed); j++) {
      int j2;
      if(j==i) continue;
      else if(j<0) j2=j+n_host;
      else if(j>=n_host) j2=j-n_host;
      else j2 = j;
      vector<double> tmp_op; tmp_op.clear();
      tmp_op.push_back(memberNodes[j2].getOpinion()); // tmp_op[0] is the opinion of node j2
      tmp_op.push_back(memberNodes[i].getOpinion()); // tmp_op[1] is the opinion of node i
      // Computing the utility
      vector<double> tmp_ut = utilityFunction(memberNodes[i].getNodeType(),
					      tmp_op[1],
					      memberNodes[j2].getNodeType(),
					      tmp_op[0]);
      memberNodes[i].addAConnection(memberNodes[j2].getId(), tmp_op[0], tmp_ut[0]);  // tmp_ut[0] is the utility node j2 gives node i
      memberNodes[j2].addAConnection(memberNodes[i].getId(), tmp_op[1], tmp_ut[1]);  // tmp_ut[1] is the utility node i gives node j2
    }

  // Connect guest nodes to neighboring guest nodes
  /* commented out so that guests don't connect initially.
  for(; i < n; i++)
    for(int j=(i+st); j<(i+ed); j++) {
      int j2;
      if(j==i) continue;
      else if(j<n_host) j2=j-n_host+n;
      else if(j>=n) j2=j-n+n_host;
      else j2=j;
      vector<double> tmp_op; tmp_op.clear();
      tmp_op.push_back(memberNodes[j2].getOpinion());
      tmp_op.push_back(memberNodes[i].getOpinion());
      vector<double> tmp_ut = utilityFunction(memberNodes[i].getNodeType(),
					      tmp_op[1],
					      memberNodes[j2].getNodeType(),
					      tmp_op[0]);
      memberNodes[i].addAConnection(memberNodes[j2].getId(), tmp_op[0], tmp_ut[0]);
      memberNodes[j2].addAConnection(memberNodes[i].getId(), tmp_op[1], tmp_ut[1]);
    }
  */
}

/************************************************************************
   This function rewires the existing connections among a group of nodes,
      with node index from $(ifirst) to $(ifirst)+$(n_node), according to
      a given rewiring probability $(rw_prob). 
   This represents the second step of the Watts-Strogatz model to make a
      small world network.
   Input values: 
       $(ifirst) is the index of the first node to rewire connections.
       $(n_node) is the number of nodes to rewire connections.
       $(rw_prob) is the probability of each connection to rewire.
   No return values.
 ***********************************************************************/
void nodeList::rewireInitialConnections(int ifirst,
					int n_node, double rw_prob) {
  int iend = ifirst+n_node; 
  // Loop through the nodes to rewire the connections
  for(int i=ifirst; i<iend; i++) {
    int n_link = memberNodes[i].getNumConnections(); // get the number of existing connections of node i.
    // Loop through the connections to determin whether and to which node
    //    an existing link would be rewired.
    for(int j=0; j<n_link; j++) { 
      double tmp = static_cast<double>(rand())/static_cast<double>(RAND_MAX); // 0 <= tmp < 1 is a random number to determine whether link j should be rewired.
      if(tmp<=rw_prob) { // Rewire link j
	// First we have to find where link j is connected and cut the link
	long unsigned int jid = memberNodes[i].getAConnection(j); // jid is the index of the connected node of link j from node i
	// Here we look for this connected node using the index jid.
	int jcnt=-1;
	for(int k=ifirst; k<iend; k++)
	  if(memberNodes[k].getId()==jid) { jcnt=k; break; }
	if(jcnt==-1) { // Print error messages if no such node is found.
	  cout << "Error in rewireInitialConnections (NodeListC.cxx)" << endl;
	  cout << "iid= " << memberNodes[i].getId();
	  cout << ", jid= " << jid << ", n_link = " << n_link << endl;
	  exit(1);
	}
	// Note that the link is stored in both connected nodes, so we have
        //    to delete it from both ends.
	memberNodes[jcnt].delAConnection(memberNodes[i].getId());
	memberNodes[i].delAConnection(j);
	
	// Connect to another node that is not already connected
	for(int check_connection=0; check_connection!=-1;) {
	  tmp = static_cast<double>(rand())/static_cast<double>(RAND_MAX);
	  int k = static_cast<int>(static_cast<double>(n_node)*tmp) + ifirst;
	  if(k<ifirst) k=k+n_node;
	  else if(k>=iend) k=k-n_node;
	  check_connection =
	    memberNodes[i].checkAConnection(memberNodes[k].getId());
	  // check_connection will return -1 when node k is not currently
          //   connected to node i.
	  if(check_connection==-1) {
	    vector<double> tmp_op; tmp_op.clear();
	    tmp_op.push_back(memberNodes[k].getOpinion()); // tmp_op[0] is the opinion of node k
	    tmp_op.push_back(memberNodes[i].getOpinion()); // tmp_op[1] is the opinion of node i
	    // Compute the utility
	    vector<double> ut_tmp = utilityFunction(memberNodes[i].getNodeType(),
						    tmp_op[1],
						    memberNodes[k].getNodeType(),
						    tmp_op[0]);
	    memberNodes[i].addAConnection(memberNodes[k].getId(), tmp_op[0], ut_tmp[0]); // ut_tmp[0] is the utility node k gives node i
	    memberNodes[k].addAConnection(memberNodes[i].getId(), tmp_op[1], ut_tmp[1]); // ut_tmp[1] is the utility node i gives node k
	  }
	} // end of for(check_connection) loop
      } // end of if(tmp<=rw_prob) loop
    } // end of j loop
  } // end of i loop
}

/************************************************************************
   This subroutine links the guest nodes to randomly picked host nodes.
   Input values: 
       $(nLinkEach) is the number of links each guest node will make.
       $(n_host) is the number of host nodes. 
   No return values.
 ***********************************************************************/
void nodeList::linkGuests2RandomHosts(int nLinkEach, int n_host)
{
  int n = memberNodes.size(); // total number of nodes
  // Connect host nodes with neighboring host nodes
  int i=n_host;
  for(; i < n; i++) // looping through the guest nodes
    for(int j=0; j<nLinkEach; j++) { // Each guest node is linked to $(nLinkEach) host nodes.
      for(int check_connection=0; check_connection!=-1;) {
	double tmp = static_cast<double>(rand())/static_cast<double>(RAND_MAX);
	int k = static_cast<int>(static_cast<double>(n_host)*tmp);
	if(k<0) k=k+n_host;
	else if(k>=n_host) k=k-n_host;
	check_connection =
	  memberNodes[i].checkAConnection(memberNodes[k].getId());
	if(check_connection==-1) {
	  vector<double> tmp_op; tmp_op.clear();
	  tmp_op.push_back(memberNodes[k].getOpinion());
	  tmp_op.push_back(memberNodes[i].getOpinion());
	  vector<double> ut_tmp = utilityFunction(memberNodes[i].getNodeType(),
						  tmp_op[1],
						  memberNodes[k].getNodeType(),
						  tmp_op[0]);
	  memberNodes[i].addAConnection(memberNodes[k].getId(), tmp_op[0], ut_tmp[0]);
	  memberNodes[k].addAConnection(memberNodes[i].getId(), tmp_op[1], ut_tmp[1]);
	}
      } // end of for(check_connection) loop
    } // end of j loop for nLink
}

/***********************************************************************
   This subroutine links the guest nodes to randomly picked guest 
      and host nodes, with a given fraction of host links.
   Input values: 
       $(nLinkEach) is the number of links each guest node will make.
       $(n_host) is the number of host nodes. 
       $(h_frac) defines the fraction of links to host nodes
   No return values.
 **********************************************************************/

void nodeList::linkGuests2FractionHosts(int nLinkEach, int n_host, double hfrac)
{
  int n = memberNodes.size();
  // Connect host nodes with neighboring host nodes
  int n_guest = n-n_host;
  int h_link = nLinkEach*hfrac;
  int g_link = nLinkEach - h_link;
  int i=n_host;
  for(; i < n; i++)
    for(int j=0; j<nLinkEach; j++) {
      for(int check_connection=0; check_connection!=-1;) {
	int k;
	if(j<h_link) {
	  double tmp = static_cast<double>(rand())
	              /static_cast<double>(RAND_MAX);
	  k = static_cast<int>(static_cast<double>(n_host)*tmp);
	  if(k<0) k=k+n_host;
	  else if(k>=n_host) k=k-n_host;
	} else {
	  double tmp = static_cast<double>(rand())
	              /static_cast<double>(RAND_MAX);
	  k = n_host + static_cast<int>(static_cast<double>(n_guest)*tmp);
	  if(k<n_host) k=k+n_guest;
	  else if(k>=n) k=k-n_guest;
	  if(k==i) continue;
	}
	check_connection =
	  memberNodes[i].checkAConnection(memberNodes[k].getId());
	if(check_connection==-1) {
	  vector<double> tmp_op; tmp_op.clear();
	  tmp_op.push_back(memberNodes[k].getOpinion());
	  tmp_op.push_back(memberNodes[i].getOpinion());
	  vector<double> ut_tmp = utilityFunction(memberNodes[i].getNodeType(),
						  tmp_op[1],
						  memberNodes[k].getNodeType(),
						  tmp_op[0]);
	  memberNodes[i].addAConnection(memberNodes[k].getId(), tmp_op[0], ut_tmp[0]);
	  memberNodes[k].addAConnection(memberNodes[i].getId(), tmp_op[1], ut_tmp[1]);
	}
      } // end of for(check_connection) loop
    } // end of j loop for nLink
}

/*********************************************************************
  This subroutine makes random social connections between nodes.
   Input values: 
       $(nLinkEach) is the number of links each node will make.
   No return values.
  -----
   Note: When we loop through each node, we connect it to $(nLinkEach)/2
         nodes. So after we go through all the nodes, the average number
         of links for each node would be $(nLinkEach), including the links
         it makes to other nodes and the links other nodes make to it.
 *********************************************************************/
void nodeList::RandomLinks(int nLinkEach)
{
  int n = memberNodes.size(); // get the total number of nodes.
  // Create social connections for each node.
  for(int i=0; i < n; i++)
    for(int j=0; j<nLinkEach/2; j++) { // making $(nLinkEach)/2 connections
      // We randomly pick a node with index k (0 <= k < n) to connect.
      // However, if k=i itself, or k is already connected to i, we will
      //   pick another index until we find a node k that is not yet
      //   connected to node i. 
      for(int check_connection=0; check_connection!=-1;) {
	double tmp = static_cast<double>(rand())/static_cast<double>(RAND_MAX);
	int k = static_cast<int>(static_cast<double>(n)*tmp); 
	if(k<0) k=k+n;
	else if(k>=n) k=k-n;
	if(i==k) continue;  // do it again if k=i.
	check_connection =
	  memberNodes[i].checkAConnection(memberNodes[k].getId());
	// check_connection will return -1 when node k is not currently
        //   connected to node i.
	if(check_connection==-1) {
	  vector<double> tmp_op; tmp_op.clear();
	  tmp_op.push_back(memberNodes[k].getOpinion()); // tmp_op[0] is opinion of node k
	  tmp_op.push_back(memberNodes[i].getOpinion()); // tmp_op[1] is opinion of node i
          // Computing the utility
	  vector<double> ut_tmp = utilityFunction(memberNodes[i].getNodeType(),
						  tmp_op[1],
						  memberNodes[k].getNodeType(),
						  tmp_op[0]);
	  memberNodes[i].addAConnection(memberNodes[k].getId(), tmp_op[0], ut_tmp[0]); // ut_tmp[0] is the utility node k gives node i
	  memberNodes[k].addAConnection(memberNodes[i].getId(), tmp_op[1], ut_tmp[1]); // ut_tmp[1] is the utilit node i gives node k
	}
      } // end of for(check_connection) loop
    } // end of j loop for nLink
}

/*********************************************************************
  This subroutine delete one node from the node list
  Input values:
     $(i) is the index of the node to be deleted.
 *********************************************************************/
void nodeList::delOneNode(int i) {
  if(i>=0 && i<memberNodes.size())
    memberNodes.erase(memberNodes.begin()+i);
  else {
    cout << "Error: Delete a node out of bound." << endl;
    exit(1);
  }
}

/*********************************************************************
  This subroutine sets all the guest nodes to or release them from 
    the idle mode. Their opinions and connections are fixed at
    the idle mode.
  Input values:
     $(value) specifies true or false the guest nodes will be idling.
 *********************************************************************/
void nodeList::setGuestsIdling(bool value) {
  int ntot=num_host+num_guest;
  for(int i=num_host; i<ntot; i++)
    memberNodes.at(i).setIdling(value);
}
