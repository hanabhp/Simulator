/*
 author: Hannaneh B. Pasandi (barahoueipash@vcu.edu)
 file: simulator.cpp
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
#include "updated.h"


safeChannel channel;
std::mutex m;
int channelBusy[MAXNODES] =0;
int accessCount;

long double prop[MAXNODES][MAXNODES];
long double dist0[MAXNODES][MAXNODES];
FILE * fEvent;                /* Pointer to file of events                            */

 	int 
       isDeferring[MAXNODES], /* is the node deferring?                               */
                              /* Is the channel busy?                                 */
       DEBUGGING, 
       done;                  /* Are we done with the simulation yet?                 */
      
   int numNodes,         /* number of nodes in the system                             */
       NUM_REARRIVAL=0,
       channelLength,    /* length of the channel                                     */
       bitsPerPac,       /* number of bits / packet                                   */
       bitsPerAck,       /* number of bits / ack                                      */
       buffPerNode,      /* Maximum size of buffer for each node                      */
       maxCWindow,       /* Maximum contention window size                            */
       minCWindow,       /* Minimum contention window size                            */
       curCWindow,       /* The current contention window size                        */
       numCollisions,    /* The total number of collisions                            */
       numPacketsLost,   /* the number of packets which overflow the nodes' buffers   */
       //numInQ[MAXNODES], /* The number of packets in queue for a node                 */
       numDeferring,     /* Number of nodes deferring                                 */
       totalPackets,     /* Total number of packets sent so far..                     */
       loopcounter,      /* Count the total number of events                          */
       iseed;            /* Seed of random number generator                           */
       
         
double propDelay,        /* End-to-end propagation delay                              */   
       channelCapacity,  /* Capacity of the channel                                   */
       probError,        /* Probability[no error] when transmitting a packet          */
       SIFS,             /* SIFS interval                                             */
       DIFS,             /* DIFS interval                                             */
       T3,               /* Time out interval                                         */
       slotTime,         /* Approximation of the slot time                            */
       packetDelay,      /* The delay a packet has incurred in the system             */
       totalPacketDelay, /* The total delay of all the packets incur in the system    */
       timeNextEvent,    /* Time of next event occurance                              */
       lambdaN,          /* Node arrival rate                                         */ 
       lambdaS,          /* System arrival rate                                       */
       sysTime,          /* Global clock                                              */
       timeXAck,         /* time to transmit an ACK                                   */
       timeXPac,         /* time to transmit a packet                                 */
       I,                /* interrupt time + service delay                            */
       P1,
       P2;
       
