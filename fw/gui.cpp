#include "gui.h"
#include "LCD_ctrl.h"
#include "rotary_encoder.h"
#include "statemachine.h"
#include "inputs.h"
#include "chsprintf.h"
#include <string.h>
#include <stdio.h>
#include "relays.h"
#include "inputs.h"

parameters_t params;
uint16_t manual_screen;

static const menu_item_t menu[] =
{
{"Hlavni", NULL, 0,100,10,  Gui::cb_title, NULL,  0},
{"Doba prani", "%3d minut", 0, 100, 5,  Gui::callback_parameter, &params.doba_prani, 0},
{"Doba machani", "%3d minut", 0, 100, 2,  Gui::callback_parameter, &params.doba_machani, 0},
{"Doba zdimani", "%3d minut", 0, 100, 2,  Gui::callback_parameter, &params.doba_zdimani, 0},
{"Doba pos zdim", "%3d minut", 0, 100, 2,  Gui::callback_parameter, &params.posledni_zdimani, 0},
{"Pocet machani", "%2d      ", 1, 10, 1,  Gui::callback_parameter, &params.pocet_machani, 0},
{"Otacky zdimani", "%3d      ", 0, 1, 1,  Gui::callback_parameter_otacky, &params.otacky_zdimani, 0},    
{"", "", 0, 100, 10,  Gui::callback_diag, 0 , 0},   //diagnostic screen
    {"Samovolny reset", "", 0, 1, 1 , Gui::cb_reset, 0, 0},  //8
    {"Manualne", "", 0, 1, 1, Gui::callback_manual, 0, 0}, //manual menu
    {"Zpatky" , "" , 0, 0, 1, Gui::callback_man, 0, 1}, //10
    {"Ventil" , "" , 0, 1, 1, Gui::callback_man, 0 ,1 },
    {"Cerpadlo" , "" , 0, 2, 1, Gui::callback_man, 0 ,1 },
    {"Zamek" , "" , 0, 3, 1, Gui::callback_man, 0 ,1 },
    {"Motor 1" , "" , 0, 4, 1, Gui::callback_man, 0 ,1 },
    {"Motor 2" , "" , 0, 5, 1, Gui::callback_man, 0 ,1 },
    {"Motor 3" , "" , 0, 6, 1, Gui::callback_man, 0 ,1 },
    {"Topeni" , "" , 0, 7, 1, Gui::callback_man, 0 ,1 },
    //vstupy

{NULL, NULL, 0,0,0,0,0,0xff},
};

#define INDEX_OF_DIAG 10


static const char * alarm_texts[] =
{
    "Ohrev dlouho" , "Napousteni tvra", "Hladinomer", "Vypousteni", "Dvere" , "P. topeni",
    "P. motor", "P. zbytek"
};

Gui::Gui():
    _first(menu)
{    
    _idx = 0;
    _active = false;

    params.doba_machani = _DR(2) & 0xff;
    params.doba_prani = _DR(2) >>8 ;
    params.doba_zdimani = _DR(3) & 0xff;
    params.otacky_zdimani = _DR(3) >> 8;
    params.pocet_machani = _DR(4) & 0xff;
    params.posledni_zdimani = _DR(4) >> 8;
}


uint8_t Gui::next() const
{
    const menu_item_t * cur, *next;
    cur = &_first[_idx];
    next = &_first[_idx + 1];

    if (cur->menu_index == next->menu_index)
    {
        return _idx + 1;
    }
    else
    {
        uint8_t i = _idx;
        while(i)
        {
            next = &_first[i-1];

            if (next->menu_index != cur->menu_index)
            {
                return i;
            }

            i--;
        }
        return 0;
    }
}

