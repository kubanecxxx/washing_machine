#include "statemachine.h"
#include "inputs.h"
#include "relays.h"
#include "chsprintf.h"
#include <string.h>

statemachine * statemachine::_instance = NULL;

ton::ton()
{
    old_systime = chVTGetSystemTime();
    edge = false;    
}

bool ton::task(bool en, uint32_t systime, uint32_t * remains)
{
    uint32_t s = chVTGetSystemTime();
    if (en && !edge)
    {
        old_systime = s;
    }
    edge = en;

    int32_t r = old_systime + systime - s;
    if (r < 0)
        r = 0;
    _remains = r;

    if (remains)
    {        
        *remains = r;
    }
    if (en)
    {
        if (s - old_systime > systime)
        {
            return true;
        }
    }

    return false;
}

void ton::format_remains(char * buffer)
{
    uint32_t s = ST2S(_remains);

    chsprintf(buffer, "%02d:%02d", s / 60, s %60);
}

statemachine::statemachine(const parameters_t & pars, const Gui & gui):
    _state((*((state_t*)(&(BKP->DR1))))),
    _rinsing(_DR(5)),
    _forced(_DR(6)),
    _gui(gui),
    _pars(pars)
{
    _instance = this;

    //_state = REFILL;
    //_forced = false;
    //_rinsing = 0;



    alarms.word = 0;

    /*
    _pair[0].state = WASH_TIME;
    _pair[0].timer = &_T_wash;

    _pair[1].state = SPIN_TIME;
    _pair[1].timer = &_T_spin;

    _pair[2].state = RINSE_TIME;
    _pair[2].timer = &_T_rinse;

    _pair[3].state = FINAL_SPIN;
    _pair[3].timer = &_T_last_spin;
    */

    //setup the right timer after powerdown
}

void statemachine::task()
{
    bool main = _gui.atMainScreen();

    //timers
    bool t_start = _T_start.task(inputs.b.enc_switch && inputs.b.door, MS2ST(1000));
    bool t_prani = _T_wash.task(_state == WASH_TIME, S2ST(_pars.doba_prani * NASOBITKO));
    bool t_empty = _T_empty.task(_state == WATER_OUT && !inputs.b.low_level, S2ST(5));
    bool t_spin = _T_spin.task(_state == SPIN_TIME, S2ST(_pars.doba_zdimani * NASOBITKO));
    bool t_rinse = _T_rinse.task(_state == RINSE_TIME, S2ST(_pars.doba_machani) * NASOBITKO);
    bool t_empty2 = _T_empty2.task(_state == FINAL_WATER_OUT && !inputs.b.low_level, S2ST(25));
    bool t_last_spin = _T_last_spin.task(_state == FINAL_SPIN, S2ST(NASOBITKO*_pars.posledni_zdimani));

    if (_state == START && main && t_start)
    {
        _state = WATER;
        _rinsing = 0;
        _forced = false;
    }

    else if (_state == WATER && inputs.b.high_level)
    {
        _state = HEAT;
    }

    else if (_state == HEAT && inputs.b.temperature)
    {
        _state = WASH_TIME;        
    }

    else if (t_prani)
    {
        _state = WATER_OUT;
    }

    else if ( t_empty)
    {
        _state = SPIN_TIME;
    }

    else if (t_spin)
    {
        _state = REFILL;
    }

    else if (_state == REFILL && inputs.b.high_level)
    {
        _state = RINSE_TIME;
    }

    else if (t_rinse)
    {
        if (++_rinsing >= _pars.pocet_machani)
        {
            _state = FINAL_WATER_OUT;
        }
        else
        {
            _state = WATER_OUT;
        }
    }

    else if (t_empty2)
    {
        if (inputs.b.nezdimat || _forced)
        {
            _state = START;
        }
        else
        {
            _state = FINAL_SPIN;
        }
    }

    else if(t_last_spin)
    {
        _state = START;
    }

    if (_T_reset.task(_state != START && inputs.b.enc_switch, S2ST(10)))
    {
        _state = FINAL_WATER_OUT;
        _forced = true;
    }

    if (_T_reset2.task(main && inputs.b.enc_switch, S2ST(15)))
    {
        reset();
    }

    //valve during spin simple sequencer
    bool spin = _state == FINAL_SPIN || _state == SPIN_TIME;
    _final_spin_pump_sub_machine =
            _T_final_spin_pump.task(spin && !_fspsm2 , MS2ST(15));
    _fspsm2 = _T_final_spin_pump2.task(spin && _final_spin_pump_sub_machine, MS2ST(60));
    bool pump = spin && !_final_spin_pump_sub_machine;

    outputs.u.heater = ((_state == WASH_TIME || _state == HEAT || _state == WATER)
                        && !inputs.b.temperature && inputs.b.low_level && inputs.b.door);
    outputs.u.valve = ((_state == WATER || _state == REFILL) && !inputs.b.high_level && inputs.b.door);
    outputs.u.pump = (_state == WATER_OUT || _state == FINAL_WATER_OUT || pump);
    outputs.u.doorlock = _state != START;

    //substate machine - washing there and back
    if ((_state == WASH_TIME || _state == RINSE_TIME) && inputs.b.door)
    {
        _slow.task(LOW_SPEED);
    }
    else if ((_state == SPIN_TIME || _state == FINAL_SPIN) && inputs.b.door)
    {
        speed_t s = HIGH_SPEED;

        if (_pars.otacky_zdimani == 0)
            s = LOW_SPEED;

        _slow.task(s);
    }
    else if (_state == HEAT)
    {
        _slow.task(LOW_SPEED, 10, 30);
    }
    else
    {
        relay_stop_motor();
        _slow.reset();
    }

    alarm_processing();

    if (alarms.word)
    {
        outputs.u.heater = 0;
        outputs.u.valve = 0;
        outputs.u.pump = 0;
        relay_stop_motor();
    }
}

