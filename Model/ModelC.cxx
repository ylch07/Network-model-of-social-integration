/* ============================================================
   Source codes for the nodeList data class (part II)
   This file contains subroutines and functions related to
     simulating the population model.
            void setDefaultParameters
	    void resetParametersFromFile
	    void changeParameter
	    double hostInitiation
            void nextTimeStep
	    void createAdjMatrix
	    void createUtMatrix
	    void updateUtMatrix
	    void updateOpinion
	    void updateOpinion2
	    void updateOpinionGuest
	    void updateOpinion2Guest
	    vector<double> utilityFunction
	    void evolveAdjMatrix
	    void updateConnection
   -----
    Note:
       There are 2 mechanisms of updating the opinions.
       updateOpinion updates the opinion of each node with respect
                     to the average opinion of all its connected
                     partners.
       updateOpinion2 updates the opinion of each node with respect
                     to the opinion of one node randomly chosen
                     from its connected partners.
       Over a long time, the two should behave similarly, but 
          at short times, the second would appear more stochastic.
       updateOpinionGuest and updateOpinio2Guest are same as above,
          but change only the opinions of guests. Host opinions are
          fixed.

   Author: Yao-li Chuang
   ============================================================ */
#include"../Node/NodeListC.hpp"

/***********************************************************
  This subroutine sets the values of the model parameters.
 ***********************************************************/
void nodeList::setDefaultParameters(void) {
  par.AH = 10.0;
  par.AG = 10.0;
  par.sigmaH = 1.;
  par.sigmaG = 1.;
  par.kappa = 100.0;
  par.alpha = 3.0;
  par.gamma = 1.0;
  par.welfare = 0.0;
  par.enable_op = true;
  par.enable_net = true;
  par.ini_hlink_frac = 0.9;
}

/***********************************************************
  This subroutine resets the values of the model parameters
      from a text input file.
  Input values:
     $(file_name) specifies the name of the input file.
 ***********************************************************/
void nodeList::resetParametersFromFile(string file_name) {
  string line;
  ifstream input_file(file_name.data());
  if (input_file.is_open()) {
    while (getline(input_file, line)) {
      stringstream line_stream(line);
      string pname;
      line_stream >> pname;
      if(   (pname.compare("enable_op")==0)
	 || (pname.compare("enable_net")==0) ) {
	bool value;
	line_stream >> value;
	changeParameter(pname, value);
	cout << "bool" << endl;
	cout << pname.data() << " is " << value << endl;
      } else if(   (pname.compare("n_node")==0)
		|| (pname.compare("immigrant_number")==0) 
		|| (pname.compare("immigrant_ratio")==0) 
		|| (pname.compare("initial_connections")==0) 
		|| (pname.compare("initial_opinions")==0) ) {
	// do nothing (parameters for initial conditions)
      } else {
	double value;
	line_stream >> value;
	changeParameter(pname, value);
	cout << pname.data() << " is " << value << endl;
      } // end of if pname is some string statement
    } // end of getline from input_file loop
    input_file.close();
  } else {
    cout << "Error in resetParametersFromFile in ModelC.cxx: unable to open " << file_name.data() << endl;
    cout << "      The simulation will proceed with the default parameter values." << endl;
  } // end of if file open statement
}

/***********************************************************
  This subroutine changes the values of the real number parameters.
  Input values:
     $(pname) specifies which parameter value to change.
     $(value) gives the value to change to.
 ***********************************************************/
void nodeList::changeParameter(string pname, double value) {
  if(pname.compare("AH")==0)
    par.AH = value;
  else if(pname.compare("AG")==0)
    par.AG = value;
  else if(pname.compare("sigmaH")==0) {
    par.sigmaH = value;
  }
  else if(pname.compare("sigmaG")==0) {
    par.sigmaG = value;
  }
  else if(pname.compare("alpha")==0)
    par.alpha = value;
  else if(pname.compare("gamma")==0)
    par.gamma = value;
  else if(pname.compare("kappa")==0)
    par.kappa = value;
  else if(pname.compare("welfare")==0)
    par.welfare = value;
  else if(pname.compare("ini_hlink_frac")==0)
    par.ini_hlink_frac = value;
  else {
    cout << "no parameter called " << pname << endl;
    exit(1);
  }
}

