// G8RTOS_Threads.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for thread functions.

/************************************Includes***************************************/

#include "./threads.h"

#include "./MultimodDrivers/multimod.h"
#include "./MiscFunctions/Shapes/inc/cube.h"
#include "./MiscFunctions/LinAlg/inc/linalg.h"
#include "./MiscFunctions/LinAlg/inc/quaternions.h"
#include "./MiscFunctions/LinAlg/inc/vect3d.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/************************************Includes***************************************/

/*************************************Defines***************************************/

// Change this to change the number of points that make up each line of a cube.
// Note that if you set this too high you will have a stack overflow!
// sizeof(float) * num_lines * (Num_Interpolated_Points + 2) = ?
#define Num_Interpolated_Points     3


/*************************************Defines***************************************/

/*********************************Global Variables**********************************/

Quat_t world_camera_pos = {0, 0, 0, 50};
Quat_t world_camera_frame_offset = {0, 0, 0, 50};
Quat_t world_camera_frame_rot_offset;
Quat_t world_view_rot = {1, 0, 0, 0};
Quat_t world_view_rot_inverse = {1, 0, 0, 0};

// How many cubes?
uint8_t num_cubes = 0;

// y-axis controls z or y
uint8_t joystick_y = 1;

uint8_t move_or_rot = 1;

// Kill a cube?
uint8_t kill_cube = 0;

/*********************************Global Variables**********************************/

/*************************************Threads***************************************/

void Idle_Thread(void) {
    time_t t;
    srand((unsigned) time(&t));
    while(1);
}



void CamMove_Thread(void) {
    // Initialize / declare any variables here
    int32_t joy_x, joy_y = 0;
    float joy_x_n, joy_y_n = 0;

    while(1) {
        // Get result from joystick
        joy_x = G8RTOS_ReadFIFO(JOYSTICK_FIFO);
        joy_y = G8RTOS_ReadFIFO(JOYSTICK_FIFO);

        // If joystick axis within deadzone, set to 0. Otherwise normalize it.
        if (joy_x < 1900 || joy_x > 2200)
        {
            joy_x_n = (2.0 * (joy_x - 0) / (4095 - 0)) - 1.0;
        }
        else
            joy_x_n = 0;
        if (joy_y < 1900 || joy_y > 2200)
                {
                    joy_y_n = (2.0 * (joy_y - 0) / (4095 - 0)) - 1.0;
                }
        else
            joy_y_n = 0;

        // Update world camera position. Update y/z coordinates depending on the joystick toggle.
        if (joystick_y)
        {
            world_camera_pos.x += -joy_y_n;
            world_camera_pos.y += joy_x_n;
        }
        else
        {
            world_camera_pos.x += -joy_y_n;
            world_camera_pos.z += joy_x_n;
        }


        // sleep
        sleep(10);
    }
}

void CharacterMove_Thread(void) {

    character_t dino;
    dino.x_pos = 10;
    dino.y_pos = 101;
    dino.length = 10;
    dino.width = 5;

    // Initialize / declare any variables here
    int32_t joy_x, joy_y = 0;
    float joy_x_n, joy_y_n = 0;
    bool jump = 0;

    while(1) {
        // Get result from joystick
        joy_x = G8RTOS_ReadFIFO(JOYSTICK_FIFO);
        joy_y = G8RTOS_ReadFIFO(JOYSTICK_FIFO);

        // If joystick axis within deadzone, set to 0. Otherwise normalize it.
        if (joy_x < 1900 || joy_x > 2200)
        {
            joy_x_n = (2.0 * (joy_x - 0) / (4095 - 0)) - 1.0;
        }
        else
            joy_x_n = 0;
        if (joy_y < 1900 || joy_y > 2200)
                {
                    joy_y_n = (2.0 * (joy_y - 0) / (4095 - 0)) - 1.0;
                    jump = 1;
                    UARTprintf("jump\n");
                }
        else
            joy_y_n = 0;

        // Update world camera position. Update y/z coordinates depending on the joystick toggle.
        if (joystick_y)
        {
            world_camera_pos.x += -joy_y_n;
            world_camera_pos.y += joy_x_n;
        }
        else
        {
            world_camera_pos.x += -joy_y_n;
            world_camera_pos.z += joy_x_n;
        }

        if (jump)
        {
            //dino.y_pos++;
        }

        G8RTOS_WaitSemaphore(&sem_SPIA);
        ST7789_DrawRectangle(dino.x_pos, dino.y_pos, dino.width, dino.length, ST7789_WHITE);
        G8RTOS_SignalSemaphore(&sem_SPIA);

        // sleep
        sleep(10);
    }
}

