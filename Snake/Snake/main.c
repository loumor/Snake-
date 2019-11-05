//
// CAB202 - Assignment 2 - Snake
//
// Author: Morgan Frearson - N9510761
// Due Date: 23/10/2016
//

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "graphics.h"
#include "cpu_speed.h"
#include "sprite.h"


int lives = 3;
int score = 0;
int last_press = 0;
char *walls_message = "Off";
int length_of_snake = 3;
int last_last_press =0;
int wall_on_off = 0;


unsigned char food_bit[8] = {0b00000000, 0b00000000, 0b00000000, 0b00010000, 0b00111000, 0b00010000, 0b00000000, 0b00000000};

unsigned char bmsnake[3] = {0b00111000, 0b00111000, 0b00111000};


Sprite food;
Sprite snake[20];

void food_setup();
void initHardware(void);
void food_spawn();
void setup_Game();
void start_Game();
void drawBorder();
void draw_Banner(int lives, int score);
void update_score();
void spawn_snake();
void collisionDetection();
void draw_Centred(unsigned char y, char* string);
void walls_create();


int main() {
    initHardware();
    
    draw_Centred(15, "Morgan Frearson");
    draw_Centred(25, "n9510761");
    
    show_screen();
    _delay_ms(3000);
    clear_screen();
    
    food_setup();

    
    return 0;
}


void initHardware(void) {
    // Set the clock-speed.
    set_clock_speed(CPU_8MHz);
    
    // Initialize the LCD screen.
    lcd_init(LCD_DEFAULT_CONTRAST); //LCD_HIGH_CONTRAST fixes fading issue at front of snake
    
    // Initialising SW1 as input.
    DDRB &= ~((1 << PB1) | (1 << PB7));
    DDRD &= ~((1 << PD0) | (1 << PD1));
    
    //Potentiomete
    DDRF &= ~((1 << PF1));
    
    // Initialising SW2 and SW3 as inputs.
    DDRF &= ~((1 << PF5) | (1 << PF6));
    
    // Initialising LED0 and LED1 as outputs.
    DDRB |= ((1 << PB2) | (1 << PB3));
    
    // Initialising ADC with a pre-scaler of 128.
    ADMUX = (1<<REFS0);
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
    
    // Setup TIMER0 and add it to the interrupt register.
    TCCR0B |= ((1 << CS02) | (1 << CS00));
    TIMSK0 = (1<<TOIE0);
    
    // Enable interrupts globally.
    sei();
    
}

void food_setup() {
    
    setup_Game();
    start_Game();
}



void setup_Game() {
    lives = 3;
    score = 0;
    srand(0);
    
    init_sprite(&food, 0, 0, 8, 8, food_bit);
   
    for (int i = 0; i < length_of_snake; i++){
        init_sprite(&snake[i], 0, 0, 5, 3, bmsnake);
        snake[i].dx	= 0.7;
        snake[i].dy	= 0.7;
        food.is_visible	= 0;
        snake[i].is_visible	= 0;
    }
    
    food_spawn();
    spawn_snake();

}

void moving_snake_const(){
    
    
    if (last_press == 1){
        if (last_last_press == 2){
            lives--;
            for (int i = 0; i < length_of_snake; i++){
                snake[i].is_visible	= 0;
                
            }
            
        }
        last_last_press = 1;
        snake[0].x -= snake[0].dx;
        _delay_ms(10);
        
    }
    
    if (snake[0].x > 78) {
        snake[0].x = -2;
    }
  
    
    if (last_press == 2){
        if (last_last_press == 1){
            lives--;
            for (int i = 0; i < length_of_snake; i++){
                snake[i].is_visible	= 0;
                
            }
        }
        last_last_press = 2;
        snake[0].x += snake[0].dx;
        _delay_ms(10);
        
    }
    
    if (snake[0].x < -2) {
        snake[0].x = 78;
    }
   
    
    if (last_press == 3){
        if (last_last_press == 4){
            lives--;
            for (int i = 0; i < length_of_snake; i++){
                snake[i].is_visible	= 0;
                
            }
        }
        last_last_press = 3;
        snake[0].y -= snake[0].dy;
        _delay_ms(10);
        
    }
    
    if (snake[0].y > 44) {
        snake[0].y = 10;
    }
   
    
    if (last_press == 4){
        if (last_last_press == 3){
            lives--;
            for (int i = 0; i < length_of_snake; i++){
                snake[i].is_visible	= 0;

            }

        }
        last_last_press = 4;
        snake[0].y += snake[0].dy;
        _delay_ms(10);
        
    }
    
    if (snake[0].y < 10) {
        snake[0].y = 44;
    }
    
    
   
    
}

void moving_snake_body(){

    if(last_press != 0){ // if the snake is moving
        for (int i = length_of_snake; i > 0; i--) {  // Might need -1 exactly here //Determines which end of the snake the head will be
            //_delay_ms(20);
            snake[i].x = snake[i-1].x;//Take position in array and moves it accordingly
            snake[i].y = snake[i-1].y;//aka moving the snake
           // _delay_ms(10);
        }
    }
    
    

}

void draw_snake(){
    
    for (int i = 0; i < length_of_snake; i++){
        draw_sprite(&snake[i]);
    }
    
}

void start_Game() {
    while (1) {
        clear_screen();
        
        update_score();
        
        collisionDetection();
        food_spawn();
        spawn_snake();
        walls_create();
        
        draw_sprite(&food);
        draw_snake();

        

        drawBorder();
        draw_Banner(lives, score);
        
        moving_snake_const();
        moving_snake_body();
        
        show_screen();
        _delay_ms(50);
    }
}