bool isBusy[MAXNODES]; /* is the node busy                                          */


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
int initParams(int argc, char * argv[])
   {
    /* This function read in all command line parameters and assigns
     * the proper values to the corresponding variables 
     */
    int counter;
   
    num_nodes=5;               /* set default values - in case not */
    channelLength=100;         /* all command line args are present */     
    propDelay=3.3e-07;
    channelCapacity=2048000;
    bitsPerPac=2048;
    bitsPerAck=8;
    buffPerNode=30;
    maxCWindow=1024;
    minCWindow=0;
    SIFS=(2.5)*(3.3e-07);
    DIFS=(4.0)*(3.3e-07);
    T3=(3.0)*(3.3e-07);
    lambdaS=1.0;
    probError=0.0;
    DEBUGGING=1;
    P1=0.1;
    P2=0.0;


    /* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> command line options */
    if (argc==1) 
      {  printf ("\nUsing the default parameters...\n");  return (1);  }


   for (counter=1;  counter<argc;  counter++)   /* Process args */
     if (argv[counter][0] == '-')               /* If it's a flag */
       {
       switch (argv[counter][1])
          {
           case 'n': numNodes=atoi(argv[counter+1]);        break;
           case 'l': channelLength=atoi(argv[counter+1]);   break;
           case 'T': propDelay=atof(argv[counter+1]);       break;
           case 'c': channelCapacity=atof(argv[counter+1]); break;
           case 'x': bitsPerPac=atoi(argv[counter+1]);      break;
           case 'a': bitsPerAck=atoi(argv[counter+1]);      break; 
           case 'b': buffPerNode=atoi(argv[counter+1]);     break;
           case 'u': maxCWindow=atoi(argv[counter+1]);      break;
           case 'v': minCWindow=atoi(argv[counter+1]);      break;
           case 's': SIFS=atof(argv[counter+1]);            break;
           case 'd': DIFS=atof(argv[counter+1]);            break;
           case 't': T3=atof(argv[counter+1]);              break;
           case 'p': probError=atof(argv[counter+1]);       break;
           case 'S': lambdaS=atof(argv[counter+1]);         break;
           case 'D': DEBUGGING=atoi(argv[counter+1]);       break;
           default :
              { printf ("\nUnrecognized command line option...(%s)(%c).\n\n", 
                     argv[counter], argv[counter][1]);
                 exit(-1);  }
          }  /* End if switch() */
       }  /* End of for loop */

  } /* End if initParams() */


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void initAll(void)
  {
   /* This function goes through and calculates all of the necesary 
    * parameters based on the command line arguments and the default
    * settings - it also initializes all variables to NULL/zero  
    */
   int i;
   
   /*_______________General stuff to set to _________________*/
   timeNextEvent=0;    /* Reset time of next event           */
   sysTime=0;          /* Reset system timer                 */
   timeXPac=0;         /* Reset time to transmit a packet    */
   timeXAck=0;         /* Reset time to transmit an Ack      */ 
   iseed=1837;         /* Seed the random # generator        */
   loopcounter=0;      /* Number of events                   */
   done=FALSE;         /* Done with simulation               */
   totalPackets=0;     /* total packets sent is zero         */
   numCollisions=0;    /* number of collisions               */
   curCWindow=minCWindow;  /* Start with the smallest C win. */
   numPacketsLost=0;   /* No packets lost due to Q overflow  */
   

    for (i=0; i<num_nodes; i++){  /* None of the nodes is     */
       isDeferring[i]=FALSE;     /* deferring                */
       isBusy[i]=false;
     }

    numDeferring=0;              /* nobody is deferring      */

    //channelBusy=FALSE;        /* the channel is assumed idle */

    packetDelay=0;               /* No packet delay          */
    totalPacketDelay=0;          /* no total packet delay    */
    I= (double)(channelLength*bitsPerPac)/channelCapacity; 


   /*_____________General stuff to calculate_______________*/
   lambdaN= lambdaS/((double)num_nodes);  /* Nodal arrival rate */
   timeXPac = (double)bitsPerPac/channelCapacity;
   timeXAck = (double)bitsPerAck/channelCapacity;
   slotTime=100000*(propDelay+timeXPac);

      

   /*____________________Open files_________________________*/          
   if (DEBUGGING) fEvent=fopen("logfile.txt", "w");
   if (DEBUGGING) if (!fEvent) fatal("initAll", "Can't open event file");

  }  /* End of initAll;



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void showParams(FILE * fp)
  {
   /* This function shows all of the values of the command line 
    * argument variables after they have been entered and processed
    */

  if (fp!=NULL)
    {
     /* end of going through all of the command line args */
      fprintf (fp, "\n\nNumber of Nodes............: %d",     num_nodes); 
      fprintf   (fp, "\nChannel length.............: %d ",    channelLength);
      fprintf   (fp, "\nPropagation Delay..........: %1.2e",  propDelay);
      fprintf   (fp, "\nChannel capacity...........: %1.3e",  channelCapacity);
      fprintf   (fp, "\nBits/packet................: %d",     bitsPerPac);
      fprintf   (fp, "\nBits/ACK...................: %d ",    bitsPerAck);
      fprintf   (fp, "\nMax number of buffers/node.: %d",     buffPerNode);
      fprintf   (fp, "\nMax contention window......: %d",     maxCWindow);   
      fprintf   (fp, "\nMin contention window......: %d",     minCWindow);
      fprintf   (fp, "\nSIFS.......................: %1.2e",  SIFS);
      fprintf   (fp, "\nDIFS.......................: %1.2e",  DIFS);
      fprintf   (fp, "\nT3.........................: %1.2e",  T3);
      fprintf   (fp, "\nProb[unsuccessful Packet]..: %1.1lf", probError);
      fprintf   (fp, "\nSystem packet arrival rate.: %lf",    lambdaS);
      fprintf   (fp, "\nNodal packet arrival rate..: %lf",    lambdaN);
      fprintf   (fp, "\nTime to transmit an ack....: %lf",    timeXAck);
      fprintf   (fp, "\nTime to transmit a packet..: %lf\n",  timeXPac);

    }
  } /* showParams */

//if (DEBUGGING) if (fEvent) fprintf (fEvent, "\nMedia free? %s", (channelBusy==FALSE)?"YES":"NO");

vector<Wireless_node> list_nodes;
//vector<Wireless_node> node1;
vector<Packet> buffer1;
vector<Packet> buffer_received;
Wireless_node node1[60];

void Packet::sent(long double current_time1)
{	
	//event *ev=NULL;
	if (first_send_attempt_ == -1)
	{
		first_send_attempt_ = current_time1;
	}
	num_retransmission_attempts_++;
	sent_time_ = current_time1;
	cout<< "  [[[[[[[[   packet sent at time: " << sent_time_ << endl;
	status_ = Packet::SENDING();
}

void Packet::received(long double current_time)
{	
	receive_time_ = current_time;
	cout<< "  [[[[[[[[   packet received at time: " << receive_time_ << endl;
	status_ = Packet::RECEIVED();
}


long double Packet::get_effective_transmission_time() const {
	return receive_time_ - sent_time_;
}

long double Packet::get_total_transmission_time() const {
	return receive_time_ - first_send_attempt_;
}

long double Packet::get_total_delay() const {
	return receive_time_ - first_schedule_time_;
}
void Packet::output() const{
	cout << "PACKET: " << id_ << " from NODE: " << from_ << " [SCH]:" << schedule_time_ << " [FIRS]:" << first_send_attempt_ << " [SENT]:" << sent_time_
		<< " [XP_RCV]:" << expected_receive_time_ << " [RCV]:" << receive_time_ << " [ATMPT]" << num_retransmission_attempts_ <<endl;

}



