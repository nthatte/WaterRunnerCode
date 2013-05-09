/*
RunningAvg.h
Class implementing Filter interface to smooth measurements using a running average
*/

#ifndef RUNNING_AVG_H
#define RUNNING_AVG_H

#include "Filter.h"
#include "QueueArray.h"

template<typename T>
class RunningAvg: public Filter<T>
{
	private:
		T total;              //total of readings in window
		T avgValue;           //Current Filtered measurement
		QueueArray<T> values; //hold readings within window in a queue
        const uint8_t windowSize;
	    
	public:
        RunningAvg(const uint8_t wSize):windowSize(wSize), total(0), avgValue(0)
        {}

        //Update running average with a new measurement
		virtual T Update(const T value)
        {
            total += value;
            values.push(value);
            if(values.count() > windowSize){
                total -= values.pop();
            }

            avgValue = total/values.count();
            return avgValue;
        }
        
        //returns average of readings in window
		virtual T GetFilteredValue(){
			return avgValue;
		}
};
#endif
