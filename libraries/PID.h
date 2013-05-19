/**
 *  PID control loop 
 */

#ifndef PID_H
#define PID_H

#include <stdint.h>
#include "Filter.h"
#include "TypeDefs.h"

class PID
{
    private:
        const int16_t Knp;
        const int16_t Kni;
        const int16_t Knd;

        const int16_t Kdp;
        const int16_t Kdi;
        const int16_t Kdd;
 
        int_vel_t oldDesState;
        int_vel_t oldState;

        int16_t oldError;
        Filter<int_accel_t> *accelFilter;

        int16_t output;
    public:
        int32_t integrator;
        int16_t pTerm;
        int16_t iTerm;
        int16_t dTerm;

        /**
         *  Construct a PID controller object.
         *  Requires PID Gains and initial conditions
         */
        PID(const int16_t KpNum, const int16_t KiNum, const int16_t KdNum, 
            const int8_t KpDen, const int8_t KiDen, const int8_t KdDen,
            int_vel_t initState, int_vel_t initDesState, Filter<int_accel_t> *filter)
        : Knp(KpNum), Kni(KiNum), Knd(KdNum), 
            Kdp(KpDen), Kdi(KiDen), Kdd(KdDen), 
            oldState(initState), oldDesState(initDesState), accelFilter(filter), 
            oldError(0), pTerm(0), iTerm(0), dTerm(0), integrator(0), output(0)
        {}

        /**
         *  Calculates output give error
         */
        int16_t Calculate(int_vel_t desState, int_vel_t state, int32_t saturatedOutput)
        {
            int16_t error = desState - state;

            //calculate proportional term
            pTerm = Knp*error>>Kdp;

            //calculate integral term using trapezoidal rule
            integrator = integrator + Kni*(oldError + error) + 
                0*((saturatedOutput - static_cast<int32_t>(output))<<(Kdi-10));  //antiwindup
            iTerm = integrator>>Kdi;
                
            //calculate derivative term
            //int_accel_t accel = error - oldError;
            //dTerm = (Knd*accelFilter->Update(accel)) >> Kdd;

            //save current state as old state
            oldDesState = desState;
            oldState = state;
            oldError = error;

            output = pTerm + iTerm + dTerm;
            
            return output;
        }
};
#endif