/***********************************************************
  This subroutine changes the values of the logical parameters.
  Input values:
     $(pname) specifies which parameter value to change.
     $(value) gives the value to change to.
 ***********************************************************/
void nodeList::changeParameter(string pname, bool value) {
  if(pname.compare("enable_op")==0)
    par.enable_op = value;
  else if(pname.compare("enable_net")==0)
    par.enable_net = value;
  else {
    cout << "no parameter called " << pname << endl;
    exit(1);
  }
}

/***********************************************************
  This function evolves the connections of the host nodes
     for 50 time steps, while setting the guest nodes idling. 
  No Input values.
  The return value is the average number of links per host node.
  -----
  This function is used when you would like to start a simulation
    with the host community at a quasi-equilibrium state when
    several guests are introduced.
 ***********************************************************/
double nodeList::hostInitiation(void) {
  setGuestsIdling(true);

  createAdjMatrix();
  createUtMatrix();
  for(int i=0; i<50; i++) {
    updateUtMatrix();
    evolveAdjMatrix();
  }
  updateConnection();
  adjMatrix.clear(); utMatrix.clear();
  setGuestsIdling(false);

  computeStats();
  return(stats.avg_link.at(1));
}

/******************************************************************
   This subroutine simulates the model and advances from time t 
      to the next time step t+1.
 ******************************************************************/
void nodeList::nextTimeStep(void) {
  createAdjMatrix();     // creating the adjacency matrix of time t
  createUtMatrix();      // creating the utility matrix of time t
  // If opinion change is enabled, calculate new opinions of time t+1
  if(par.enable_op) {
    //updateOpinion2Guest();
    updateOpinion2();
  }
  updateUtMatrix();      // updating the utility matrix to time t+1
  // If network remodeling is enabled, evolve the adjacency matrix
  //    to time t+1
  if(par.enable_net) {
    evolveAdjMatrix();
  }
  updateConnection(); // updating the network connections with the new adjacency matrix
  updateGraphData();  // updating the graphic agents for visual display
  adjMatrix.clear(); utMatrix.clear(); // no further use of the adjacency and the utility matrices for this time step.
}

/***********************************************************
  This subroutine creates the adjacency matrix.
    0: disconnected
    1: connected
  Moreover, the member vector $(num_link) is also updated, which
    keeps track of the number of links each node currently has.
 ***********************************************************/
void nodeList::createAdjMatrix(void) {
  int n=memberNodes.size();
  int m=n*n;
  if(!adjMatrix.empty()) adjMatrix.clear();
  if(!num_link.empty()) num_link.clear();
  adjMatrix.assign(m, 0);
  num_link.assign(n, 0);
  for(int i=0; i<n; i++) {
    for(int j=i+1; j<n; j++)
      if(memberNodes[i].checkAConnection(memberNodes[j].getId()) != -1) {
	adjMatrix[i*n + j] = 1;
	adjMatrix[j*n + i] = 1;
      }
    num_link[i] = memberNodes[i].getNumConnections(); // updating num_link
  }
}

/***********************************************************
  This subroutine creates the utility matrix.
 ***********************************************************/
void nodeList::createUtMatrix(void) {
  int n=memberNodes.size();
  int m=n*n;
  if(adjMatrix.size()!=m || num_link.size() != n) createAdjMatrix();
  if(!utMatrix.empty()) utMatrix.clear();
  utMatrix.assign(m, 0.0);
  updateUtMatrix();
}

