/* ============================================================
   Source codes for the nodeList data class (part III)
   This file contains subroutines and functions related to
     the graphic display, where each node is represented
     by a graphic agent. (The agent data class is defined
     in agentC.hpp.)
	    void updateGraphData
	    void createForceMatrix
	    void updateForceMatrix
	    void updatePosition
	    vector<double> forceFunction
	    vector<double> repulsionFunction

   Author: Yao-li Chuang
   ============================================================ */
#include"../Node/NodeListC.hpp"

/*****************************************************************
  This subroutine update the positions of the graphic agents.
  The force matrix is created and updated for the computation 
     and then cleared at the end.
*****************************************************************/
void nodeList::updateGraphData(void) {
  createForceMatrix();
  updateForceMatrix();
  updatePosition();
  forceMatrix.clear();
}

/*****************************************************************
   This subroutine creates the force matrix and fills it with 0.
*****************************************************************/
void nodeList::createForceMatrix(void) {
  int n=memberNodes.size();
  int m=2*n*n;
  if(!forceMatrix.empty()) forceMatrix.clear();
  forceMatrix.assign(m, 0.0);
}

/*****************************************************************
   This subroutine fills the elements of the force matrix.
       forceMatrix[i][j][k]: the k-th component of the force 
                             on agent(node) i, caused by agent(node) j
*****************************************************************/
void nodeList::updateForceMatrix(void) {
  int n=memberNodes.size();
  int m=2*n*n;
  if(forceMatrix.size()!=m) createForceMatrix();

  for(int i=0; i<n; i++)
    for(int j=i+1; j<n; j++) {
      int ij = i*n + j;
      int ji = j*n + i;
      // Compute the force between nodes i and j. 
      // For connected nodes i and j, they are linked by an elastic force.
      // For unconnected nodes, there is a repulsive force between them.
      if(adjMatrix.at(ij)==1) { // elastic force for connected nodes
	if(adjMatrix.at(ji) != 1) { // For undirectional edges, the adjacency Matrix should be symmetric.
	  cout << "Error: adjMatrix is not symmetric in updateForceMatrix" << endl;
	  exit(1);
	}
	agent ai=memberNodes[i].getGraphAgent(),
	  aj=memberNodes[j].getGraphAgent(); // graphic agents for nodes i,j
	vector<double> ftmp = forceFunction( memberNodes[i].getNodeType(),
					     memberNodes[i].getOpinion(),
					     ai.getPos(),
					     memberNodes[j].getNodeType(),
					     memberNodes[j].getOpinion(),
					     aj.getPos() ); // computing the force on node i, caused by node j
	forceMatrix.at(ij*2) = ftmp[0]; // force on i by j
	forceMatrix.at(ij*2+1) = ftmp[1];
	forceMatrix.at(ji*2) = -ftmp[0]; // force on j by i is the opposite vector
	forceMatrix.at(ji*2+1) = -ftmp[1];
      } else { // repulsive force for unconnected nodes
	agent ai=memberNodes[i].getGraphAgent(),
	  aj=memberNodes[j].getGraphAgent();
	vector<double> ftmp = repulsionFunction( memberNodes[i].getNodeType(),
						 memberNodes[i].getOpinion(),
						 ai.getPos(),
						 memberNodes[j].getNodeType(),
						 memberNodes[j].getOpinion(),
						 aj.getPos() );
	forceMatrix.at(ij*2) = ftmp[0];
	forceMatrix.at(ij*2+1) = ftmp[1];
	forceMatrix.at(ji*2) = -ftmp[0];
	forceMatrix.at(ji*2+1) = -ftmp[1];
	/*
	// In case we do not need the repulsive force.
	forceMatrix.at(ij*2) = 0.0;
	forceMatrix.at(ij*2+1) = 0.0;
	forceMatrix.at(ji*2) = 0.0;
	forceMatrix.at(ji*2+1) = 0.0;
	*/
      } // end of if (adjMatrix element == 1) statement
    } // end of i,j loop
}

/*****************************************************************
   This subroutine updates the positions of the agents according to
     the force defined in forceMatrix.
*****************************************************************/
void nodeList::updatePosition(void) {
  int n=memberNodes.size();
  int m=n*n*2;
  if(forceMatrix.size() != m) createForceMatrix();

  // Save the positions at time t
  vector<double> pos_old; 
  for(int i=0; i<n; i++) {
    vector<double> posi = memberNodes[i].getGraphAgent().getPos();
    pos_old.push_back(posi.at(0));
    pos_old.push_back(posi.at(1));
  }
  // Update the positions to time t+1
  for(int i=0; i<n; i++) {
    //if(num_link.at(i)!=0) {
      vector<double> tforce(2, 0.0);
      for(int j=0; j<n; j++) {
	//if(adjMatrix[i*n + j]==0) continue;
	int ij2 = (i*n+j)*2;
	for(int k=0; k<2; k++)
	  tforce.at(k) += forceMatrix[ij2+k];
      } // end of j loop
      vector<double> pos_new(2, 0.0);
      for(int k=0; k<2; k++)
	pos_new.at(k) = pos_old.at(2*i+k) + tforce.at(k);
      agent tmp_agent;
      tmp_agent.setPos(pos_new);
      tmp_agent.setForce(tforce);
      memberNodes[i].setGraphAgent(tmp_agent);
    } // end of i loop and if num_link[i] not zero
}

