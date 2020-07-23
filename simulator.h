/*
 author: Hannaneh B. Pasandi (barahoueipash@vcu.edu)
 file: simulator.h
 */


#include <stdio.h>
#include <stdlib.h>





class packet{

	int id_;
	int source_;
	int dest_;
	int length_; /* ACK length = 64*/

	bool isACK; 
	
	double scheduled_time_; /* for statistics (Network delay)*/
	double sent_time_;
	double received_time_;

	int num_retransmit_; /* if >= 16 drop the packet*/

public:
	packet(int id = 0, int source, int dest, bool ack, double schedule_time, 
	double sent_time, double received_time, int num_retransmit):
		
		id_(id),
		source_(source),
		dest_(dest),
		isAck(ack),
		scheduled_time_(schedule_time),
		sent_time_(sent_time),
		received_time_(received_time),
		num_retransmit_(num_retransmit)
		{}


	void set_id(int i) { id_ = i; }
	int get_id()  const { return id_; }

	void set_type(bool c) { type_ = c; }
	bool get_type()  const { return isACK; }

	void set_schedule_time( double t) { schedule_time_ = t; }
	double get_schedule_time()  const { return schedule_time_; }

	void set_first_schedule_time( double t) { first_schedule_time_ = t; }
	double get_first_schedule_time()  const { return first_schedule_time_; }

	void set_transmission_time(){}

	void set_to(int i) { source_ = i; }
	int get_to()  const { return source_; }

	void set_from(int i) { source_ = i; }
	int get_from()  const { return source_; }

	void set_length(int i) { length_ = i; }
	int get_length() const { return length_; }

	void set_retransmit_attempt(int n)  {
		retransmit_attempt = retransmit_attempt + n;
	};

	double get_retransmit_attempt()  {
		return retransmit_attempt;
	};

	/* Method declaration */
	void sent(long double time);
	void received(long double time);
	void output() const;
	long double get_effective_transmission_time() const;
	long double get_total_transmission_time() const;
	long double get_total_delay() const;


};

class node{
};

class event{
};

class sync{
};

          /* Utility functions */
void queueItUp( int node );
int getNumCompeting( void );
bool Prob( void ); 
bool inRange(double low, double high, double x);
void nextarrival( void );
double uni(void);
void statistics();