/***********************************************************
  This subroutine updates the utility matrix.
     utMatrix[i][j] -> utility of node i received from node j
 ***********************************************************/
void nodeList::updateUtMatrix(void) {
  int n=memberNodes.size();
  int m=n*n;
  if(utMatrix.size()!=m) createUtMatrix();

  for(int i=0; i<n; i++)
    for(int j=i+1; j<n; j++) {
      int ij = i*n + j;
      int ji = j*n + i;
      if(adjMatrix.at(ij)==1) {
	if(adjMatrix.at(ji) != 1) {
          // Print error messages if i is linked to j but not vice versa.
	  cout << "Error: adjMatrix is not symmetric in createUtMatrix" << endl;
	  exit(1);
	}
	vector<double> ut_tmp = utilityFunction( memberNodes[i].getNodeType(),
						 memberNodes[i].getOpinion(),
						 memberNodes[j].getNodeType(),
						 memberNodes[j].getOpinion() );
	utMatrix.at(ij) = ut_tmp[0]; // utility of node i given by node j
	utMatrix.at(ji) = ut_tmp[1]; // utility of node j given by node i
      } else {
	utMatrix.at(ij) = 0.0;
	utMatrix.at(ji) = 0.0;
      } // end of if (adjMatrix element == 1) statement
    } // end of i,j loop
}

/***********************************************************
  This subroutine updates the opinions of all nodes.
     At every time step, the opinion of a node is influenced 
     by all of its connected partners, where the influence is
     weighted by the utility given by each partner. 
  (In other words, those who generate more utility have higher
     influence.)
 ***********************************************************/
void nodeList::updateOpinion(void) {
  int n=memberNodes.size();
  int m=n*n;
  if(adjMatrix.size() != m || utMatrix.size()!=m) createUtMatrix();

  // Save opinions of all nodes at the current time t.
  vector<double> op_old;
  for(int i=0; i<n; i++)
    op_old.push_back(memberNodes[i].getOpinion());

  for(int i=0; i<n; i++)
    if(num_link.at(i)!=0) {
      double result = 0.0, tut=0.0;
      int ntype = memberNodes[i].getNodeType();
      for(int j=0; j<n; j++) {
	if(adjMatrix[i*n + j]==0) continue;
	double tmp_ut = utMatrix[i*n + j];
	result += tmp_ut * op_old[j];   // forward Euler
	tut += tmp_ut;
      } // end of j loop
      result = ((par.kappa+par.welfare)*op_old[i]+result)/((par.kappa+par.welfare)+tut); // new opinion
      // set the result to 0 if the new opinion goes to the other side
      if((ntype==1 && result<0) || (ntype==-1 && result>0))
	result = 0;
      memberNodes[i].setOpinion(result);
    } // end of i loop and if (num_link[i] not zero) statement
}

/***********************************************************
  This subroutine updates the opinions of guest nodes.
     At every time step, the opinion of a guest node is influenced 
     by all of its connected partners, where the influence is
     weighted by the utility given by each partner. 
  (In other words, those who generate more utility have higher
     influence.)
 ***********************************************************/
void nodeList::updateOpinionGuest(void) {
  int n=memberNodes.size();
  int m=n*n;
  if(adjMatrix.size() != m || utMatrix.size()!=m) createUtMatrix();

  // Save opinions of all nodes at the current time t.
  vector<double> op_old;
  for(int i=0; i<n; i++)
    op_old.push_back(memberNodes[i].getOpinion());

  for(int i=0; i<n; i++) {
    int ntype = memberNodes[i].getNodeType();
    if(ntype == 1) continue; // skipping the host nodes
    if(num_link.at(i)!=0) {
      double result = 0.0, tut=0.0;
      for(int j=0; j<n; j++) {
	if(adjMatrix[i*n + j]==0) continue;
	double tmp_ut = utMatrix[i*n + j];
	result += tmp_ut * op_old[j];   // forward Euler
	tut += tmp_ut;
      } // end of j loop
      result = ((par.kappa+par.welfare)*op_old[i]+result)/((par.kappa+par.welfare)+tut); // new opinion
      // set the result to 0 if the new opinion goes to the other side
      if((ntype==1 && result<0) || (ntype==-1 && result>0))
	result = 0;
      memberNodes[i].setOpinion(result);
    } // end of if (num_link[i] not zero) statement
  } // end of i loop
}