/************************************************************************
  This function computes a "force" between 2 graphic agents of linked nodes.
  Here we adopt the Hooke's law F = k(p1-p2)^2, and model the pseudo-force
     as a spring.
  Inputs --
     ntype1 : type of node 1
     x1 : opinion of node 1
     p1 : (x, y) position of node 1 on the display
     ntype2 : type of node 2
     x2 : opinion of node 2
     p2 : (x, y) position of node 2 on the display
  Return values -- 
     The force on agent (node) 1, caused by agent (node) 2, 
        expressed as a vector of 2 double-precision numbers.
     force[0] : x-component of the force.
     force[1] : y-component of the force.
 ************************************************************************/
vector<double> nodeList::forceFunction(int ntype1, double x1, vector<double> p1,
				       int ntype2, double x2, vector<double> p2
				       ) {
  double difop = fabs(x1-x2); // opinion difference between the two nodes
  // The Hooke's coefficient $(spring_k) depends on the opinion difference.
  // The spring is stiffer when the opinion difference is larger.
  double spring_k = 0.01*(1.0-0.5*difop); 
  vector<double> dist, force;
  double distance=0.0;
  for(int i=0; i<2; i++) {
    dist.push_back(p2.at(i) - p1.at(i));
    distance += dist.at(i)*dist.at(i);
  }
  distance = sqrt(distance);
  for(int i=0; i<2; i++) {
    double tmp = spring_k*dist.at(i)*(1.0-50.0/distance); // the force
    force.push_back(tmp);
  }
  
  return(force);
}

/**************************************************************************
  This function computes a short-ranged repulsive force 
     between 2 graphic agents of unconnected nodes.
  For simplicity, a step function is adopted for the repulsion, where
     the repulsion is a constant force within a threshold distance and
     zero beyond. 
  Inputs --
     ntype1 : type of node 1
     x1 : opinion of node 1
     p1 : (x, y) position of node 1 on the display
     ntype2 : type of node 2
     x2 : opinion of node 2
     p2 : (x, y) position of node 2 on the display
  Return values -- 
     The force on agent (node) 1, caused by agent (node) 2, 
        expressed as a vector of 2 double-precision numbers.
     force[0] : x-component of the force.
     force[1] : y-component of the force.
  -----
    Note: Since the force is only used for visual display, the step
          function is adopted to minimize the computational tasks.
          If the subroutine will be used to model actual physical
          interactions among individuals, a formulation of a more
          natural force should be used here.
 **************************************************************************/
vector<double> nodeList::repulsionFunction(int ntype1, double x1,
					   vector<double> p1,
					   int ntype2, double x2,
					   vector<double> p2)
{
  vector<double> dist, force;
  double distance=0.0;
  double dist_threshold = 30.0; // the threshold distance
  for(int i=0; i<2; i++) {
    dist.push_back(p2.at(i) - p1.at(i));
  }
  if(dist.at(0)>dist_threshold || dist.at(0)<-dist_threshold
     || dist.at(1)>dist_threshold || dist.at(1)<-dist_threshold) {
    // If the distance is outside of a square box defined by the threshold
    //   distance, assign 0 to the force.
    //  (Note that this is where the computations are reduced significantly:
    //   instead of computing sqrt(x^2+y^2) for every pair of agents, 
    //   we simply compare x and y to the threshold values, saving the 
    //   computation of the square root for most pairs of the agents.)
    force.assign(2,0.0);
    return(force);
  } else { // Only for those close enough, we compute the distance.
    for(int i=0; i<2; i++) {
      distance += dist.at(i)*dist.at(i);
    }    
    distance = sqrt(distance);
    if(distance>dist_threshold) {
      // Inside the square box, there are still some pairs with a distance
      //   larger than the threshold, we also assign 0 to the force.
      force.assign(2,0.0);
      return(force);
    }
    double mag = -0.01/(distance + 0.00001);
    for(int i=0; i<2; i++) {
      double tmp = mag*dist.at(i); // force = A*vector(x)/(abs(x)+B), where A is a constant magnitude of the force, and B is a small positive number to prevent the denominator from going to zero.
      force.push_back(tmp);
    }
    
    return(force);
  }
}
