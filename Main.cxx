/* ============================================================
   Main routines that handle the model simulation and graphic display of the
   simulation results. 
   -----
   Subroutines related to model simulation:
      init_model - sets the initial conditions and model parameters.
      read_init_cond - reads the initial conditions from an input file.
      model - runs model simulations
      output - writes simulations to the terminal.
   Subroutines related to graphic display:
      init_Graph - sets graphic parameters.
      display - displays the initial graphics (i.e., the initial conditions)
      idle - executes tasks while the graphic display is not updating
      keys - defines the tasks of keyboard inputs
      MaterialProperties - defines the material properties of graphic objects
      LightingProerties - defines the lighting enviroment of the display

   Author: Yao-li Chuang
   ============================================================ */
#include "Main.H"
#include "Node/NodeListC.hpp"
#include"Graphics/GraphicCommon.hpp"

// Global vairables for the model simulation
nodeList *nlist;         // List of nodes
long int t = 0;          // time 
struct iniConditions {   // Parameters for the initial conditions
  int n_node;
  int immigrant_number;
  double immigrant_ratio; // It is used only when immigrant_number=0.
  int initial_connections;
  double initial_opinions;
} initial_conditions = { 500, 50, 0.1, 5, 1.0 }; // default values

// Global variables for the graphic display
double *x,*c;
bool *connection;
GLUquadricObj *myquadric;
int show_line=1;
int run_id=0;


/********************************************
  Main routine 
  -----
  The basic structure of the main routine is as follows:
  1. The simulation model is initiated.
  2. The graphic interface is initiated.
  3. Several graphic display functions are set.
  4. The model simulation runs in the idle function. 
  -----
  The current version takes no arguments. 
  All the parameter values are hard-coded
    in the source codes. The codes have to
    be recompiled when the parameter values
    are changed.
 ********************************************/
int main(int argc, char* argv[]) {

  int base_id, er_status;
  void init_model(string file_name = "");
  void init_graph(void);
  void display(void);
  void keys(unsigned char, int, int);
  void idle(void);

  // Initiate the model
  string file_name = "";
  if (argc == 2)
    file_name.assign(argv[1]);
  init_model(file_name);

  // Initiate the graphic interface
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(750,750);
  glutInitWindowPosition(-1,-1);
  base_id = glutCreateWindow("SocialIntegration");
  init_graph();

  // The reshape function would be called if the window size is changed.
  //glutReshapeFunc(reshape);  
  // The display function draws the graphic objects in the display
  glutDisplayFunc(display); 
  // The idle function tells the program what to do while it is running
  glutIdleFunc(idle);
  // The mouse function specifies the behaviors of mouse movements & clicks
  //glutMouseFunc(mouse);
  // The keyboard function defines the functions of key pressing.
  glutKeyboardFunc(keys);

  // glutMainLoop is the main loop that keeps the program running.
  glutMainLoop();
}

/*************************************
  Initiation of the graphic parameters.
 *************************************/
void init_graph() {

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  //glColor3f(0.0, 0.0, 0.0);
  glMatrixMode(GL_PROJECTION);
  glOrtho(-200, 200, -200, 200, -200, 200);

  // Initiate an openGL quadric object, which will be defined as a ball
  //   later, used to reprent each node in the graphic display
  myquadric = gluNewQuadric();
  gluQuadricDrawStyle(myquadric, GLU_FILL);
}

/**************************************************************
  The display function draws the graphic objects in the display.
  It is called at the beginning of glutMainLoop(), thus drawing
    the initial display. 
  It is also called by glutPostRedisplay(), usually in the
    idle function, to redraw the display when some of the 
    objects are modified.
 **************************************************************/
void display() {
  void MaterialProperties(float, float, float);
  void LightingProperties(void);

  // clear the display
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // set the view point
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  //gluLookAt(0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
  //glRotatef(-90, 1.0, 0.0, 0.0); // Rotate to a more conventional orientation

  // set the lights
  LightingProperties();

  vector<node> tmpnode = nlist->getMemberNodes();
  int totalN = tmpnode.size();
  // Draw lines between connected nodes
  if(show_line==1) {
    glLineWidth(0.5);
    glBegin(GL_LINES);
    for(int i=0; i<totalN; i++)
      for(int j=i+1; j<totalN; j++)
	if(tmpnode[i].checkAConnection(tmpnode[j].getId())>=0) {
	  glVertex3d(tmpnode[i].getGraphAgent().getPos()[0], 
		     tmpnode[i].getGraphAgent().getPos()[1], 0.0);
	  glVertex3d(tmpnode[j].getGraphAgent().getPos()[0], 
		     tmpnode[j].getGraphAgent().getPos()[1], 0.0);
	}
    glEnd();
  }
  // Draw the nodes as balls
  for(int i=0; i<totalN; i++) {
    glPushMatrix();
    glPushAttrib(GL_LIGHTING_BIT);
    // Draw the hosts and the guests with different colors 
    //   (defined in the MaterialProperties subroutine) 
    if(c[i]>0.0)
      MaterialProperties(1.0-c[i],1.0-c[i],1.0);
    else
      MaterialProperties(1.0,1.0+c[i],1.0+c[i]);
    glTranslated(tmpnode[i].getGraphAgent().getPos()[0],
		 tmpnode[i].getGraphAgent().getPos()[1], 0.0);
    glScaled(3.0,3.0,3.0);
    gluSphere(myquadric, 0.7, 15, 15); // define the object as a sphere
    glPopAttrib();
    glPopMatrix();
  }

  glutSwapBuffers();  // Show the drawn objects in the display
}