/***********************************************************
  This subroutine updates the opinions of all nodes.
     At every time step, the opinion of a node is influenced 
     by one of its connected partners, which is randomly selected
     by a probability proportional to the utilities given by 
     these connected partners.
 ***********************************************************/
void nodeList::updateOpinion2(void) {
  int n=memberNodes.size();
  int m=n*n;
  if(adjMatrix.size() != m || utMatrix.size()!=m) createUtMatrix();

  // Save opinions of all nodes at the current time t.
  vector<double> op_old;
  for(int i=0; i<n; i++)
    op_old.push_back(memberNodes[i].getOpinion());

  for(int i=0; i<n; i++)
    if(num_link.at(i)!=0) { // if node i has at least 1 connection
      vector<double> link_op, link_ut;
      double tut=0.0;
      int ntype = memberNodes[i].getNodeType();
      for(int j=0; j<n; j++) {
	if(adjMatrix[i*n + j]==0) continue; // skipping the nodes not connected with node i
	double tmp_ut = utMatrix[i*n + j]; // utility of i given by j
	link_ut.push_back(tmp_ut); // save the utility to array $(link_ut)
	link_op.push_back(op_old[j]); // save the opinion of j to array
	tut += tmp_ut; // add utility to the total utility of i
      } // end of j loop
      tut += par.welfare; // add welfare contribution

      /* ==========
         If node i has at least 1 link, the opinion of node i will 
           shift towards one connected node randomly picked 
           by a probability proportional to the utility it gives node i.
         (In other words, those who generate more utility are more likely
           to influence the opinions of others.)
	  =========== */
      int link_num = link_op.size();
      if(link_num!=0) {
	double tmp = static_cast<double>(rand())
	  /static_cast<double>(RAND_MAX);
	double acc_ut=0.0;
	double ori_op = par.kappa*op_old[i];
	for(int j=0; j<link_num; j++) {
	  double result = ori_op;
	  acc_ut += link_ut.at(j);
	  if(tmp<=(acc_ut/tut)) {
	    result = (result+link_op.at(j))/(par.kappa+1.0); // new opinion
	   // set the result to 0 if the new opinion goes to the other side
	    if((ntype==1 && result<0) || (ntype==-1 && result>0))
	      result = 0;
	    memberNodes[i].setOpinion(result);
	    break;
	  } // end of randomly selecting a neighbor
	} // end of j loop among linked neighbors
      } // end of if (link_num != 0) statement
    } // end of i loop and if (num_link[i] not zero) statement
}

/***********************************************************
  This subroutine updates the opinions of the guest nodes.
     At every time step, the opinion of a guest node is influenced 
     by one of its connected partners, which is randomly selected
     by a probability proportional to the utilities given by 
     these connected partners.
 ***********************************************************/
