/* ============================================================
   Source codes for the agent data class.
   This file contains 2 of the 3 contructors of an agent.
   The other contrutor is in the header file, which simply
      creates an empty agent.

   Author: Yao-li Chuang
   ============================================================ */
#include"AgentC.hpp"

/************************************************************************
  This contructor creates an agent with coordinates specified 
    by a radius $(r) and an angle $(theta).
  $(vel) and $(force) are set to zero for the moment.
  Inputs --
     r : radius
     theta : angle
 ************************************************************************/
agent::agent(double r, double theta) {
  //Assign an initial position on a ring of R=100 with an angle theta.
  //Setting initial velocity and Force to zero.
  if(!pos.empty()) pos.clear();
  if(!vel.empty()) vel.clear();
  if(!force.empty()) force.clear();
  pos.push_back(r*cos(theta));
  pos.push_back(r*sin(theta));
  for(int i=0; i<2; i++) {
    vel.push_back(0.0);
    force.push_back(0.0);
  }
}

/************************************************************************
  This contructor creates an agent at coordinates randomly chosen
     within a circle.
  $(vel) and $(force) are set to zero for the moment.
  Inputs --
     cxy : (x, y) coordinates of the center of the circle
     r : radius of the circle
 ************************************************************************/
agent::agent(vector<double> cxy, double r) {
  //Assign an initial position on a ring of R=100 with an angle theta.
  //Setting initial velocity and Force to zero.
  if(!pos.empty()) pos.clear();
  if(!vel.empty()) vel.clear();
  if(!force.empty()) force.clear();

  double tmp_r = r*sqrt( static_cast<double>(rand())
			 /static_cast<double>(RAND_MAX) );
  double tmp_th = M_PI*2.0*( static_cast<double>(rand())
				    /static_cast<double>(RAND_MAX) );

  double tmpx = tmp_r*cos(tmp_th);
  double tmpy = tmp_r*sin(tmp_th);

  pos.push_back(cxy.at(0)+tmpx);
  pos.push_back(cxy.at(1)+tmpy);
  for(int i=0; i<2; i++) {
    vel.push_back(0.0);
    force.push_back(0.0);
  }
}