Packet Wireless_node::create_new_packet(char type, int from,  int to, long double current_time)
{	

	Packet pkt;	
	pkt.set_type(type);
	pkt.set_status(Packet::SCHEDULED());
	pkt.set_from(from);
	pkt.set_to(to);
	if(pkt.get_type()==Packet::ACK())
	{
		node1[from].set_generated_ack_packets(1);
		pkt.set_id(get_generated_ack_packets());
		pkt.set_expected_receive_time(pkt.get_schedule_time() + timeXAck + prop[from][to]);
	}
	else
	{
		
		node1[from].set_generated_packets(1);
		pkt.set_id(get_generated_packets());
		pkt.set_expected_receive_time(pkt.get_schedule_time() + timeXPac + prop[from][to]);
		
	}
	
	pkt.set_first_schedule_time(current_time);
	pkt.set_schedule_time(pkt.get_first_schedule_time()); 

	long double thisSchedule = pkt.get_schedule_time(); //current_time + rnd_transmission.exponential(); 

	return pkt;
}

Packet Wireless_node::broadcast(char type, int from, int to, long double current_time){

}

Packet Wireless_node::create_new_msg_packet(int from, int to, long double current_time)
{
	return create_new_packet(Packet::MSG(), from, to, current_time);
}

Packet Wireless_node::create_new_ack_packet(int from, int to, long double current_time, int pkt_id)
{
	Packet p = create_new_packet(Packet::ACK(), from, to, current_time);
	p.set_payload(pkt_id);
	return p;
}

/*
  Sets the seed of the random variables
*/
void Wireless_node::set_seed(long double trans)
{
     rnd_transmission.set_seed(trans);
}

/*
  Creates instances of the Random Variate Generator and
  also the first event...
*/
void Wireless_node::initialize()
{
	if (!initialized_) {
		rnd_transmission.set_rate(transmission_rate_);
		initialized_ = true;
	}
}

void Wireless_node::output() const{
	for (int j=0; j< num_nodes; ++j)
	cout << "NODE: " << j << " [COMP]" << node1[j].get_completed_transmissions() << " [ATMPT]" 
	<< node1[j].get_attempted_transmissions()<< " [Total-GEN-MSG]" << node1[j].get_generated_packets() << " [GEN-ACK]" << node1[j].get_generated_ack_packets() -600 << " [Average Delay]" << abs (node1[j].get_average_delay()/ num_nodes)<< endl;

}

void Wireless_node::attempt_transmission(Packet *p, long double current_time1) {
	attempted_transmissions_++;
	p->sent(current_time1);
}

void Wireless_node::completed_transmission(Packet *p, long double current_time) {
	completed_transmissions_++;
	p->received(current_time);
	efficiency_ += p->get_effective_transmission_time()/p->get_total_transmission_time();
	efficiency_ = efficiency_ / 2;
	delay_ += p->get_total_delay();
}

void Wireless_node::reschedule_packet(Packet *p, long double time1)
{
	p->set_status(Packet::RESCHEDULED());
	if (p->get_first_schedule_time()==-1)
	{
		p->set_first_schedule_time(p->get_schedule_time());
	}
	p->set_schedule_time(time1);
	p->set_expected_receive_time(p->get_schedule_time()+ prop [p->get_from()] [p->get_to()]);
	int from = p->get_from();
	Packet pk = *p;
	node1[from].getMyVec().push_back(pk);
}

void Wireless_node::collision(Packet *p, long double current) {
	Random_Number_Generator r;
	failed_transmissions_++;
	numCollisions ++;
	int from = p->get_from();
	int to = p->get_to();
	if (backoff){
		gen_rearrival(from, to, current + r.exponential() );
	}
	else{
		gen_rearrival(from, to, current);
	}
}

/* this next method returns TRUE if the simulation is within the desired error range */
bool Wireless_node::is_within_error_range(long double err) const {
	return (efficiency_ > 0) && (abs(efficiency_ - get_expected_efficiency()) < err);
}

long double Wireless_node::get_expected_efficiency() const {
	long double G = transmission_rate_ ; /* propagation_time_;*/
	return G*exp((-1)*G);
}

long double Wireless_node::distance (long double x1, long double y1, long double x2, long double y2)
{
  
  	int distancex = abs(x2 - x1) * abs(x2 - x1);
  	int distancey = abs(y2 - y1) * abs(y2 - y1);

  	long double distance = sqrt(abs(distancex - distancey));

  	return distance;
}

long double Wireless_node::propagation_delay (long double dist)
{
  
	dist= dist/300000000;

  	return dist;
}

long double Wireless_node::range (int from, int to)
{	
	Wireless_node dis;
	long double distance_two;
	long double x1 = list_nodes[from].x;
	long double y1 = list_nodes[from].x;
	long double x2 = list_nodes[to].y;
	long double y2 = list_nodes[to].y;
	distance_two = dis.distance(x1,y1,x2,y2);
	if (distance_two - trans_range_ < 0)
		return true;
	else
		return false;
}


