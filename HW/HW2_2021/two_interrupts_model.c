#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RND (rand()/(double )RAND_MAX)
#define BINS 20
#define CYCLES 10000
#define INTERRUPTS 2
#define HIST_WIDTH 4

struct {
    double prob;  /* Probability of interrupt occurence */
    int priority; /* Interrupt Priority - higher number => higher priority */ 
    int run_time; /* Time to service the interrupt */
    int start_time; /* Start time for the interrupt routine */ 
    int arrive_time_present; /* Arrive time for the interrupt currently being serviced */
    int arrive_time_next; /* Arrive time for the interrupt in queue */
    int active; /* active = 1 => Current interrupt is being serviced */
    int missed; /* Stores the count of missed interrupts */
    int pending; /* pending = 1 => Current interrupt is in queue, some other interrupt is active */
    int max_latency;
} inter[INTERRUPTS] = {
  {.prob = 0.4, .priority = 1, .run_time = 3, .active = 0, .missed = 0, .pending = 0, .max_latency = 3 },
  {.prob = 0.03, .priority = 2, .run_time = 5, .active = 0, .missed = 0, .pending = 0, .max_latency = 5 }};

int hist_data[BINS][INTERRUPTS]; /* Stores histogram data for each interrupt */
void histogram(int inter_no, int val); /* Puts val in the right bin for interrupt inter_no */
int clock_time = 0;

main() {
    int fire;
    int trigger[INTERRUPTS];
    int i, j, active, pending, priority;
    time_t seconds;
  
    seconds = time(NULL);

    srand(seconds);

    while (clock_time < CYCLES) {
        /* who is requested */
        for (i=0; i< INTERRUPTS; i++)
            trigger[i] = (inter[i].prob > RND) ? 1 : 0;

        /* who is active (only one can be active at a time!) */
        active = -1;
        for (i=0; i<INTERRUPTS; i++)
            if (inter[i].active == 1)
                active = i;
    
        /* who got missed */
        for (i=0; i<INTERRUPTS; i++)
            if (inter[i].pending == 1 && trigger[i] == 1)
                inter[i].missed++;

        /* who is pending */
        for (i=0; i<INTERRUPTS; i++)
            if (active != -1 && trigger[i] == 1 && (inter[i].pending !=1)) {
                inter[i].pending = 1; 
                inter[i].arrive_time_next = clock_time;
            }

        /* highest priority pending */
        pending = -1; 
        for (i=priority=0; i<INTERRUPTS; i++) 
	       if (inter[i].pending && (priority < inter[i].priority)) {
	       	   pending = i; 
	       	   priority = inter[i].priority;
	       	}

        /* who becomes active (only one can be active at a time!) */
        if (active != -1) /* somebody is active */{
            if (inter[active].run_time <= clock_time - inter[active].start_time) { /* somebody has finished executing */
                inter[active].active = 0;
                histogram(active, clock_time-inter[active].arrive_time_present);
	
                if (pending != -1) { /* Highest priority pending interrupt is made active */
                    inter[pending].active = 1;
                    inter[pending].start_time = clock_time;
                    inter[pending].pending = 0;
                    inter[pending].arrive_time_present = inter[pending].arrive_time_next;
                }
            }
        } else { /* nobody is active */
            fire = -1; /* find highest priority triggered interrupt */
            for (i = priority = 0; i<INTERRUPTS; i++)
                if ((trigger[i] == 1) && (priority < inter[i].priority)) { /* Found someone with a higher priority */
                    if (fire != -1) { /* Make the lower priority interrupt pending */
	       	           inter[fire].pending = 1; 
	       	           inter[fire].arrive_time_next=clock_time;
                    }

                    fire = i; 
                    priority = inter[i].priority;
                }

            if (fire != -1) { /*somebody was triggered */
                /* fire the interrupt with the highest priority */
                inter[fire].active = 1;
                inter[fire].start_time = clock_time;
                inter[fire].arrive_time_present = clock_time;
            }
        }

        clock_time++;
    }

	//prints data for all interrupts
	for (i=0; i<INTERRUPTS; i++){
        printf("Number of missed interrupt %d's: %d\n",i,inter[i].missed);
        printf("Max latency for interrupt %d: %d\n",i,inter[i].max_latency);
        printf("Histogram data for interrupt%d\n",i);

        for (j=0; j<BINS; j++)
            printf("Bin %d count: %d\n",j,hist_data[j][i]);
        
        printf("\n");
	}
}

void histogram(int inter_no , int val){
	int min, max, binSize, binIndex;

	//min and max initialized
	min = inter[inter_no].run_time;
	max = (inter[inter_no].run_time)*HIST_WIDTH;

	binSize = 1;

	//Bin number is found here
	binIndex = 0;
	if (binSize > 0) {
		binIndex = (val - min)/binSize;
		if (binIndex >= BINS)
            binIndex--;

		hist_data[binIndex][inter_no]++;
	}
	else
        hist_data[0][inter_no]++;

	if (val > inter[inter_no].max_latency)
        inter[inter_no].max_latency = val;
}
