/* ============================================================
   Header file for the agent data class
   -----
   Brief Summary: Agent contain graphic data for visual display
                  of the nodes and the network.
   -----
      variables --
          pos: positions of each unit in the display
          vel: velocities specify how the units move in the display.
          force: force between pairs of agents which would affect
   -----
      Note: These properties are for graphic display and 
            not directly associated to the population model. 
            Nonetheless, the positions mean to visually indicate 
            the affinity between units within the society. 

   Author: Yao-li Chuang
   ============================================================ */
#ifndef __AgentC_hpp_INCLUDED__
#define __AgentC_hpp_INCLUDED__

#include"../CCommon.h"

class agent {

public:
  // Constructors and destructor
  agent(void) { pos.clear(); vel.clear(); force.clear(); }
  agent(double r, double theta);
  agent(vector<double> cxy, double r);
  ~agent(void) { pos.clear(); vel.clear(); force.clear(); }
  // Getters and setters
  vector<double> getPos(void) {return pos;}
  void setPos(vector<double> value) {pos = value;}
  vector<double> getVel(void) {return vel;}
  void setVel(vector<double> value) {vel = value;}
  vector<double> getForce(void) {return force;}
  void setForce(vector<double> value) {force = value;}

private:
  vector<double> pos, vel, force;
};


#endif
