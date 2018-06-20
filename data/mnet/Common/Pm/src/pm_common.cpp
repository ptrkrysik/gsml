#include <stdio.h>
#include <string.h>
#include <semlib.h>
#include <tickLib.h>

#include "defs.h"
#ifndef UNIT_TEST_PMCLASS
#include "vipermib.h"
#endif
#include "pm/pm_common.h"

/*
 * PM_BaseObject::takeSemaphore
 */
bool PM_BaseObject::takeSemaphore()
{
    if (m_semaphore != NULL)
        semTake(m_semaphore,WAIT_FOREVER);
    return(TRUE);
}

/*
 * PM_BaseObject::giveSemaphore
 */
bool PM_BaseObject::giveSemaphore()
{
    if (m_semaphore != NULL)
        semGive(m_semaphore);
    return(TRUE);
}

/*
 * PM_SimpleCounter::reset
 */
void PM_SimpleCounter::reset()
{
    takeSemaphore();
	currentInterval=0;
    maxValue[currentInterval] = value[currentInterval] = 0;
    giveSemaphore();
}

/*
 * PM_SimpleCounter::increment()
 */
bool PM_SimpleCounter::increment()
{
    takeSemaphore();
    value[currentInterval]++;
	if( maxValue[currentInterval] < value[currentInterval] ) {
		maxValue[currentInterval] = value[currentInterval];
	}
    giveSemaphore();
    return(TRUE);
}


/*
 * PM_SimpleCounter::increment(int x)
 */
bool PM_SimpleCounter::increment(int x)
{
    takeSemaphore();
    value[currentInterval] += x;
	if( maxValue[currentInterval] < value[currentInterval] ) {
		maxValue[currentInterval] = value[currentInterval];
	}
    giveSemaphore();
    return(TRUE);
}


/*
 * PM_SimpleCounter::decrement
 */
bool PM_SimpleCounter::decrement()
{
    takeSemaphore();
    value[currentInterval]--;
    giveSemaphore();
    return(TRUE);
}

/*
 * PM_SimpleCounter::getValue
 */
int PM_SimpleCounter::getValue()
{
    int return_value;
    takeSemaphore();
	return_value = value[currentInterval];
    giveSemaphore();
    return(return_value);

}

/*
 * PM_SimpleCounter::setValue
 */
bool PM_SimpleCounter::setValue(int new_value)
{
    takeSemaphore();
	value[currentInterval]	= new_value;
	if( maxValue[currentInterval] < value[currentInterval] ) {
		maxValue[currentInterval] = value[currentInterval];
	}
    giveSemaphore();
    return(TRUE);

}


/*
 * PM_SimpleCounter::getMaxValue
 */
int PM_SimpleCounter::getMaxValue()
{
    int return_value=0,count;
    takeSemaphore();
	return_value = maxValue[currentInterval];
    giveSemaphore();
    return(return_value);

}



/*
 * PM_SimpleCounter::advanceInterval
 */
void  PM_SimpleCounter::advanceInterval()
{
    takeSemaphore();
    if (++currentInterval >= PM_MAX_INTERVAL) {
        currentInterval = 0;
    }
    maxValue[currentInterval] = value[currentInterval] = 0;
    giveSemaphore();
    return;
}



/*
 * PM_PerCauseCounter::reset
 */
void PM_PerCauseCounter::reset()
{
    int count;
    for(count=0;count<PM_MAX_CAUSES;count++){
            perCauseCounter[count].reset();
    }
}


/*
 * PM_PerCauseCounter::getValue
 */
int PM_PerCauseCounter::getValue(CauseCode cause)
{
    if (cause < PM_MAX_CAUSES)
        return(perCauseCounter[cause].getValue());
    else
        return(0);
}


/*
 * PM_PerCauseCounter::advanceInterval
 */
void PM_PerCauseCounter::advanceInterval()
{
    int count;
    for(count=0;count<PM_MAX_CAUSES;count++) {
    	perCauseCounter[count].advanceInterval();
    }
}

/*
 * PM_PerCauseCounter::increment
 */
bool PM_PerCauseCounter::increment(CauseCode cause)
{
	int index;

	index = pm_map_cause_to_index(cause);

    if (index < PM_CURRENT_MAX_CAUSES)
        perCauseCounter[index].increment();
	else printf("PM ERRRO: Invalid Cause Code %d passed in from API\n", (int) cause );
}


/*
 * PM_PerCauseCounter::decrement
 */
bool PM_PerCauseCounter::decrement(CauseCode cause)
{
	int index;

	index = pm_map_cause_to_index(cause);

    if (index < PM_CURRENT_MAX_CAUSES)
        perCauseCounter[index].decrement();
	else printf("PM ERRRO: Invalid Cause Code %d passed in from API\n", (int) cause );
}


/*
 * PM_StatisticCounter::reset
 */