void nodeList::updateOpinion2Guest(void) {
  int n=memberNodes.size();
  int m=n*n;
  if(adjMatrix.size() != m || utMatrix.size()!=m) createUtMatrix();

  // Save opinions of all nodes at the current time t.
  vector<double> op_old;
  for(int i=0; i<n; i++)
    op_old.push_back(memberNodes[i].getOpinion());

  for(int i=0; i<n; i++) {
    int ntype = memberNodes[i].getNodeType();
    if(ntype==1) continue; // skipping the host nodes
    if(num_link.at(i)!=0) { // if guest node i has at least 1 connection
      vector<double> link_op, link_ut;
      double tut=0.0;
      for(int j=0; j<n; j++) {
	if(adjMatrix[i*n + j]==0) continue; // skipping the nodes not connected with node i
	double tmp_ut = utMatrix[i*n + j]; // utility of i given by j
	link_ut.push_back(tmp_ut); // save the utility to array $(link_ut)
	link_op.push_back(op_old[j]); // save the opinion of j to array
	tut += tmp_ut; // add utility to the total utility of i
      } // end of j loop
      tut += par.welfare; // add the contribution of welfare

      /* =====
         If node i has at least 1 link, the opinion of node i will 
           shift towards one connected node randomly picked 
           by a probability proportional to the utility it gives node i.
         (In other words, those who generate more utility are more likely
           to influence the opinions of others.)
	 ===== */
      int link_num = link_op.size();
      if(link_num!=0) {
	double tmp = static_cast<double>(rand())
	  /static_cast<double>(RAND_MAX);
	double acc_ut=0.0;
	double ori_op = par.kappa*op_old[i];
	for(int j=0; j<link_num; j++) {
	  double result = ori_op;
	  acc_ut += link_ut.at(j);
	  if(tmp<=(acc_ut/tut)) {
	    result = (result+link_op.at(j))/(par.kappa+1.0); // new opinion
	   // set the result to 0 if the new opinion goes to the other side
	    if((ntype==1 && result<0) || (ntype==-1 && result>0))
	      result = 0;
	    memberNodes[i].setOpinion(result);
	    break;
	  } // end of randomly selecting a neighbor
	} // end of j loop among linked neighbors
      } // end of if (link_num != 0) statement
    } // end of if (num_link[i] not zero) statement
  } // end of i loop
}

/******************************************************************
  This function returns the utility between two connected nodes
    based on their opinions and types.
  Input:
      ntype1 - type of node 1
      x1 - opinion of node 1
      ntype2 - type of node 2
      x2 - opinion of node 2
  Return value: a 2D vecor of double floating point
      1st value - utility of node 1 recieved from node 2
      2nd value - utility of node 2 received from node 1
  ------
  Note that the utility here represents only the reward provided
    by each connection. 
  The cost incurrs at each node for maintaining a number of connections
    and will be calcualted separately.
 ******************************************************************/
vector<double> nodeList::utilityFunction(int ntype1, double x1,
					 int ntype2, double x2) {
  vector<double> ut, A, sigma2;
  // Setting the values of the model parameters A and sigma2, 
  //     according to the node types
  if(ntype2 == ntype1) {
    A.push_back(par.AH);
    A.push_back(par.AH);
  } else {
    A.push_back(par.AG);
    A.push_back(par.AG);
  }

  if(ntype1 == 1)
    sigma2.push_back(2.0*par.sigmaH);
  else
    sigma2.push_back(2.0*par.sigmaG);

  if(ntype2 == 1)
    sigma2.push_back(2.0*par.sigmaH);
  else
    sigma2.push_back(2.0*par.sigmaG);

  // Computing utility
  double diff = x1 - x2;
  for(int i=0; i<2; i++) {
    double tmp = A[i] * exp( - (diff*diff/sigma2[i]) );
    ut.push_back(tmp);
    if(ntype1 == ntype2) {
      ut.push_back(tmp);
      break;
    }
  }
  // ut[0] is the utility of node 1, while ut[1] is the utility of node 2
  return(ut);
}

/******************************************************************
  This subroutine evolves the adjacency matrix. 
  Here every node either adds a new edge or cuts an existing edge.
  The probability of adding an edge depends on how many links a node
    currently has, and the target of the new edge is randomly chosen.
  If a node is not adding a new edge, an existing edge is selected
    for deletion, where the probability of selection depends on
    the utility each edge is providing.
  -----
  Note: The current version changes only the adjacency matrix at the
        nodeList level but not the list of connections at the node level.
        the subroutine updateConnection should be called immediately
        to ensure that the list of connections is consistent with
        the new adjacency matrix.
  -----
  To do in the future: It is better to also update the connection list
                       of the node here in this subroutine, instead of
                       having to call updateConnection.
 ******************************************************************/
