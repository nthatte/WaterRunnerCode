/*
RunningAvg.h
Class implementing running avergae for smoothing measurements
*/

#include "QueueArray.h"

template<typename T>
class RunningAvg
{
	private:
		QueueArray<T> readings; //hold readings within window in a queue
		T total;                //total of readings in window
		uint8_t windowSize;
	
	public:
        RunningAvg(uint8_t wSize);
		T newReading(T reading);

        //returns average of readings in window
		T getAvg(){
			return total/readings.count();
		}
};

//initialize running average class (constructor)
template<typename T>
RunningAvg<T>::RunningAvg(uint8_t wSize)
{
    windowSize = wSize;
    total = 0;
}

//Add a new measurment to window and remove oldest measurement.
//Returns: new average
template<typename T>
T RunningAvg<T>::newReading(T newReading)
{
	total += newReading;
	readings.push(newReading);
	if(readings.count() > windowSize){
		total -= readings.pop();
	}

	return getAvg();
}
