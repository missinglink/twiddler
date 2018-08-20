#include <TM1638.h>

// connect to TM1638 with data pin, clock pin, strobe pin
TM1638 module(5, 6, 7);

// constants
#define MODE_DECIMAL 0
#define MODE_HEXADECIMAL 1
#define MODE_BINARY 2
#define MODE_FONT 3
#define TICK_FREQ 100
byte logo[] = { 120,190,52,94,94,48,123,80 };

// button values
#define BTN1 byte(1)
#define BTN2 byte(2)
#define BTN3 byte(4)
#define BTN4 byte(8)
#define BTN5 byte(16)
#define BTN6 byte(32)
#define BTN7 byte(64)
#define BTN8 byte(128)

// dot values
#define DOT1 byte(128)
#define DOT2 byte(64)
#define DOT3 byte(32)
#define DOT4 byte(16)
#define DOT5 byte(8)
#define DOT6 byte(4)
#define DOT7 byte(2)
#define DOT8 byte(1)

// inital state
byte mode = MODE_DECIMAL;
byte state[] = { byte(0),byte(0),byte(0),byte(0) };
byte dots;
byte page = 3;
byte buttons = 0;
byte brightness = 1;
unsigned int counter = 0;
boolean skipNextButtonRelease = false;
boolean leadingZeros = false;
boolean tick = true;

void setup(){
    Serial.begin(9600);
    Serial.print("Hello world.");

    // turn display on and set intensity (range from 0-7)
    module.setupDisplay(true, brightness);

    welcome(3000);
}

// print the welcome message
void welcome(int d){
    module.setDisplay(logo);
    delay(d);
}

void loop(){
    ticker();
    inputs();
    display();
    leds();
    Serial.print("loop");
    Serial.print(state[page]);
}

// ticker provide a boolean which can be used
// to render blinking lights based on a timer.
void ticker(){
    counter++;
    if( counter >= TICK_FREQ ){
        tick = !tick;
        counter = 0;
    }
}

// zero state to 0 across all pages
void zero(){
    for( int i = 0; i < 4; i++ ) {
        state[i] = 0;
    }
}

// increment state
void inc(byte n){
    unsigned int i = byteArrayTo32BitInteger(state) + n;
    setStateFrom32BitInteger( constrain( i, 0, 99999999L ) );
}

// decrement state
void dec(byte n){
    unsigned int i = byteArrayTo32BitInteger(state) - n;
    setStateFrom32BitInteger( constrain( i, 0, 99999999L ) );
}

void inputs(){
    byte newButtonState = module.getButtons();
    if( newButtonState == 0 && buttons > 0 ){ onButtonsReleased(); }
    if( newButtonState != buttons ){ onButtonChange(newButtonState); }
    onButtonEvent(newButtonState);
}

void onButtonChange(byte newButtonState){

    // multi-button combination with button 8
    // controls mode selection functions
    if( newButtonState > BTN8 ){
        
        // mode selection
        if( newButtonState == BTN8 + BTN1 ){ mode = MODE_DECIMAL; }
        if( newButtonState == BTN8 + BTN2 ){ mode = MODE_HEXADECIMAL; }
        if( newButtonState == BTN8 + BTN3 ){ mode = MODE_BINARY; }
        if( newButtonState == BTN8 + BTN4 ){ mode = MODE_FONT; }
        
        // page selection
        if( newButtonState == BTN8 + BTN5 ){ page = constrain( page - 1, 0, 3 ); }
        if( newButtonState == BTN8 + BTN6 ){ page = constrain( page + 1, 0, 3 ); }
        
        // welcome message
        if( newButtonState == BTN8 + BTN7 ){ welcome(3000); }
        
        skipNextButtonRelease = true;
    }

    // multi-button combination with button 7
    // controls incrementing / decrementing / clearing state
    else if( newButtonState > BTN7 && newButtonState < BTN8 ){

        // increment/decrement state +/- 1
        if( newButtonState == BTN7 + BTN1 ){ dec( 1 ); }
        if( newButtonState == BTN7 + BTN2 ){ inc( 1 ); }

        // increment/decrement live for combos:
        // BTN7 + BTN3
        // BTN7 + BTN4

        // reset state
        if( newButtonState == BTN7 + BTN5 ){ zero(); }

        skipNextButtonRelease = true;
    }

    // multi-button combination with button 4
    // display / device settings
    else if( newButtonState > BTN4 && newButtonState < BTN5 ){

        // brightness down
        if( newButtonState == BTN4 + BTN1 ){
            brightness = constrain( brightness - 1, 0, 7 );
            module.setupDisplay(brightness > 0, brightness);
        }

        // brightness up
        if( newButtonState == BTN4 + BTN2 ){
            brightness = constrain( brightness + 1, 0, 7 );
            module.setupDisplay(brightness > 0, brightness);
        }

        // enable/disable leading zeros
        if( newButtonState == BTN4 + BTN3 ){ leadingZeros = !leadingZeros; }
        
        skipNextButtonRelease = true;
    }

    buttons = newButtonState;
}

