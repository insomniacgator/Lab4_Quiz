// threads.h
// Date Created: 2023-07-26
// Date Updated: 2023-07-26
// Threads

#ifndef THREADS_H_
#define THREADS_H_

/************************************Includes***************************************/

#include "./G8RTOS/G8RTOS.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/

#define SPAWNCOOR_FIFO      0
#define JOYSTICK_FIFO       1

/*************************************Defines***************************************/

/***********************************Semaphores**************************************/

semaphore_t sem_I2CA;
semaphore_t sem_SPIA;
semaphore_t sem_PCA9555_Debounce;
semaphore_t sem_Joystick_Debounce;
semaphore_t sem_KillCube;
semaphore_t sem_UART;

/***********************************Semaphores**************************************/

/***********************************Structures**************************************/
// Character
typedef struct character_t {
    uint16_t x_pos;
    uint16_t y_pos;
    uint8_t width;
    uint8_t length;
    uint16_t cx_pos;
    uint16_t cy_pos;

} character_t;

// Obstacle
typedef struct obstacle_t {
    uint16_t x_pos;
    uint16_t y_pos;
    uint8_t width;
    uint8_t length;
    uint16_t cx_pos;
    uint16_t cy_pos;

} obstacle_t;
/***********************************Structures**************************************/


/*******************************Background Threads**********************************/

void Idle_Thread(void);
void Cube_Thread(void);
void CamMove_Thread(void);
void Read_Buttons(void);
void Read_JoystickPress(void);

void DisplayUpdate_Thread(void);
void CharacterMove_Thread(void);

void LED_Thread(void);
/*******************************Background Threads**********************************/

/********************************Periodic Threads***********************************/

void Print_WorldCoords(void);
void Get_Joystick(void);

/********************************Periodic Threads***********************************/

/*******************************Aperiodic Threads***********************************/

void GPIOE_Handler(void);
void GPIOD_Handler(void);

/*******************************Aperiodic Threads***********************************/


#endif /* THREADS_H_ */