void nodeList::evolveAdjMatrix(void) {
  int n=memberNodes.size();
  int m=n*n;
  // If something has a wrong size, recreate utility matrix,
  //   which will also correct num_link and adjMatrix
  if(num_link.size() != n || adjMatrix.size() != m
     || utMatrix.size()!=m) createUtMatrix();

  for(int i=0; i<n; i++) {
    /* ===== 
       Here a node is either adding a connection or deleting one.
       The probability of adding a link is reduced 
           by the number of links the node already has.
	===== */
    if(memberNodes.at(i).isIdling()) continue; // skipping the idling nodes
    // Select a candidate to add or break links
    int nlinki = num_link.at(i);
    int j_opt;
    vector<double> ut_opt;
    int check_connection;
    for(bool opt_found=false; opt_found==false;) {
      double tmp = static_cast<double>(rand())
	/static_cast<double>(RAND_MAX);
      int j = static_cast<int>(static_cast<double>(n)*tmp); 
      if(j<0) j=j+n;
      else if(j>=n) j=j-n;
      if(j==i) continue;
      if(memberNodes.at(j).isIdling()) continue; // skipping idling nodes
      int ij=i*n+j, ji=j*n+i;
      check_connection = adjMatrix.at(ij);
      if(nlinki==0) check_connection=0;
      if(check_connection==0) {
	ut_opt = utilityFunction( memberNodes[i].getNodeType(),
				  memberNodes[i].getOpinion(),
				  memberNodes[j].getNodeType(),
				  memberNodes[j].getOpinion() );
	j_opt = j;         // candidate for making a connection
	opt_found = true;
      } else if(check_connection==1) {
	ut_opt.push_back(-utMatrix.at(ij));
	j_opt = j;          // candidate for disconnecting
	opt_found = true;
      }
    } // end of the loop for searching a candidate to add/break connections

    // Check whether add or break a link gets more utility
    /* =====
       Here we check if it increases the net utility by changing 
          the current connection status.
       If j_opt is connected with i, cutting the connection loses the
          rewards from the connection but saves the costs of keeping 
          the connection.
       If j_opt is not connected with i, adding the new connection gets
          additional rewards from the connection but also incurs the
          costs of keeping one additional connection.
       ===== */
    double cost_ori = exp(nlinki/par.alpha);
    double cost_opt;
    if(check_connection==0)
      cost_opt = exp((nlinki+1)/par.alpha); // new cost of adding a link
    else if(check_connection==1)
      cost_opt = exp((nlinki-1)/par.alpha); // new cost of cutting a link
    double diff_ori = - cost_ori;
    double diff_opt = ut_opt.at(0) - cost_opt; 
    if(diff_opt >= diff_ori) { // if changing connections gets more utility
      if(check_connection==0) { // add a link
	int ij=i*n+j_opt, ji=j_opt*n+i;
	adjMatrix.at(ij) = 1;            // update adjacency matrix
	adjMatrix.at(ji) = 1;
	utMatrix.at(ij) = ut_opt.at(0);  // update utility matrix
	utMatrix.at(ji) = ut_opt.at(1);
	num_link.at(i)++;                // num_link increases by 1
	num_link.at(j_opt)++;
      } else if (check_connection==1) { // break a link
	int ij=i*n+j_opt, ji=j_opt*n+i;
	adjMatrix.at(ij) = 0;           // update adjacency matrix
	adjMatrix.at(ji) = 0;
	utMatrix.at(ij) = 0.0;          // update utility matrix
	utMatrix.at(ji) = 0.0;
	num_link.at(i)--;               // num_link decreases by 1
	num_link.at(j_opt)--;
      } // end of adding/breaking a link
    } // end of if statement diff_opt >= diff_ori
  } // end of i loop  
}

