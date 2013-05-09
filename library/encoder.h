// Base class for incremental encoders
#ifndef ENCODER_H
#define ENCODER_H

#include <arduino.h>
#include <stdint.h>
#include "Filter.h"
class Encoder
{   
    protected:
        /*
        Angles stored in units of milliradians
        Speed stored in units of milliradians/second
        */
        int32_t angleNew;
        int32_t angleOld;
        uint32_t timeNew;
        uint32_t timeOld;
        int32_t speedNew;
        int32_t speedOld;
        Filter<int32_t> *speedFilter;
    
    public:
        Encoder(Filter<int32_t> *filter)
        :angleNew(0), angleOld(0), timeNew(0), timeOld(0),
            speedNew(0), speedOld(0), speedFilter(filter)
        {}

        /*
        Sample the encoder and return measured angle in milliradians.

        Implementation of MeasureAngle() function will be different for each
        encoder counter chip. Hence it is a virtual function.
        */
        virtual int32_t MeasureAngle() = 0;

        /*
        Get a speed measurement taking finite difference between new and last angle. 
        Return filtered result
        */
        int32_t MeasureSpeed()
        {
            angleOld = angleNew;
            timeOld = timeNew;
           
            timeNew = micros();
            MeasureAngle(); //Will update angleNew as well
            speedNew = static_cast<int32_t>(((angleNew - angleOld)*1e6)/(timeNew - timeOld));
            
            return speedFilter->Update(speedNew);
            
        }

        int32_t GetAngle(){return angleNew;}
        int32_t GetSpeed(){return speedFilter->GetFilteredValue();}

};
#endif