// We are goig
void DisplayUpdate_Thread(void) {
    cube_t cube;

    G8RTOS_WaitSemaphore(&sem_SPIA);
            ST7789_DrawLine(0, 100, 240, 100, ST7789_WHITE);
            G8RTOS_SignalSemaphore(&sem_SPIA);



    //UARTprintf("Cube Thread started\n");
    /*************YOUR CODE HERE*************/
    // Get spawn coordinates from FIFO, set cube.x, cube.y, cube.z
    cube.x_pos = G8RTOS_ReadFIFO(SPAWNCOOR_FIFO) - 200;
    cube.y_pos = G8RTOS_ReadFIFO(SPAWNCOOR_FIFO) - 200;
    cube.z_pos = G8RTOS_ReadFIFO(SPAWNCOOR_FIFO) - 200;

    cube.width = 50;
    cube.height = 50;
    cube.length = 50;

    Quat_t v[8];
    Quat_t v_relative[8];

    Cube_Generate(v, &cube);

    uint32_t m = Num_Interpolated_Points + 1;
    Vect3D_t interpolated_points[12][Num_Interpolated_Points + 2];
    Vect3D_t projected_point;

    Quat_t camera_pos;
    Quat_t camera_frame_offset;
    Quat_t view_rot_inverse;

    uint8_t kill = 0;

    while(1) {
        /*************YOUR CODE HERE*************/
        // Check if kill ball flag is set.
        if (kill_cube)
        {
            G8RTOS_WaitSemaphore(&sem_KillCube);
            kill = kill_cube;
            kill_cube = 0;
            G8RTOS_SignalSemaphore(&sem_KillCube);
        }

        camera_pos.x = world_camera_pos.x;
        camera_pos.y = world_camera_pos.y;
        camera_pos.z = world_camera_pos.z;

        camera_frame_offset.x = world_camera_frame_offset.x;
        camera_frame_offset.y = world_camera_frame_offset.y;
        camera_frame_offset.z = world_camera_frame_offset.z;

        view_rot_inverse.w = world_view_rot_inverse.w;
        view_rot_inverse.x = world_view_rot_inverse.x;
        view_rot_inverse.y = world_view_rot_inverse.y;
        view_rot_inverse.z = world_view_rot_inverse.z;

        // Clears cube from screen
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < m+1; j++) {
                getViewOnScreen(&projected_point, &camera_frame_offset, &(interpolated_points[i][j]));
                /*************YOUR CODE HERE*************/
                // Wait on SPI bus
                G8RTOS_WaitSemaphore(&sem_SPIA);

                ST7789_DrawPixel(projected_point.x, projected_point.y, ST7789_BLACK);

                /*************YOUR CODE HERE*************/
                // Signal that SPI bus is available
                G8RTOS_SignalSemaphore(&sem_SPIA);
            }
        }

        /*************YOUR CODE HERE*************/
        // If ball marked for termination, kill the thread.
        if (kill)
        {
            G8RTOS_WaitSemaphore(&sem_KillCube);
            kill = 0;
            G8RTOS_KillSelf();
            ST7789_Fill(0);
            G8RTOS_SignalSemaphore(&sem_KillCube);
            sleep(100);
        }

        // Calculates view relative to camera position / orientation
        for (int i = 0; i < 8; i++) {
            getViewRelative(&(v_relative[i]), &camera_pos, &(v[i]), &view_rot_inverse);
        }

        // Interpolates points between vertices
        interpolatePoints(interpolated_points[0], &v_relative[0], &v_relative[1], m);
        interpolatePoints(interpolated_points[1], &v_relative[1], &v_relative[2], m);
        interpolatePoints(interpolated_points[2], &v_relative[2], &v_relative[3], m);
        interpolatePoints(interpolated_points[3], &v_relative[3], &v_relative[0], m);
        interpolatePoints(interpolated_points[4], &v_relative[0], &v_relative[4], m);
        interpolatePoints(interpolated_points[5], &v_relative[1], &v_relative[5], m);
        interpolatePoints(interpolated_points[6], &v_relative[2], &v_relative[6], m);
        interpolatePoints(interpolated_points[7], &v_relative[3], &v_relative[7], m);
        interpolatePoints(interpolated_points[8], &v_relative[4], &v_relative[5], m);
        interpolatePoints(interpolated_points[9], &v_relative[5], &v_relative[6], m);
        interpolatePoints(interpolated_points[10], &v_relative[6], &v_relative[7], m);
        interpolatePoints(interpolated_points[11], &v_relative[7], &v_relative[4], m);

        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < m+1; j++) {
                getViewOnScreen(&projected_point, &camera_frame_offset, &(interpolated_points[i][j]));

                if (interpolated_points[i][j].z < 0) {
                    /*************YOUR CODE HERE*************/
                    // Wait on SPI bus
                    G8RTOS_WaitSemaphore(&sem_SPIA);
                    ST7789_DrawPixel(projected_point.x, projected_point.y, ST7789_BLUE);

                    /*************YOUR CODE HERE*************/
                    // Signal that SPI bus is available
                    G8RTOS_SignalSemaphore(&sem_SPIA);
                }
            }
        }

        /*************YOUR CODE HERE*************/
        // Sleep
        sleep(10);
    }
}