void walls_create(){
    if (wall_on_off == 1){
        for (int i = 0; i<20; i++){
            set_pixel(18,10+i,1);
            set_pixel(65+i,30,1);
            set_pixel(50,50-i,1);
        }
        
    }
    
}


void drawBorder() {
    draw_line(0, 0, 83, 0);		// Top
    draw_line(83, 0, 83, 47);	// Right
    draw_line(83, 47, 0, 47);	// Bottom
    draw_line(0, 47, 0, 0);		// Left
}

void draw_Banner(int lives, int score) {
    
    char messa[20];
    sprintf(messa, "L:%d", lives);
    draw_string(3, 2, messa);
    
    draw_line(20, 0, 20, 10);
    
    sprintf(messa, "S:%02d", score);
    draw_string(23, 2, messa);
    
    draw_line(45, 0, 45, 10);
    
    sprintf(messa, "W:%s", walls_message);
    draw_string(52, 2, messa);
    
    draw_line(0, 10, 83, 10);
}

void spawn_snake() {
    for (int i = 0; i < length_of_snake; i++){

    if (!snake[i].is_visible) {
        last_last_press = 0;
        last_press = 0;
        snake[i].x = LCD_X / 2 - (i*3);
        snake[i].y = LCD_Y - 10;
        
        if ((abs(snake[i].x - food.x) < 5  && (abs(snake[i].y - food.y) < 5))) {
            spawn_snake();
        } else {
            snake[i].is_visible = 1;
        }
    }
    }
}

void food_spawn() {
    if (!food.is_visible) {
        food.x = (rand() % 81) + 2;
        food.y = (rand() % 30) + 12;
        
        
        if (wall_on_off == 1){
            if ( abs(food.x == 18) && abs(food.y >= 10 && food.y <= 30)) {
                food_spawn();
            }
            if ( abs(food.x == 50) && abs(food.y <= 50 && food.y >= 30)) {
                food_spawn();
            }
            if ( abs(food.y == 30) && abs(food.x >= 65 && food.x <= 85)) {
                food_spawn();
            }
        }

        if ( ((abs(food.x - snake[0].x)) < 8 && (abs(food.y - snake[0].y)) < 8)) {
            food_spawn();
        } else {
            food.is_visible = 1;
        }
        
    }
}



void collisionDetection() {
    
    if ((((snake[0].x<=food.x+3)&&(snake[0].x>=food.x))&&((snake[0].y<=food.y+3)&&(snake[0].y>=food.y)))||(((snake[0].x+3<=food.x+3)&&(snake[0].x+3>=food.x))&&((snake[0].y+3>=food.y)&&(snake[0].y+3<=food.y+3)))) {
    	score += 1;
        if (wall_on_off == 1){
            score += 1;
        }
        //length_of_snake++;
        
    	food.is_visible = 0;
    }
    
    if(wall_on_off==1){
       if((((snake[0].x>=16)&&(snake[0].x<=18))&&((snake[0].y>10)&&(snake[0].y<=32)))){
           lives--;

            for (int i = 0; i < length_of_snake; i++){
                snake[i].is_visible	= 0;
            }
            
        }
        
        if((((snake[0].x>=48)&&(snake[0].x<=50))&&((snake[0].y<50)&&(snake[0].y>=28)))){            for (int i = 0; i < length_of_snake; i++){
                lives--;
                snake[i].is_visible	= 0;
            }
            
        }
        
        
        
        if((((snake[0].y>=28)&&(snake[0].y<=30))&&((snake[0].x>65)&&(snake[0].x<=86)))) {
            for (int i = 0; i < length_of_snake; i++){
                lives--;
                snake[i].is_visible	= 0;
            }
            
        }

        
    }
    
}

void update_score() {
    if (lives <= 0) {
        draw_Centred(15, "Game Over!");
        draw_Centred(25, "Respawing Now");
        show_screen();
        _delay_ms(3000);
        clear_screen();
        length_of_snake = 3;
        last_press =0;
        wall_on_off = 0;
        walls_message = "Off";
        last_last_press = 0;
        food_setup();
    }
}

void draw_Centred(unsigned char y, char* string) {
    unsigned char l = 0, i = 0;
    while (string[i] != '\0') {
        l++;
        i++;
    }
    char x = 42-(l*5/2);
    draw_string((x > 0) ? x : 0, y, string);
}

//--------------------------------------------------
// Interrupt Service Routines (ISR)
//--------------------------------------------------
ISR(TIMER0_OVF_vect) {

    if (PINB >> 1 & 1){
        last_press = 1;
    }
    
    if (PIND >> 0 & 1){
        last_press = 2;

    }
    
    if (PIND >> 1 & 1){
        last_press = 3;

    }
    
    if (PINB >> 7 & 1){
        last_press = 4;

    }
    
    if (PINF >> 5 & 1){ //SW3
        wall_on_off = 1;
        walls_message = "On";
    }
    
    if (PINF >> 6 & 1){ //SW2
        wall_on_off = 0;
        walls_message = "Off";
    }
    
    if (PINF >> 1 & 1){ //Potenomete
        snake[0].dx = 0.3;
        snake[0].dy = 0.3;

    }
    else{
        snake[0].dx = 0.7;
        snake[0].dy = 0.7;
    }
    
    
    
    
}