uint8_t Gui::prev() const
{
    const menu_item_t * cur, *next;
    cur = &_first[_idx];
    next = &_first[_idx - 1];

    if (cur->menu_index == next->menu_index && _idx > 0)
    {
        return _idx - 1;
    }
    else
    {
        uint8_t i = _idx;
        while( true)
        {
            next = &_first[i+1];

            if (next->menu_index != cur->menu_index)
            {
                return i;
            }

            if (next->title == NULL)
            {
                chDbgAssert(false, "nelze");
            }
            i++;
        }
        return 0;
    }
}

const menu_item_t * Gui::screen() const
{
    return &_first[_idx];
}

void Gui::task()
{
    uint8_t value;
    int8_t dir;
    uint8_t changed = rotenc_get(&value, &dir);
    static uint8_t enc_old;

    uint8_t enc_edge = inputs.b.enc_switch && !enc_old;
    enc_old = inputs.b.enc_switch;

    const menu_item_t * screen = this->screen();

    if (!changed && !enc_edge)
    {
        if (screen->cb)
        {
            screen->cb(screen, RENDER, this);
        }
        return;
    }

    if (!_active && changed)
    {
        if (screen->cb)
        {
            screen->cb(screen, LEAVE, this);
        }
        if (dir > 0)
            _idx = next();
        else
            _idx = prev();

        screen = this->screen();
        if (screen->cb)
        {
            screen->cb(screen, ENTER, this);
        }
    }
    else
    {
        if (screen->cb)
        {
            if (enc_edge )
            {
                screen->cb(screen, CLICK, this);
            }
            if (changed)
            {
                if (dir > 0 )
                {
                    screen->cb(screen, PLUS, this);
                }
                else if (dir < 0 )
                {
                    screen->cb(screen, MINUS, this);
                }
            }
        }
    }

    if (screen->cb)
    {
        screen->cb(screen, RENDER, this);
    }

    _DR(2) = params.doba_machani | params.doba_prani << 8;
    _DR(3) = params.doba_zdimani | params.otacky_zdimani << 8;
    _DR(4) = params.pocet_machani | params.posledni_zdimani << 8;
}

void Gui::render_line(const char * text, uint16_t line)
{
    LCD_xy(0,line);
    LCD_puts(text);
    uint8_t len = strlen(text);
    chDbgAssert(len < 17, "dlouhe");
    len = 16 - len;
    while(len)
    {
        LCD_wr_data(' ');
        len--;
    }
}

void Gui::render_title()
{
    char title[20];
    strcpy(title, screen()->title);
    if (_active)        
        strupr(title);

    render_line(title, 0);
}

void Gui::callback_diag(const menu_item_t * item, event_t event, Gui * instance)
{
    (void) item;
    if (event == RENDER)
    {
        //                0123456789abcdef
        char line1[17] = "h ml mr mf l v p";
        if (outputs.u.heater)
        {
            line1[0] = 'H';
        }

        if (outputs.u.motor_slow_l)
        {
            line1[2] = 'M';
        }

        if (outputs.u.motor_slow_r)
        {
            line1[5] = 'M';
        }

        if (outputs.u.motor_fast)
        {
            line1[8] = 'M';
        }

        if (outputs.u.doorlock)
        {
            line1[0xb] = 'L';
        }

        if (outputs.u.valve)
        {
            line1[0xd] = 'V';
        }

        if (outputs.u.pump)
        {
            line1[0xf] = 'P';
        }

        instance->render_line(line1, 0);

        //              0123456789abcdef
        char ins[17] = "t l h n d m z hf";

        if (inputs.b.temperature)
        {
            ins[0] = 'T';
        }
        if (inputs.b.low_level)
        {
            ins[2] = 'L';
        }
        if (inputs.b.high_level)
        {
            ins[4] = 'H';
        }
        if (inputs.b.nezdimat)
        {
            ins[6] = 'N';
        }
        if (inputs.b.door)
        {
            ins[8] = 'D';
        }
        if (inputs.b.fuse_motor)
        {
            ins[0xa] = 'M';
        }
        if (inputs.b.fuse_zbytek)
        {
            ins[0xc] = 'Z';
        }
        if (inputs.b.fuse_heat)
        {
            ins[0xe] = 'H';
            ins[0xf] = 'F';
        }

        instance->render_line(ins, 1);
    }
}