/****************************************************************
  This subroutine defines the material properties of 
      the openGL objects. Here we can change the RGB
      values of the color.
  Input --
     r : red    (0.0 <= r <= 1.0)
     g : green  (0.0 <= g <= 1.0)
     b : blue   (0.0 <= b <= 1.0)
  --------
   There are online resources for openGL material settings.
   For example:
      http://devernay.free.fr/cours/opengl/materials.html
 ****************************************************************/
void MaterialProperties(float r, float g, float b) {
	GLfloat ambient[4], diffuse[4], specular[4], shininess=32.0;
	GLfloat mat_color[3] = { r, g, b };
	
	for(int i=0; i<3; i++) {
	  //	ambient[i] = 0.3*mat_color[i];
	  //	diffuse[i] = 0.6*mat_color[i];
	  //	specular[i] = 0.6+0.2*mat_color[i];
		ambient[i] = 0.05*mat_color[i];
		diffuse[i] = 0.4+0.1*mat_color[i];
		specular[i] = 0.04+0.66*mat_color[i];
	}
	shininess = 0.078125;
	ambient[3] = diffuse[3] = specular[3] = 1.0;

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);	
}

/****************************************************************
  This subroutine defines the lighting of the display. 
 ****************************************************************/
void LightingProperties(void) {
  float ambient[] = {0.0, 0.0, 0.0, 1.0};
  float lightings[] = {1.0, 1.0, 1.0, 1.0};
  float position[] = {0.5,0.5,1.0,0.0};
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightings);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightings);

  // Define the direction of the diffuse and the specular lights.
  glLightfv(GL_LIGHT0, GL_POSITION, position);
}

/****************************************************************
  This subroutine defines the key functions.
  Input --
     k : the key pressed (the ANSI character representing the key)
     x : x coordinate of the cursor when the key is pressed
     y : y coordinate of the cursor when the key is pressed
  ---------
  The current key functions:
     q: quit the program
     r: toggle between running and pausing the simulation
     l: turn the lines of connections on or off
     d: get the degree distribution (up to 20) at the moment
     c: print the current number of clusters in the terminal
 ****************************************************************/
void keys(unsigned char k, int x, int y) {
  switch(k) {
     case 'c':
       cout << "Number of clusters = " << nlist->numCluster() << endl;
       break;
     case 'd':
       nlist->degreeConnectionSnapshot(20);
       break;
     case 'l':
       show_line=1-show_line;
       break;
     case 'r':
       run_id=1-run_id;
       break;
     case 'q':
       gluDeleteQuadric(myquadric);
       exit(0);
       break;
       /*
     case 's':
       string filename("avg_connect.txt");
       plist->outputStats(filename);
       break;
       */
  }
  glutPostRedisplay(); // redraw the display
}

/******************************************************************
 This subroutine initiates the model.
 ******************************************************************/
void init_model(string file_name) {
  void read_init_cond(string);

  // If an input file is given, read the initial conditions from it.
  if(file_name.length()>0)
    read_init_cond(file_name); 
  // Use $(initial_conditions.immigrant_number) to define the number of 
  //   guest nodes if it is not zero; 
  //   otherwise, use $(initial_conditions.immigrant_ratio) to define 
  //   the ratio of guest nodes.
  if(initial_conditions.immigrant_number != 0)
    nlist = new nodeList(initial_conditions.n_node,
			 static_cast<int>(initial_conditions.immigrant_number),
			 initial_conditions.initial_connections,
			 initial_conditions.initial_opinions);
  else
    nlist = new nodeList(initial_conditions.n_node, 
			 static_cast<double>(initial_conditions.immigrant_ratio),
			 initial_conditions.initial_connections,
			 initial_conditions.initial_opinions);
  //nlist->hostInitiation();
  // If an input file is given, read the model parameters from it.
  if(file_name.length()>0)
    nlist->resetParametersFromFile(file_name);

  // The vector $(x) and $(c) obtain respectively the (x,y) coordinates
  //     and the opinions to draw the nodes in the graphic display later 
  int n_node = initial_conditions.n_node;
  x = new double[2*n_node];
  c = new double[n_node];
  vector<node> node_list = nlist->getMemberNodes();
  for(int i=0; i<n_node; i++) {
    vector<double> tmpos;
    tmpos = node_list[i].getGraphAgent().getPos();
    x[i] = tmpos[0];
    x[i+n_node] = tmpos[1];
    c[i] = node_list[i].getOpinion();
  }

  // The boolean vector $(connection) specify if there is a connection
  //   between a pair of nodes; here we initiate it with no connections
  //   at all (false) and will update it later.
  int n_square = n_node*n_node;
  connection = new bool[n_square];
  for(int i=0; i<n_square; i++) connection[i] = false;
}

