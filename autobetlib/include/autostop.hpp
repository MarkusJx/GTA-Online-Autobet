#ifndef AUTOBET_AUTOSTOP_HPP
#define AUTOBET_AUTOSTOP_HPP

#include <functional>

/**
 * The autostop namespace
 */
namespace autostop {
    /**
     * Check if one (or more) of the stop conditions are met
     * 
     * @return true, if one or more stop conditions are met
     */
    bool checkStopConditions();
}

/**
 * Set the money value to stop at
 * 
 * @param val the value
 */
void set_autostop_money(int val);

/**
 * Get the money value to stop at
 * 
 * @return the value to stop at
 */
int get_autostop_money();

/**
 * Set the time to stop at
 * 
 * @param val the time to stop at
 */
void set_autostop_time(int val);

/**
 * Get the time to stop at
 * 
 * @return the time to stop at
 */
int get_autostop_time();

#endif //AUTOBET_AUTOSTOP_HPP