void PM_StatisticCounter::reset()
{
    /*
     * quicker way of cleaning up all the variables
     * is to memset
     */
    takeSemaphore();
	currentInterval = 0;
	numRecords[currentInterval]=0;
	for(int i=0;i< PM_MAX_RECORDS;i++) records[currentInterval][i]=0;
    giveSemaphore();
}


/*
 * PM_StatisticCounter::addRecord
 */
bool PM_StatisticCounter::addRecord(int value)
{
    takeSemaphore();
    if (numRecords[currentInterval] >= PM_MAX_RECORDS) {
        /*
         * No more records can be added now
         */
        giveSemaphore();
        return(FALSE);
    }
    records[currentInterval][numRecords[currentInterval]++] = value;
    giveSemaphore();
    return(TRUE);
}


/*
 * PM_StatisticCounter::getMeanValue
 */
int PM_StatisticCounter::getMeanValue()
{
    int index,total=0;
    takeSemaphore();
    for(index=0;index<numRecords[currentInterval];index++) {
        total += records[currentInterval][index];
    }
    giveSemaphore();
	if (numRecords[currentInterval] == 0) return(0);
    return(total / numRecords[currentInterval]);
}


/*
 * PM_StatisticCounter::getMaxValue
 */
int PM_StatisticCounter::getMaxValue()
{
    int maxValue=0,index;
    takeSemaphore();
    for(index=0;index<numRecords[currentInterval];index++) {
        if (records[currentInterval][index] > maxValue) {
			maxValue = records[currentInterval][index];
		}
    }
    giveSemaphore();
    return(maxValue);
}


/*
 * PM_StatisticCounter::advanceInterval
 */
void PM_StatisticCounter::advanceInterval()
{
    takeSemaphore();
    int count,last_record;
	BOOL not_zero=FALSE;

	if (numRecords[currentInterval] != 0) {
		last_record = records[currentInterval][numRecords[currentInterval]-1];
		not_zero =  TRUE;
	}

    if (++currentInterval >= PM_MAX_INTERVAL) {
        currentInterval = 0;
    }
    for(count=0;count<PM_MAX_RECORDS;count++)
        records[currentInterval][count] = 0;
	numRecords[currentInterval]=0;
    giveSemaphore();
	if (not_zero) {
		addRecord(last_record);
	}
}

/*
 * PM_AccumTimeCounter::reset
 */
void PM_AccumTimeCounter::reset()
{
    takeSemaphore();
	currentInterval = 0;
	for(int i=0;i< PM_MAX_INTERVAL;i++) 
	{
		accumValue[i]=0;
		lastStart[i]=0;
		state[i]=false;
	}
    giveSemaphore();
}

/*
 * PM_AccumTimeCounter::start
 */
void PM_AccumTimeCounter::start()
{
	// start the counter
    takeSemaphore();
	if(	! state[currentInterval]) {
		lastStart[currentInterval]=tickGet();
		state[currentInterval]=true;
	}
    giveSemaphore();
}

/*
 * PM_AccumTimeCounter::stop
 */
void PM_AccumTimeCounter::stop()
{
	// stop the counter
    takeSemaphore();
	if( state[currentInterval]) {
		accumValue[currentInterval] += (tickGet() - lastStart[currentInterval]);
		state[currentInterval]=false;
	}
    giveSemaphore();
}

/*
 * PM_AccumTimeCounter::getValue
 */
int PM_AccumTimeCounter::getValue()
{
	int value;
	// start the counter
    takeSemaphore();
	if( state[currentInterval] ) {
		// do not change the current state, return the last accumulated value plus the
		// timer value up the now
		value = accumValue[currentInterval] + tickGet() - lastStart[currentInterval];
		giveSemaphore();
		return value;
	}
	else
	{
		// timer is not running, the accumValue shoudl hold the correct data
		value = accumValue[currentInterval];
		giveSemaphore();
		return value;
	}

}


/*
 * PM_AccumTimeCounter::advanceInterval
 */
void PM_AccumTimeCounter::advanceInterval()
{
    takeSemaphore();
	int currentTick;
	if( state[currentInterval] ) {
		// timer is running
		currentTick = tickGet();
		accumValue[currentInterval] += (currentTick - lastStart[currentInterval]);
		state[currentInterval]=false;
		// advance the interval
		currentInterval = (currentInterval + 1 ) % PM_MAX_INTERVAL;
		// carry over the last interval's timer status and tick
		lastStart[currentInterval] = currentTick;
		accumValue[currentInterval] = 0;
		state[currentInterval]=true;
	} else {
		// timer is NOT running, directly advance the interval
		currentInterval = (currentInterval + 1 ) % PM_MAX_INTERVAL;
		lastStart[currentInterval] = 0;
		accumValue[currentInterval] = 0;
		state[currentInterval]=false;
	}
    giveSemaphore();
}