void Gui::render_value(uint16_t value)
{
    char buffer[20];
    //LCD_xy(0,1);
    chsprintf(buffer, screen()->formatter, value);
    //LCD_puts(buffer);
    render_line(buffer,1);
}

void Gui::render_value()
{
    render_value(*screen()->value);
}

void Gui::callback_button(const menu_item_t * , event_t event, Gui * instance)
{
    if (event == RENDER)
    {
        instance->render_title();
    }
    else if (event == CLICK)
    {

    }
}

void strkat(char * buffer, uint16_t counter)
{
    char temp[10];
    temp[0] = ' ';
    temp[1] = ' ';
    temp[2] = ' ';
    temp[3] = counter + 1 + '0';
    temp[4] = '/';
    temp[5] = params.pocet_machani + '0';
    temp[6] = 0;
    strcat(buffer, temp);
}

void Gui::cb_title(const menu_item_t * , event_t event, Gui * instance)
{
    if (event == RENDER)
    {
        static const char * table[] =
        {
            "Cekani na start", "Napousteni", "Ohrev",
            "Prani:", "Vypousteni", "Zdimani:", "Dopousteni","Machani:","Posl. vypousteni",  "Posledni zdimani"

        };
        //add error messages - timeout of filling, timeout of heating
        chDbgAssert(statemachine::intstance(), "no state machine instance");
        statemachine::state_t s = statemachine::intstance()->getState();


        char buffer[20];

        uint32_t alarms = statemachine::intstance()->alarms.word;

        if (!alarms )
        {
            instance->render_line(table[s], 0);
        }

        if (alarms )
        {
            instance->render_line("Porucha",0);

            for (int i = 0; i < 32; i++)
            {
                if (alarms & 1)
                {
                    instance->render_line(alarm_texts[i], 1);
                    break;
                }
                alarms >>= 1;
            }
        }
        else if (s == statemachine::START)
        {
            if (!inputs.b.door)
                instance->render_line("Zavrit dvere",1);
            else
                instance->render_line("Stiskni tlacitko", 1);
        }
        else if (s == statemachine::WATER || s == statemachine::REFILL)
        {
            //instance->render_line("CekaniNaHladinu", 1);
            statemachine::intstance()->_T_alarm_fill.format_remains(buffer);
            if (s == statemachine::REFILL)
            {
                uint16_t cnt = statemachine::intstance()->getRinsingCounter();
                strkat(buffer, cnt);
            }

            instance->render_line(buffer, 1);
        }
        else if (s == statemachine::HEAT)
        {
            //instance->render_line("Cekanina teplotu", 1);
            statemachine::intstance()->_T_alarm_heat.format_remains(buffer);
            instance->render_line(buffer, 1);
        }
        else if (s == statemachine::WASH_TIME)
        {
            statemachine::intstance()->_T_wash.format_remains(buffer);
            instance->render_line(buffer, 1);
        }
        else if (s == statemachine::SPIN_TIME)
        {
            uint16_t cnt = statemachine::intstance()->getRinsingCounter();
            statemachine::intstance()->_T_spin.format_remains(buffer);
            strkat(buffer, cnt);
            instance->render_line(buffer, 1);
        }
        else if (s == statemachine::RINSE_TIME)
        {
            statemachine::intstance()->_T_rinse.format_remains(buffer);
            uint16_t cnt = statemachine::intstance()->getRinsingCounter();
            strkat(buffer, cnt);
            instance->render_line(buffer, 1);
        }
        else if(s == statemachine::FINAL_SPIN)
        {
            statemachine::intstance()->_T_last_spin.format_remains(buffer);
            instance->render_line(buffer, 1);
        }
        else if (s == statemachine::FINAL_WATER_OUT || s == statemachine::WATER_OUT)
        {
            statemachine::intstance()->_T_alarm_empting.format_remains(buffer);
            if (s == statemachine::WATER_OUT)
            {
                uint16_t cnt = statemachine::intstance()->getRinsingCounter();
                strkat(buffer, cnt);
            }
            instance->render_line(buffer, 1);
        }
        else
        {
            instance->render_line("", 1);
        }
    }
}

