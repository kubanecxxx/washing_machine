#ifndef GUI_H
#define GUI_H

#include "ch.h"

#define _DR(num) (*((uint16_t*)(&(BKP->DR##num))))

class Gui;

typedef enum
{
    CLICK, ENTER, LEAVE, PLUS, MINUS, RENDER
} event_t;

typedef struct menu_item_t menu_item_t;
typedef void(*callback_t)(const menu_item_t * item, event_t event, Gui * instance);

typedef struct
{
    uint16_t doba_prani;
    uint16_t doba_machani;
    uint16_t doba_zdimani;
    uint16_t pocet_machani;
    uint16_t posledni_zdimani;
    uint16_t otacky_zdimani;
} parameters_t;

struct menu_item_t
{
    const char * title;
    const char * formatter;
    uint16_t low_limit;
    uint16_t high_limit;
    uint16_t step;    
    callback_t cb;
    uint16_t * value;
    uint8_t menu_index;
};

typedef struct
{
    const char * name;
    const uint8_t * value;
} menu_item_port;

extern parameters_t params;

class Gui
{
public:
    Gui();
    void task();
    bool atMainScreen() const {return _idx == 0; }

    static void callback_parameter(const menu_item_t * item, event_t event, Gui * intance);
    static void callback_parameter_otacky(const menu_item_t * item, event_t event, Gui * intance);
    static void callback_button(const menu_item_t * item, event_t event, Gui * intance);
    static void callback_diag(const menu_item_t * item, event_t event, Gui * intance);
    static void callback_enter_diag(const menu_item_t * item, event_t event, Gui * intance);
    static void cb_title(const menu_item_t * item, event_t event, Gui * intance);

    static void par_plus_minus(const menu_item_t * item, event_t event, Gui * intance);

private:
    uint8_t _idx;
    uint8_t _return_idx;
    const menu_item_t * const _first;
    bool _active;

    void render_title();
    void render_value(uint16_t value);
    void render_value();
    void clear_line(uint16_t line = 1);
    void render_line(const char * text, uint16_t line);

    uint8_t next() const;
    uint8_t prev() const;
    const menu_item_t * screen() const;



};


#endif // GUI_H