void statemachine::reset()
{
    _state = START;
    _forced = false;
    _rinsing = 0;

}

void statemachine::alarm_processing()
{
    alarms.names.ohrev_dlouho = _T_alarm_heat.task(_state == HEAT, S2ST(900));
    alarms.names.napousteni_dlouho = _T_alarm_fill.task(_state == WATER || _state == REFILL, S2ST(120));

    alarms.names.porucha_hladinomeru = (!inputs.b.low_level && inputs.b.high_level);

    alarms.names.vypousteni_dlouho = _T_alarm_empting.task(
                _state == WATER_OUT || _state == FINAL_WATER_OUT, S2ST(120));


    alarms.names.dvere = !inputs.b.door && _state != START;

    alarms.names.fh = !inputs.b.fuse_heat;
    alarms.names.fm = !inputs.b.fuse_motor;
    alarms.names.fz = !inputs.b.fuse_zbytek;

    if (alarms.names.fh || alarms.names.fm || alarms.names.fz)
    {
        outputs.status = 0;
        _state = START;
    }

}

void seq::task(speed_t speed, uint16_t rotate_time, uint16_t sleep_time)
{
    if (_tleft.task(_state == 0, S2ST(rotate_time)))
    {
        _state = 1;
    }
    else if (_toff.task(_state == 1, S2ST(sleep_time)))
    {
        _state = 2;
    }
    else if (_tright.task(_state == 2, S2ST(rotate_time)))
    {
        _state = 3;
    }
    else if (_toff2.task(_state == 3, S2ST(sleep_time)))
    {
        _state = 0;
    }

    if (_state == 0)
    {
        relay_start_motor(LEFT, speed);
    }
    else if (_state == 1)
    {
        relay_stop_motor();
    }
    else if (_state == 2)
    {
        relay_start_motor(RIGHT, speed);
    }
    else if (_state == 3)
    {
        relay_stop_motor();
    }
}

void seq::reset()
{
    _state = 0;
}

seq::seq()
{
    _state = 0;
}