// 'live' button events, for trigger which aren't only on keyup
// triggered once per loop cycle
void onButtonEvent(byte newButtonState){
    // multi-button combination with button 7
    if( newButtonState > BTN7 && newButtonState < BTN8 ){
        // increment/decrement state +/- 1 per cycle
        if( newButtonState == BTN7 + BTN3 ){ dec( 1 ); }
        if( newButtonState == BTN7 + BTN4 ){ inc( 1 ); }
    }
}

// only trigger button changes when transitioning
// from a pressed state to an unpressed state
void onButtonsReleased(){

    // support for skipping button releases
    // when triggering multi-button workflows.
    if( skipNextButtonRelease ){
        skipNextButtonRelease = false;
        return;
    }

    // skip for multi-button triggers
    if( bitOnlyOneSet(buttons) ){
        // if buttons are pressed, we toggle bits in state[page]
        for( int i = 0; i < 8; i++ ) {
            if( bitCheck(buttons, i) > 0 ){
                bitToggle(state[page], 7-i);
            }
        }
    }
}

// set the decimal point dot state
void setDots(){
    // reset dots
    dots = byte(0);

    // pressed buttons indicators
    // dots |= byteReverse(buttons);

    // mode indicator
    if( mode == 0 ){ dots |= DOT1; }
    else if( mode == 1 ){ dots |= DOT2; }
    else if( mode == 2 ){ dots |= DOT3; }
    else if( mode == 3 ){ dots |= DOT4; }

    // active pages indicators
    if( page != 0 && state[0] > 0 ){ dots |= DOT5; }
    if( page != 1 && state[1] > 0 ){ dots |= DOT6; }
    if( page != 2 && state[2] > 0 ){ dots |= DOT7; }
    if( page != 3 && state[3] > 0 ){ dots |= DOT8; }

    // current page indicator (blinking)
    if( tick ){
        if( page == 0 ){ dots |= DOT5; }
        else if( page == 1 ){ dots |= DOT6; }
        else if( page == 2 ){ dots |= DOT7; }
        else if( page == 3 ){ dots |= DOT8; }
    }
}

void display(){
    setDots();
    if( mode == MODE_DECIMAL ){
        module.setDisplayToDecNumber(byteArrayTo32BitInteger(state), dots, leadingZeros);
    } else if( mode == MODE_HEXADECIMAL ){
        module.setDisplayToHexNumber(byteArrayTo32BitInteger(state), dots, leadingZeros);
    } else if( mode == MODE_BINARY ){
        module.setDisplayToBinNumber(state[page], dots);
    } else if( mode == MODE_FONT ){
        byte nr = state[3];
        byte font[] = { nr,nr,nr,nr,nr,nr,nr,nr };
        module.setDisplay(font);
    }
}

void leds(){
    for( int i = 0; i < 8; i++ ) {
        if( bitCheck(state[page], i) > 0 ){
            module.setLED(TM1638_COLOR_RED, 7-i);
        } else {
            module.setLED(TM1638_COLOR_NONE, 7-i);
        }
    }
}

// void bitSet(byte &b, int pos){ b |= 1UL << pos; }
// void bitClear(int &b, int pos){ b &= ~(1UL << pos); }
void bitToggle(byte &b, int pos){ b ^= 1UL << pos; }
int bitCheck(byte b, int pos){ return (b >> pos) & 1U; }
int bitOnlyOneSet(byte b){ return b && !(b & (b-1)); }

// b = ((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;
byte byteReverse(byte c){
    c = ((c>>1)&0x55)|((c<<1)&0xAA);
    c = ((c>>2)&0x33)|((c<<2)&0xCC);
    c = (c>>4) | (c<<4);
    return c;
}

// long int byteArrayTo64BitInteger(byte bytes[]){
//     unsigned int i = 0;
//     i |= bytes[0]; i = i << 8;
//     i |= bytes[1]; i = i << 8;
//     i |= bytes[2]; i = i << 8;
//     i |= bytes[3]; i = i << 8;
//     i |= bytes[4]; i = i << 8;
//     i |= bytes[5]; i = i << 8;
//     i |= bytes[6]; i = i << 8;
//     i |= bytes[7];
//     return i;
// }

unsigned int byteArrayTo32BitInteger(byte bytes[]){
    unsigned int i = 0;
    i |= bytes[0]; i = i << 8;
    i |= bytes[1]; i = i << 8;
    i |= bytes[2]; i = i << 8;
    i |= bytes[3];
    return i;
}

void setStateFrom32BitInteger(unsigned int i){
    state[0] = ((i & 0xFF000000) >> 24);
    state[1] = ((i & 0x00FF0000) >> 16);
    state[2] = ((i & 0x0000FF00) >> 8);
    state[3] = (i & 0x000000FF);
}