/******************************************************************
 The idle function tells glutMainLoop what to do while the main
    loop is running.
 ******************************************************************/
void idle(void) {
  void model(int);
  void output(void);

  // $(run_id) toggles between running and pausing the model simualtion.
  if(run_id) {
    model(10);     // simulating the model
    if(t%10==0)  // output the results every 10 steps
       output();
    //run_id = 1-run_id;    // if we want to pause the simulation after every step
  }
  glutPostRedisplay(); // redraw the display
}

/******************************************************************
  This subroutine runs the model simulation.
  Input ---
     t_steps: number of time steps for the model to proceed
  No return values ---
     The results will be updated to the global variables $(x) and $(c)
     for the other openGL subroutines to draw the graphics.
 ******************************************************************/
void model(int t_steps) {
  for(int i=0; i<t_steps; i++)
    nlist->nextTimeStep();
  t += t_steps;

  // Update the global variables $(x) and $(c)
  vector<node> node_list = nlist->getMemberNodes();
  int n_node = nlist->getNumMemberNodes();
  for(int i=0; i<n_node; i++) {
    vector<double> tmpos;
    tmpos = node_list[i].getGraphAgent().getPos();
    x[i] = tmpos[0];
    x[i+n_node] = tmpos[1];
    c[i] = node_list[i].getOpinion();
  }

}

/******************************************************************
 This subroutine reads the initial conditions from an input file.
  Input ---
     file_name: name of the input file
 +++++
  Note ---
     Whenever a new parameters for the initial conditions is added
     here, remember to set an exception in the subroutine
     resetParametersFromFile in Model/ModelC.cxx, or else 
     it will return an error message.
 ******************************************************************/
void read_init_cond(string file_name) {
  string line;
  ifstream input_file(file_name.data());
  if (input_file.is_open()) {
    while (getline(input_file, line)) {
      stringstream line_stream(line);
      string pname;
      line_stream >> pname;
      if(pname.compare("n_node")==0) {
	int value;
	line_stream >> initial_conditions.n_node;
      } else if(pname.compare("immigrant_number")==0) {
	int value;
	line_stream >> initial_conditions.immigrant_number;
      } else if(pname.compare("immigrant_ratio")==0) {
	double value;
	line_stream >> initial_conditions.immigrant_ratio;
      } else if(pname.compare("initial_connections")==0) {
	int value;
	line_stream >> initial_conditions.initial_connections;
      } else if(pname.compare("initial_opinions")==0) {
	double value;
	line_stream >> initial_conditions.initial_opinions;
      } // end of if pname is some string statement
    } // end of getline from input_file loop
    input_file.close();
  } else {
    cout << "Error in read_init_cond in Main.cxx: unable to open " << file_name.data() << endl;
    cout << "      The simulation will proceed with the default parameter values." << endl;
  } // end of if file is open statement
}


/******************************************************************
  This subroutine prints the statistical results in the terminal
 ******************************************************************/
void output(void) {

  // Update the statistics
  nlist->computeStats();

  struct modelStats stats = nlist->getStats(); // get the statistics

  vector<double> alink = stats.avg_link, aut=stats.avg_ut, aop=stats.avg_op, arw=stats.avg_rw;
  cout << "Time = " << t << '\n';
  double guest_ratio = static_cast<double>(nlist->getNumGuest())
                      /static_cast<double>(nlist->getNumMemberNodes());
  cout << "Average number of links per node: all, h2h/h, h2g/h, g2h/g, g2g/g " << '\n';
  for(int i=0; i<5; i++)
    cout << '\t' << alink.at(i);
  cout << '\n';
  double iint = (alink.at(3)/(alink.at(3)+alink.at(4)))/(1-guest_ratio);
  cout << "Indicator of guest integration = " << iint << '\n';
  cout << "Average utility per node: all, host, guest " << '\n';
  for(int i=0; i<3; i++)
    cout << '\t' << aut.at(i);
  cout << '\n';
  cout << "Average opinion per node: all, host, guest " << '\n';
  for(int i=0; i<3; i++)
    cout << '\t' << aop.at(i);
  cout << '\n';
  double uguest = aut.at(2)/aut.at(1);
  double cross_ratio = 2.0*guest_ratio*(1.0-guest_ratio);
  double rwcross = (arw.at(3)/arw.at(0))/cross_ratio;
  cout << "Guest utility compares to host utility = " << uguest << '\n' << "Rewards through host-guest links compares to the fair share = " << rwcross << '\t';
    
  cout << endl;
  //if(alink.at(3)<0.001) run_id=0; // pause the simulation
}
