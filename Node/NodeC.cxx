/* ============================================================
   Source codes for the node data class

   Author: Yao-li Chuang
   ============================================================ */
#include"NodeC.hpp"

/*********************************************************************
 Constructor of the node data class.
 Input --
     ntype : type of node (+1: host, -1: guest)
     ini_op : initial opinion
              host node: 0 < ini_op <= 1
              guest node: -1 <= ini_op < 0
**********************************************************************/
node::node(int ntype, double ini_op): idling(false), node_type(ntype),
				      opinion(ini_op),
				      total_utility(0.0), cost(0.0),
				      ut_cost(0.0) {
  static long unsigned int totalId = 0;
  id = totalId++;               // give the agent an id.

  if(!connections.empty()) connections.clear();
  if(!utility.empty()) utility.clear();
  if(!con_op.empty()) con_op.clear();
  if(!con_time.empty()) con_time.clear();
}

/***********************************************************************
   This function adds a node $(addId) to the connection list 
      of the current node.
   We also stores the opinion value of the connected node, 
      as well as the the utility it gives the current node;
      they are given in the input arguments $(add_op) and $(add_ut). 
   Moreover, the utility of the new connection is added to the 
      total_utility of the current node.
************************************************************************/
void node::addAConnection(long unsigned int addId,
			  double add_op, double add_ut) {
  connections.push_back(addId);
  con_time.push_back(0);  // set the time of connection to zero.
  con_op.push_back(add_op);
  utility.push_back(add_ut);
  total_utility += add_ut; // add to the total utility
}

/***********************************************************************
   This is one of the two overloaded functions that delete an existing
     connection.
   This one deletes a connection by giving the node index of the
     linked target. (The node index $(delId) is a long unsigned integer.)
   The return value is an integer, indicating the duration (time steps)
     when this link remained active.
 ***********************************************************************/ 
int node::delAConnection(long unsigned int delId) {
  // First look for the target of deletion via the agent ID.
  int delOffset = -1;
  for(int i=0; i<connections.size(); i++) {
    if(connections[i] == delId) {
      delOffset = i; // Get the link index in $(delOffset)
      break;
    }
  }
  if(delOffset == -1) { // Print error messages if such a link is not found.
    cout << "Error: Attempt to delete a connection to node " << delId << ", which is not in the list." << endl;
    cout << "       This node " << id << " has " << getNumConnections() << " connections." << endl;
    //exit(1);
    return 0;
  }
  // delete the id and various data of the disconnected agent.
  connections.erase(connections.begin()+delOffset);  
  total_utility -= utility[delOffset];  
  utility.erase(utility.begin()+delOffset); 
  con_op.erase(con_op.begin()+delOffset);  
  int rtn_tmp = con_time[delOffset];
  con_time.erase(con_time.begin()+delOffset);
  return(rtn_tmp); // return the duration of connection
}

/***********************************************************************
   This is one of the two overloaded functions that delete an existing
     connection.
   This one deletes a connection by giving the link index. 
     (The link index $(delOffset) is an integer.)
   The return value is an integer, indicating the duration (time steps)
     when this link remained active.
 ***********************************************************************/ 
int node::delAConnection(int delOffset) {
  if(delOffset<0 || delOffset>=connections.size()) {
    // If link index is out of bound, print the error message.
    cout << "Error: delete a connection with an offset out of bound." << endl;
    exit(1);
  }
  // delete the link and its related data.
  connections.erase(connections.begin()+delOffset);
  total_utility -= utility[delOffset];
  utility.erase(utility.begin()+delOffset);
  con_op.erase(con_op.begin()+delOffset);
  int rtn_tmp = con_time[delOffset];
  con_time.erase(con_time.begin()+delOffset);
  return(rtn_tmp); // return the duration of connection
}

/*****************************************************************
  This function checks whether the current node is connected
    to the node $(nId). 
   If not, return -1.
   If it is, return the connection index.
 *****************************************************************/
int node::checkAConnection(long unsigned int nId) {
  // First look for the target of deletion via the agent ID.
  int checkOffset = -1;
  for(int i=0; i<connections.size(); i++) {
    if(connections[i] == nId) {
      checkOffset = i;
      break;
    }
  }
  return checkOffset;
}

/*****************************************************************
  This function returns the utility provided by an existing connection.
  Input:
        $(getId) is the index of the connection 
  Return value:
        The utility generated by this particular connection.
 *****************************************************************/
double node::getUtility(long unsigned int getId) {
  // First look for getId among the connected nodes.
  int getOffset = -1;
  int n = connections.size();
  for(int i=0; i<n; i++) {
    if(connections[i] == getId) {
      getOffset = i;
      break;
    }
  }
  if(getOffset == -1) {
    cout << "Error: Attempt to get the utility with an unconnected node."
	 << endl;
    exit(1);
  }
  // return the utility of the node.
  return(utility[getOffset]);  
}

/*****************************************************************
  This function returns the opinion of a connected partner.
  Input:
        $(getId) is the index of the connection. 
  Return value:
        The opinion of the node linked by this particular connection.
 *****************************************************************/
double node::getConOp(long unsigned int getId) {
  // First look for getId among the connected nodes.
  int getOffset = -1;
  int n = connections.size();
  for(int i=0; i<n; i++) {
    if(connections[i] == getId) {
      getOffset = i;
      break;
    }
  }
  if(getOffset == -1) {
    cout << "Error: Attempt to get the connected opinion with an unconnected node."
	 << endl;
    exit(1);
  }
  // return the utility of the node.
  return(con_op[getOffset]);  
}

/*********************************************************************
  The subroutine below is incomplete and not currently in use.

   The goal is for this subroutine to update $(total_utility), $(cost), 
     and $(ut_cost) at once. However, $(cost) requires the knowledge of 
     the model parameter alpha (par.alpha), which is defined at the
     nodeList level but not known by each node. It would take some 
     modifications to pass down the information. We didn't complete this
     subroutine because we never encountered a situation where we need 
     to know $(total_utility), $(cost), or $(ut_cost) during a single node 
     operation. However, we keep it around just in case such a need might
     arise later. 

   Currently, $(total_utility), $(cost), and $(ut_cost) of each node
     are updated during the model simulation when network connections
     are updated in the subroutine updateConnection within ModelC.cxx. 
**********************************************************************/
void node::computeTotalUtility(void) {
  int n = utility.size();
  total_utility = 0.0;
  for(int i=0; i<n; i++)
    total_utility += utility[i];
  ut_cost = total_utility - cost;
}