/******************************************************************
  This subroutines updates the list of connections (i.e., edges) 
    of each node based on the adjacency matrix. 
  -----
  Note: This subroutine first erases the list of connections of a
        node and then recreate the list from the adjacency matrix.
        The vector $(con_time) is also reset here as a result, 
        rendering it useless in the current version. 
        In order for $(con_time) to properly count the duration of
        connection, the list of connections should be updated in
        the evolveAdjMatrix subroutine, which should leave
        $(con_time) intact. 
 ******************************************************************/
void nodeList::updateConnection(void) {
  int n=memberNodes.size();
  int m=n*n;
  if(adjMatrix.size() != m || utMatrix.size()!=m) {
    cout << "Error in updateconnection: wrong dimension of adjMatrix or utMatrix" << endl;
    exit(1);
  }

  int tot_link = 0;
  int hh_link=0, gg_link=0, hg_link=0;
  double tot_rw = 0.;
  double hh_rw=0., gg_rw=0., hg_rw=0.;
  for(int i=0; i<n; i++) {
    memberNodes[i].deleteAllConnections();  // erasing all connections
    vector<long unsigned int> tmp_connect;
    vector<double> tmp_ut, tmp_op;
    for(int j=0; j<n; j++) {  // recreating all connections from adjMatrix
      int ij = i*n+j;
      int inode = memberNodes[i].getNodeType();
      if(adjMatrix[ij]==1) {
	memberNodes[i].addAConnection( memberNodes[j].getId(), 
				       memberNodes[j].getOpinion(),
				       utMatrix[ij]);  
	int jnode = memberNodes[j].getNodeType();

	// Count the number of host-host, guest-guest, host-guest links,
        //    as well as the rewards they provide
	if(inode == 1) {
	  if(jnode == 1) {
	    hh_link++;
	    hh_rw += utMatrix[ij]; // rewards from host-host links
	  } else {
	    hg_link++;
	    hg_rw += utMatrix[ij]; // rewards from host-guest links
	  } 
	} else {
	  if(jnode == 1) {
	    hg_link++;
	    hg_rw += utMatrix[ij]; // rewards from host-guest links
	  } else {
	    gg_link++;
	    gg_rw += utMatrix[ij]; // rewards from guest-guest links
	  } 
	} 
      } // end of if (adjMatrix[ij]==1) statement.
    } // end of j loop

    // Compute the cost of maintaining the links and 
    //   the rewards provided by those links.
    int nlinki = num_link.at(i);
    memberNodes[i].setCost(exp(static_cast<double>(nlinki)/par.alpha));
    memberNodes[i].computeTotalUtility(); 
    
    tot_link += nlinki;
    tot_rw += memberNodes[i].getTotalUtility(); // total reward node i gets
  } // end of i loop

  // Update the statistics of the average number of links per node.
  //   (total per node, host-host per host, host-guest per host,
  //    host-guest per guest, guest-guest per guest)
  double tmp_link[] = {static_cast<double>(tot_link)/static_cast<double>(n),
		       static_cast<double>(hh_link)/static_cast<double>(num_host),
		       static_cast<double>(hg_link)/static_cast<double>(num_host)/2,
		       static_cast<double>(hg_link)/static_cast<double>(num_guest)/2,
		       static_cast<double>(gg_link)/static_cast<double>(num_guest) };
  stats.avg_link.clear(); stats.avg_link.assign(tmp_link, tmp_link+5);
  // Update the statistics of the reward from each type of links.
  double tmp_rw[] = {tot_rw, hh_rw, gg_rw, hg_rw };
  stats.avg_rw.clear(); stats.avg_rw.assign(tmp_rw, tmp_rw+4);

  dist_up2date = false; // Since connections are changed, set the flag of the distance matrix to false so that it will be recaluclated.
}