void Cube_Thread(void) {
    cube_t cube;

    // Get spawn position
    uint32_t packet = G8RTOS_ReadFIFO(SPAWNCOOR_FIFO);

    cube.x_pos = (packet >> 20 & 0xFFF) - 100;
    cube.y_pos = (packet >> 8 & 0xFFF) - 100;
    cube.z_pos = -(packet & 0xFF) - 50;

    cube.width = 50;
    cube.height = 50;
    cube.length = 50;

    Quat_t v[8];
    Quat_t v_relative[8];

    Cube_Generate(v, &cube);

    // Declare a 2d array to store interpolated points
    // This is faster and more robust at the cost of vastly increased space.
    uint32_t m = Num_Interpolated_Points + 1;
    Vect3D_t interpolated_points[12][Num_Interpolated_Points + 2];
    Vect3D_t projected_point;

    Quat_t camera_pos;
    Quat_t camera_frame_offset;

    Quat_t view_rot_inverse;

    uint8_t kill = 0;

    while(1) {

        G8RTOS_WaitSemaphore(&sem_KillCube);
        if (kill_cube) {
            kill = 1;
            kill_cube = 0;
        }
        G8RTOS_SignalSemaphore(&sem_KillCube);

        // set so that the positions are static during viewpoint calculations
        camera_pos.x = world_camera_pos.x;
        camera_pos.y = world_camera_pos.y;
        camera_pos.z = world_camera_pos.z;

        camera_frame_offset.x = world_camera_frame_offset.x;
        camera_frame_offset.y = world_camera_frame_offset.y;
        camera_frame_offset.z = world_camera_frame_offset.z;

        view_rot_inverse.w = world_view_rot_inverse.w;
        view_rot_inverse.x = world_view_rot_inverse.x;
        view_rot_inverse.y = world_view_rot_inverse.y;
        view_rot_inverse.z = world_view_rot_inverse.z;

        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < m+1; j++) {
                getViewOnScreen(&projected_point, &camera_frame_offset, &(interpolated_points[i][j]));
                G8RTOS_WaitSemaphore(&sem_SPIA);
                ST7789_DrawPixel(projected_point.x, projected_point.y, ST7789_BLACK);
                G8RTOS_SignalSemaphore(&sem_SPIA);
            }
        }

        // If kill is set, killself after clearing the cube from the screen.
        if (kill) {
            num_cubes--;
            G8RTOS_KillSelf();
        }

        // Get relative view points (for perspective calculations)
        for (int i = 0; i < 8; i++) {
            getViewRelative(&(v_relative[i]), &camera_pos, &(v[i]), &view_rot_inverse);
        }

        // Interpolate all pixels between vertices
        interpolatePoints(interpolated_points[0], &v_relative[0], &v_relative[1], m);
        interpolatePoints(interpolated_points[1], &v_relative[1], &v_relative[2], m);
        interpolatePoints(interpolated_points[2], &v_relative[2], &v_relative[3], m);
        interpolatePoints(interpolated_points[3], &v_relative[3], &v_relative[0], m);
        interpolatePoints(interpolated_points[4], &v_relative[0], &v_relative[4], m);
        interpolatePoints(interpolated_points[5], &v_relative[1], &v_relative[5], m);
        interpolatePoints(interpolated_points[6], &v_relative[2], &v_relative[6], m);
        interpolatePoints(interpolated_points[7], &v_relative[3], &v_relative[7], m);
        interpolatePoints(interpolated_points[8], &v_relative[4], &v_relative[5], m);
        interpolatePoints(interpolated_points[9], &v_relative[5], &v_relative[6], m);
        interpolatePoints(interpolated_points[10], &v_relative[6], &v_relative[7], m);
        interpolatePoints(interpolated_points[11], &v_relative[7], &v_relative[4], m);

        // Draw all points by projecting them to the screen.
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < m+1; j++) {
                getViewOnScreen(&projected_point, &camera_frame_offset, &(interpolated_points[i][j]));

                if (interpolated_points[i][j].z < 0) {
                    G8RTOS_WaitSemaphore(&sem_SPIA);
                    ST7789_DrawPixel(projected_point.x, projected_point.y, ST7789_BLUE);
                    G8RTOS_SignalSemaphore(&sem_SPIA);
                }
            }
        }

        sleep(20);
    }
}

