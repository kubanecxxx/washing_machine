#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include "ch.h"
#include "gui.h"
#include "relays.h"
#include "sequencer.h"

class ton
{
public:
    ton();
    bool task(bool en, uint32_t systime, uint32_t * remains = NULL);    

    void format_remains(char * buffer);

private:
    uint32_t old_systime;
    uint32_t _remains;
    bool edge;    
};

class seq
{
public:
    seq();
    void task(speed_t speed, uint16_t rotate_time = 60, uint16_t sleep_time = 15);
    void reset();
private:
    ton _tleft;
    ton _toff;
    ton _tright;
    ton _toff2;


    uint8_t _state;

};

class statemachine
{

public:
    typedef enum
    {
        START, WATER, HEAT, WASH_TIME, WATER_OUT,SPIN_TIME,  REFILL, RINSE_TIME, FINAL_WATER_OUT,FINAL_SPIN
    } state_t;

    ton _T_wash;
    ton _T_spin;
    ton _T_last_spin;
    ton _T_rinse;
    ton _T_final_spin_pump;
    ton _T_final_spin_pump2;

    ton _T_alarm_heat;
    ton _T_alarm_fill;
    ton _T_alarm_empting;

    bool _final_spin_pump_sub_machine;
    bool _fspsm2;

    union
    {
        struct
        {
            uint8_t ohrev_dlouho : 1;
            uint8_t napousteni_dlouho: 1;
            uint8_t porucha_hladinomeru: 1;
            uint8_t vypousteni_dlouho :1;
            uint8_t dvere:1;
            uint8_t fh :1;
            uint8_t fm:1;
            uint8_t fz:1;
        } names;
        uint32_t word;
    } alarms;


    void reset();

private:
    state_t & _state;
    uint16_t & _rinsing;
    uint16_t & _forced;
    const Gui & _gui;
    static statemachine * _instance;

    const parameters_t & _pars;

    ton _T_start;
    ton _T_empty;
    ton _T_empty2;
    ton _T_reset;
    ton _T_reset2;

    ton * _timer;

    seq _slow;

    void alarm_processing();

public:
    statemachine(const parameters_t & pars, const Gui & gui);
    void task();

    state_t getState() const {return _state;}
    static statemachine * intstance() {return _instance;}





#define NASOBITKO 60

};



#endif // STATEMACHINE_H
