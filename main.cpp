/*
 author: Hannaneh B. Pasandi (barahoueipash@vcu.edu)
 file: main.cpp
 */


using namespace std;
#include <iostream>
#include <fstream>
#include <deque> 
#include <queue>
#include <string>
#include <vector>
#include <fstream>
#include <cmath>
#include <time.h>
#include <random>
#include <string>
#include <sstream>
#include <thread>   
#include <mutex> 
#include "simulator.h"

					

int main(int argc,  char * argv[])
{	

	Wireless_node node;

	ifstream infile;   
	string line;
  	int num = 0;
  	initParams(argc, argv); /* Process command line arguments */       
    	initAll();                	/* Set system parameters */

    	node.init_simulator();
	cout << "------------------------------------------------------ \n";
    	theSimulation.run ();
    	node.output();
    	statistics();
     	cout << "End of Simulation \n";

    	return 0;
}

