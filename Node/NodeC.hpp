/* ============================================================
   Header file for the node data class
   -----
   Brief Summary: Node contains the properties of each unit 
                  for the simulated population model.
   -----
      variables --
          id : index of the node
          opinion : opinion of the node
          node_type : type of the node (host or guest)
	  idling : enabling the idling mode (node unchanged in simulation)
          connections : a list of the indices of its social connections
	  utility : a list of rewards provided by $(connections)
          con_op : a list of opinions of the partners via $(connections)
	  total_utility : total reward received from social connections
          cost : cost for maintaining social connections
          ut_cost : the net utility (total_utility - cost)
	  con_time : duration of connection (currently not in use)
       additional -- 
          graphAgent : graphic agents of the node for graphic visualization (see AgentC.hpp for the agent data class)
   -----
       Functions and subroutines not defined explicitly here are
          defined in NodeC.cxx.

   Author: Yao-li Chuang
   ============================================================ */
#ifndef __NodeC_hpp_INCLUDED__
#define __NodeC_hpp_INCLUDED__

#include"../CCommon.h"
#include"../Graphics/AgentC.hpp"

class node {

public:
  // Constructor & destructor
  node(int ntype, double ini_op);
  ~node(void) { deleteAllConnections(); }
  // Getters & setters
  long unsigned int getId(void) {return id;}
  void setId(long unsigned int value) {id = value;}
  bool isIdling(void) {return idling;}
  void setIdling(bool value) {idling = value;}
  double getOpinion(void) {return opinion;}
  void setOpinion(double value) {opinion = value;}
  double getTotalUtility(void) {return total_utility;}
  double getUtCost(void) {return ut_cost;}
  double getCost(void) {return cost;}
  void setCost(double value) {cost = value;}
  int getNodeType(void) {return node_type;}
  void setNodeType(int value) {node_type = value;}
  vector<long unsigned int> getConnections(void) {return connections;}
  long unsigned int getAConnection(int i) {return connections[i];}
  int getNumConnections(void) {return connections.size();}
  vector<double> getUtility(void) {return utility;}
  double getUtility(long unsigned int getId);
  vector<double> getConOp(void) {return con_op;}
  double getConOp(long unsigned int getId);
  agent getGraphAgent(void) {return graphAgent;}
  void setGraphAgent(agent value) {graphAgent = value;}
  // Operators of connections
  void addAConnection(long unsigned int addId, double add_op, double add_ut);
  int delAConnection(long unsigned int delId);
  int delAConnection(int delOffset);
  void deleteAllConnections(void) { connections.clear(); utility.clear();
    con_op.clear(); con_time.clear(); }
  // Note that the above functions of adding/deleting connections only
  //    add/delete a connection from one end of the connection.
  // Thus for undirectional connections, the same connection also need
  //    to be added/deleted from the other end.
  int checkAConnection(long unsigned int checkId); // return offset or -1
  // Compute total utility (incomplete and not currently in use)
  void computeTotalUtility(void);
private:
  long unsigned int id;
  double opinion;
  int node_type;
  double total_utility, cost, ut_cost; 
  bool idling;
  vector<long unsigned int> connections;
  vector<double> utility;
  vector<double> con_op;
  vector<int> con_time; // duration of connection (currently not in use; see the comments of the subroutine updateConnection in ModelC.cxx.)
  agent graphAgent;
};


#endif
