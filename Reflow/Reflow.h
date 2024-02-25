#ifndef __REFLOW_H__
#define __REFLOW_H__
#include "stm32f3xx_hal.h"

#define X_MIN	0
#define X_MAX 159
#define X_118 118
#define Y_MIN	0
#define Y_MAX 127

#define BTN_OFFSET 10
#define BTN_HEIGHT 20
#define BTN_WIDTH_SMALL 40
#define BTN_SPACING 50
#define BTN_WIDTH_LARGE 55

#define COORD_28 28

#define PID_KP 0.00400000019f    /* Proportional = The Temperature to target */
#define PID_KI 0.00800000038f    /* Integral */
#define PID_KD 0.000500000024f    /* Derivative = Speed of temperature change */
#define PID_INT_MAX 1000.0f  /* Maximum value for integral term */
#define PID_INT_MIN -1000.0f /* Minimum value for integral term */
#define CONTROL_EFFORT_MAX 1000.0f  /* Maximum value for control effort */
#define CONTROL_EFFORT_MIN -1000.0f /* Minimum value for control effort */

#define TO_MS				1000.0f
#define TEMP_SCALE	100.0f
#define UPDATE_FREQ	10.0f

typedef struct {
    float Kp;         // Proportional gain
    float Ki;         // Integral gain
    float Kd;         // Derivative gain
    float prevError;  // Previous error
    float integral;   // Integral term
} PIDController;

typedef struct {
  float Temperature; // Temperature for a specific point
  uint32_t Time; // Time for a specific point
	float m; // Line slope
	float B; // Line equation parameter y = m*x + b
} profilePoint;

// Points order on the reflow graph
enum {
	startPoint = 0,
	startSoak = 1, //End of ramp to soak
	startRamp = 2, //End of soak
	startPeak = 3, //End of ramp to peak
	startCool = 4, //End of peak
	endCool = 5 //End of cooling
};

// Encoder key states
enum {
	key_none = 0, // Noting to do
	key_down = 1, // Counter is going down
	key_up = 2, // Counter is going up
	key_press = 3 // The button is pressed
};

// Array position to hold the values from user
enum {
	rampToSoak		= 0, // 1..3C/s
	tempSoak			= 1, // Preheat/Soaking temperature 150C+-20C for 60s..120s
	durationSoak	= 2, //60s..120s
	rampToPeak		= 3, // 1..3C/s
	tempPeak			= 4, // Reflow temperature max. 255C
	durationPeak	= 5, //45s..75s
	rampToCool		= 6 // 1..4C/s
};

// Work mode for the assembly
enum {
	modeRework	= 0,
	modeReflow	= 1
};
extern float autotuneSetpoint;
void makeSound();
void TemperatureControl(float targetTemp);
void getKeys(void);
void Navigation(void);
void changeValue(void);
void drawMenu(uint8_t menuIdx);
void Menu_Main(void);
void coolMode(void);
void workMode(uint8_t mode);
void drawGraph(uint32_t time);

void autotunePID(void);

#endif // __REFLOW_H__