void Read_Buttons() {
    uint8_t buttons;

    while(1) {
        G8RTOS_WaitSemaphore(&sem_PCA9555_Debounce);

        // For switch debouncing
        sleep(15);

        // Get buttons
        G8RTOS_WaitSemaphore(&sem_I2CA);
        buttons = ~(MultimodButtons_Get());
        G8RTOS_SignalSemaphore(&sem_I2CA);

        if (buttons & SW1) {
            // Spawn cube
            int8_t result = G8RTOS_AddThread(Cube_Thread, 254, "cube\0");

            if (result == NO_ERROR) {
                num_cubes++;

                // Get random positions
                uint32_t x_pos, y_pos, z_pos;

                x_pos = (rand() % 200);
                y_pos = (rand() % 200);
                z_pos = (rand() % 100);

                // Write to FIFO
                uint32_t packet = ((x_pos & 0xFFF) << 20) | ((y_pos & 0xFFF) << 8) | ((z_pos & 0xFF) << 0);
                G8RTOS_WriteFIFO(SPAWNCOOR_FIFO, packet);
            }
        }

        if (buttons & SW2) {
            if (num_cubes > 0) {
                kill_cube = 1;
            }
        }

        if (buttons & SW3) {
            if (move_or_rot) {
                move_or_rot = 0;
            } else {
                move_or_rot = 1;
            }
        }

        GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
        GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);

    }
}

void Read_JoystickPress() {
    uint8_t joystick_s;

    while(1) {
        G8RTOS_WaitSemaphore(&sem_Joystick_Debounce);

        // For switch debouncing
        sleep(5);

        joystick_s = JOYSTICK_GetPress();

        if (joystick_s) {
            if (joystick_y) {
                joystick_y = 0;
            } else {
                joystick_y = 1;
            }
        }

        GPIOIntClear(JOYSTICK_INT_GPIO_BASE, JOYSTICK_INT_PIN);
        GPIOIntEnable(JOYSTICK_INT_GPIO_BASE, JOYSTICK_INT_PIN);

    }
}

void LED_Thread(void) {
    uint8_t i = 2;
    while(1) {
        PCA9956b_SetAllOff();
        PCA9556b_SetLED(i, 128, 128);
        i++;
        if (i == 22) {
            i = 2;
        }

        sleep(100);
    }
}


/********************************Periodic Threads***********************************/

void Print_WorldCoords(void) {
    UARTprintf("Cam Pos, X: %d, Y: %d, Z: %d\n", (int32_t) world_camera_pos.x, (int32_t)world_camera_pos.y, (int32_t)world_camera_pos.z);
}

void Get_Joystick(void) {
    uint32_t packet;
    packet = JOYSTICK_GetXY();
    G8RTOS_WriteFIFO(JOYSTICK_FIFO, packet);
}

/********************************Periodic Threads***********************************/


/*******************************Aperiodic Threads***********************************/

void GPIOE_Handler() {
    GPIOIntDisable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
    G8RTOS_SignalSemaphore(&sem_PCA9555_Debounce);
}

void GPIOD_Handler() {
    GPIOIntDisable(JOYSTICK_INT_GPIO_BASE, JOYSTICK_INT_PIN);
    G8RTOS_SignalSemaphore(&sem_Joystick_Debounce);
}

/*******************************Aperiodic Threads***********************************/