void Gui::callback_parameter_otacky(const menu_item_t * item, event_t event, Gui * instance)
{
    static const char * table[] = {"Nizke", "Vysoke"};
    if (event == CLICK)
    {
        instance->_active = !instance->_active;
    }
    else if (event == RENDER)
    {
        instance->render_title();
        //instance->render_value();
        instance->render_line(table[*item->value],1);
    }
    else if (event == PLUS || event == MINUS)
    {
        par_plus_minus(item,event,instance);
    }
}

void Gui::callback_parameter(const menu_item_t * item, event_t event, Gui * instance)
{
    if (event == CLICK)
    {
        instance->_active = !instance->_active;
    }
    else if (event == RENDER)
    {
        instance->render_title();
        instance->render_value();
    }
    else if (event == PLUS || event == MINUS)
    {
        par_plus_minus(item,event,instance);
    }
}

void Gui::par_plus_minus(const menu_item_t * item, event_t event, Gui *)
{
    if (!item->value)
        return;

    uint16_t & value = *item->value;
    int16_t v = value;

    if (event == PLUS)
    {
        v += item->step;
    }
    else if (event == MINUS)
    {
        v -= item->step;
    }

    if (v > item->high_limit)
        v = item->low_limit;

    if (v < item->low_limit)
        v = item->high_limit;
    value = v;
}

void Gui::clear_line(uint16_t line)
{
    LCD_xy(0,line);
    LCD_puts("                ");
}

void Gui::change_menu(uint16_t new_idx)
{
    _return_idx = _idx;
    _idx = new_idx;
}

void Gui::return_menu()
{
    _idx = _return_idx;
}

void Gui::callback_manual(const menu_item_t *manual, event_t event, Gui *instance)
{
    if (event == RENDER)
    {
       instance->render_title();
       instance->clear_line();
    }

    if (event == CLICK)
    {
        if (statemachine::intstance()->getState() == statemachine::START)
            instance->change_menu(INDEX_OF_DIAG);
    }
}

void Gui::cb_reset(const menu_item_t *item, event_t event, Gui *instance)
{
    if (event == RENDER)
    {
        instance->return_menu();
        uint8_t watchdog = (RCC->CSR & RCC_CSR_IWDGRSTF);
        instance->render_value(watchdog);

    }
}

void Gui::callback_man(const menu_item_t *item, event_t event, Gui *instance)
{
    if (event == RENDER)
    {
        instance->render_title();
        instance->clear_line();
    }

    if (event == CLICK)
    {
    switch (item->high_limit)
    {
    case 0:
        outputs_forced.status = 0;
        instance->return_menu();
        break;
    case 1:
        outputs_forced.u.valve = !outputs_forced.u.valve;
        break;
    case 2:
        //invert(pump);
        outputs_forced.u.pump = !outputs_forced.u.pump;
        break;
    case 3:
        outputs_forced.u.doorlock = !outputs_forced.u.doorlock;
        break;
    case 4:
        outputs_forced.u.motor_slow_l = !outputs_forced.u.motor_slow_l;
        break;
    case 5:
        outputs_forced.u.motor_slow_r = !outputs_forced.u.motor_slow_r;
        break;
    case 6:
        outputs_forced.u.motor_fast = !outputs_forced.u.motor_fast;
        break;
    case 7:
        outputs_forced.u.heater = !outputs_forced.u.heater;
        break;
    }
    }
}


