#include "Reflow.h"
#include "gpio.h"
#include "adc.h"
#include "tim.h"
//#include "GFX_FUNCTIONS.h"
#include "ST7735.h"
#include <stdio.h>
#include <string.h>

// Temp            /----\
// T2|            /|    |\
//   |           / |    | \
//   |          /  |    |  \
//   |	 /-----/   |    |   \
// T1|  /|     |   |    |    \
//   | / |     |   |    |     \
//   |/  |     |   |    |      \
// T0|----------------------------time
//  t0	  t1    t2  t3   t4     t5

// Global variables for PID controller and temperature
extern volatile float crtTemp;
extern volatile uint8_t BtnPressed;

extern TIM_HandleTypeDef htim2;
// LCD menu size

// Text to be displayed in main menu items
const char * const txt[] = {"C/s", "C", "s", "C/s", "C", "s", "C/s", "Rework", "Reflow", "Cool"};
// Text color, yellow is the current menu
uint16_t colorTxt[] = {GREEN, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE}; 
// Defaut values for main menu parameters
uint8_t menuVals[] = {3, 150, 60, 3, 255, 45, 4};


// Encoder values 
uint16_t encVal, tmp_enc;
// Variables to retain the navigation parameters
uint8_t prevMenu, crtMenu, crtKey;

// Scaling factors for temperature and time considering screen size
float scalingX, scalingY;
// Period to update the screen
uint16_t pixelUpdatePeriod;
// When to update the screen
uint32_t updateTime, pidPrev, pidCrt;
// Array to hold the reflow profile points
profilePoint myPoints[7];
PIDController pid = {PID_KP, PID_KI, PID_KD, 0.0f, 0.0f};

#ifdef AUTOTUNE
	// Constants for autotuning
	#define AUTOTUNE_DURATION 60000  // Autotune duration in milliseconds
	#define AUTOTUNE_STEP 1         // Step size for parameter adjustment
	// Autotune state enumeration
	typedef enum {
			AUTOTUNE_IDLE,
			AUTOTUNE_HEATING,
			AUTOTUNE_COOLING,
			AUTOTUNE_COMPLETE
	} AutotuneState;

	// Autotune parameters
	// Autotune parameters
	float autotuneSetpoint = 50.0;  // Setpoint for autotune
	AutotuneState autotuneState = AUTOTUNE_IDLE;
	float autotuneStartTime = 0;
#endif
// Turn off heater and fan, return to Main menu
static void endProcess(){
	HEAT_OFF;
	FAN_OFF;
	stopADC();
	Menu_Main();
}

// Calculates times, temperatures and scaling coefficients
static void computeData(){
	// Clear screen
	fillScreen(BLACK);
	// Start ADC
	startADC();
	// Reset global variables
	scalingX = 0.0;
	scalingY = 0.0;
	for(uint8_t cnt = 0; cnt < 6; cnt++){
		myPoints[cnt].Time = 0;
		myPoints[cnt].Temperature = 0;
		myPoints[cnt].m = 0.0;
		myPoints[cnt].B = 0.0;
	}
	// Calculate Temperatures
	myPoints[startPoint].Temperature = myPoints[endCool].Temperature = crtTemp;
	myPoints[startSoak].Temperature = myPoints[startRamp].Temperature = (float)menuVals[tempSoak];
	myPoints[startPeak].Temperature = myPoints[startCool].Temperature = (float)menuVals[tempPeak];
	
	// Calculate Times
	// Get temperature interval. crtTemp will be at the base of the graph y=127 (0 to 27 is reserved)
	myPoints[startPoint].Time = HAL_GetTick();
	// When we need to get to soak temperature (T soak - Tstart)/soak gradient
	myPoints[startSoak].Time = myPoints[startPoint].Time + TO_MS * ((myPoints[startSoak].Temperature - myPoints[startPoint].Temperature)/(float)menuVals[rampToSoak]);
	// When to start the ramp to peak
	myPoints[startRamp].Time = myPoints[startSoak].Time + TO_MS * menuVals[durationSoak];
	// When to start the peak (T peak - T soak)/ peak gradient
	myPoints[startPeak].Time = myPoints[startRamp].Time + TO_MS * ((myPoints[startPeak].Temperature - myPoints[startSoak].Temperature)/(float)menuVals[rampToPeak]);
	// When to start the cooling
	myPoints[startCool].Time = myPoints[startPeak].Time + TO_MS * menuVals[durationPeak];
	// When to end the cooling (T peak - T start)/ cooling gradient
	myPoints[endCool].Time = myPoints[startCool].Time + TO_MS * ((myPoints[startCool].Temperature - myPoints[startPoint].Temperature)/(float)menuVals[rampToCool]);
	
	// Slope m = rise/run = (TempEnd-TempStart)/(TimeEnd-TimeStart)
	// Equation Y = m*X + B; Y is the temperature and X is the time
	
	// Slope for ramp to soak
	myPoints[startPoint].m = (myPoints[startSoak].Temperature - myPoints[startPoint].Temperature) / (myPoints[startSoak].Time - myPoints[startPoint].Time);
	// To get B we need to calculate Y1 = m*X1 + B => B = Y1 - m*X1
	myPoints[startPoint].B = myPoints[startPoint].Temperature - myPoints[startPoint].m * myPoints[startPoint].Time;
	// The soaking is a horizontal line
	myPoints[startSoak].m = 0.0;
	myPoints[startSoak].B = myPoints[startSoak].Temperature;
	// Slope for ramp to peak
	myPoints[startRamp].m = (myPoints[startPeak].Temperature - myPoints[startRamp].Temperature) / (myPoints[startPeak].Time - myPoints[startRamp].Time);
	myPoints[startRamp].B = myPoints[startRamp].Temperature - myPoints[startRamp].m * myPoints[startRamp].Time;
	// The peak is a horizontal line
	myPoints[startPeak].m = 0.0;
	myPoints[startPeak].B = myPoints[startPeak].Temperature;
	// Slope for cooling
	myPoints[startCool].m = (myPoints[endCool].Temperature - myPoints[startCool].Temperature) / (myPoints[endCool].Time - myPoints[startCool].Time);
	myPoints[startCool].B = myPoints[startCool].Temperature - myPoints[startCool].m * myPoints[endCool].Time;
	
	//scalingY = (myPoints[startPeak].Temperature - myPoints[startPoint].Temperature) / TEMP_SCALE; //Degrees C per pixel
	if(crtMenu == 7){
		// For Rework
		pixelUpdatePeriod = myPoints[startRamp].Time / (float)X_MAX;
		scalingX = (float)X_MAX / myPoints[startRamp].Time; // pixel per ms
		scalingY = TEMP_SCALE / (myPoints[startRamp].Temperature - myPoints[startPoint].Temperature); //Pixel per Degree C
	} else {
		// For Reflow
		pixelUpdatePeriod = myPoints[endCool].Time / (float)X_MAX;
		scalingX = (float)X_MAX / myPoints[endCool].Time; // pixel per ms
		scalingY = TEMP_SCALE / ( myPoints[startCool].Temperature -  myPoints[endCool].Temperature); //Pixel per Degree C
	}
}

	//PWM Buzzer
	//htim1.Instance->CCR1 = 250;//Duty Cycle%*20
void makeSound(void){
	//isSoundOn = 1;
	HAL_TIM_Base_Start_IT(&htim4);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
}


// Function to calculate PID control effort
float calculatePID(float error) {
	// The terms are set depending on temperature
	if(crtTemp < 50.0){
		pid.Kp = 0.0120000001;
		pid.Ki = 0.0240000002;
		pid.Kd = 0.00150000001;
	} else if(crtTemp < 100.0){
		pid.Kp = 0.00600000005;
		pid.Ki = 0.0120000001;
		pid.Kd = 0.000750000007;
	} else if(crtTemp < 150.0){
		pid.Kp = 0.00400000019;
		pid.Ki = 0.00800000038;
		pid.Kd = 0.000500000024;
	} else if(crtTemp < 200.0){
		pid.Kp = 0.00300000003;
		pid.Ki = 0.00600000005;
		pid.Kd = 0.000375000003;
	} else {
		pid.Kp = 0.00266666664;
		pid.Ki = 0.00533333328;
		pid.Kd = 0.00033333333;
	}
	// Calculate PID terms
	float proportional = pid.Kp * error;
	pid.integral += pid.Ki * error;
	// Anti-windup for integral term
	if (pid.integral > PID_INT_MAX) {
		pid.integral = PID_INT_MAX;
	} else if (pid.integral < PID_INT_MIN) {
		pid.integral = PID_INT_MIN;
	}
	float derivative = pid.Kd * (error - pid.prevError);
	pid.prevError = error;
	// Calculate and return PID control effort
	float controlEffort = proportional + pid.integral + derivative;
	// Saturate the control effort
	if (controlEffort > CONTROL_EFFORT_MAX) {
		controlEffort = CONTROL_EFFORT_MAX;
	} else if (controlEffort < CONTROL_EFFORT_MIN) {
		controlEffort = CONTROL_EFFORT_MIN;
	}
	return controlEffort;
}

// Update heating element based on current temperature and target temperature
// Update heating element and cooling fan based on current temperature and target temperature
void TemperatureControl(float targetTemperature) {
	float error = targetTemperature - crtTemp;
	// Calculate PID output
	float controlEffort = calculatePID(error);
	// Control the heating element and cooling fan based on PID output
	if (controlEffort > 0) {
		HEAT_ON;  // Turn on the heating element
		FAN_OFF;  // Turn off the cooling fan
	} else if (controlEffort < 0) {
		HEAT_OFF;  // Turn off the heating element
		FAN_ON;   // Turn on the cooling fan
	} else {
		HEAT_OFF;  // Maintain current state for heating element
		FAN_OFF;   // Maintain current state for cooling fan
	}
}
#ifdef AUTOTUNE
	// Function to perform PID autotuning
	void autotunePID() {
		float error = autotuneSetpoint - crtTemp;
		float controlEffort = calculatePID(error);
		switch (autotuneState) {
			case AUTOTUNE_IDLE:
				// Start autotune process
				autotuneState = AUTOTUNE_HEATING;
				autotuneStartTime = HAL_GetTick();  // Record start time
				break;
			case AUTOTUNE_HEATING:
				// Check if autotune duration has passed
				if (HAL_GetTick() - autotuneStartTime >= AUTOTUNE_DURATION) {
					// Stop heating and move to cooling
					HEAT_OFF;
					autotuneState = AUTOTUNE_COOLING;
					autotuneStartTime = HAL_GetTick();  // Record start time for cooling
				} else {
					// Continue heating
					HEAT_ON;
				}
				break;
			case AUTOTUNE_COOLING:
				// Check if autotune duration has passed
				if (HAL_GetTick() - autotuneStartTime >= AUTOTUNE_DURATION) {
					// Autotune complete, calculate PID parameters
					HEAT_OFF;
					autotuneState = AUTOTUNE_COMPLETE;
					// Calculate PID parameters based on Ziegler-Nichols method
					pid.Kp = 0.6 / autotuneSetpoint;
					pid.Ki = 2 * pid.Kp;
					pid.Kd = pid.Kp / 8;
				} else {
					// Continue cooling
					HEAT_OFF;
					FAN_ON;
				}
				break;
			case AUTOTUNE_COMPLETE:
				// Use the calculated PID parameters for temperature control
				TemperatureControl(autotuneSetpoint);
				break;
		}
	}
#endif
// Check the encoder keys
void getKeys(void){
	if(BtnPressed) {
		BtnPressed = 0;
		crtKey = key_press;
		makeSound();
		//printf("Button was pressed\r\n");
	}
	tmp_enc = ((htim2.Instance->CNT)>>2);
	if(tmp_enc != encVal){
		if(tmp_enc > encVal) {
			crtKey = key_down;
		} else {
			crtKey = key_up;
		}
		encVal = tmp_enc;
		makeSound();
	}
}

// Handles Main Manu navigation
void Navigation(void){
	// Check if we have user input at encoder
	getKeys();
	// If there is input
	if(crtKey) {
		prevMenu = crtMenu;
		switch(crtKey) {
			case key_up:
				crtMenu++;
				if(crtMenu > 9) crtMenu = 0;
			break;
			case key_down:
				if(crtMenu == 0){
					crtMenu = 9;
				} else {
					crtMenu--;
				}
			break;
			case key_press:
				crtKey = key_none;
				if(crtMenu < 7){
				changeValue();
				} else {
					if(crtMenu == 7){
						//Show rework
						crtKey = key_none;
						workMode(modeRework);
					} else if(crtMenu == 8) {
						//Show reflow
						crtKey = key_none;
						workMode(modeReflow);
					} else {
						//Show reflow
						crtKey = key_none;
						coolMode();
					}
				}
			break;		
		}
		colorTxt[prevMenu] = WHITE;
		drawMenu(prevMenu);
		colorTxt[crtMenu] = GREEN;
		drawMenu(crtMenu);
		crtKey = key_none;
	}	
}

// Change Item Value
void changeValue(void){
	ST7735_InvertColors(true);
	while(crtKey != key_press){
		uint8_t prevVal = menuVals[crtMenu];
		switch(crtKey) {
			case key_up:
				menuVals[crtMenu]++;
			break;
			case key_down:
				menuVals[crtMenu]--;
			break;
		}
		if(prevVal != menuVals[crtMenu])
			drawMenu(crtMenu);
		crtKey = key_none;
		getKeys();
	}
	ST7735_InvertColors(false);
}

//Draw Main Menu Items
void drawMenu(uint8_t menuIdx){
	uint8_t x = 0;
	uint8_t y = 0;
	char str[9];
	if(menuIdx < 3) {
		x = BTN_OFFSET + (menuIdx * BTN_SPACING);
		y = 14;
	} else if(menuIdx < 6) {
		x = BTN_OFFSET + ((menuIdx - 3) * BTN_SPACING);
		y = 52;
	} else if(menuIdx < 7) {
		x = BTN_OFFSET;
		y = 74;
	} else {
		x = 1 + ((menuIdx - 7) * 57);
		y = 100;
	}
	
	if(menuIdx < 7) {
		sprintf(str, "%d", menuVals[menuIdx]);
		strcat(str, txt[menuIdx]);
		drawButton(x, y, BTN_WIDTH_SMALL, BTN_HEIGHT, RED, colorTxt[menuIdx], str);
	} else if(menuIdx < 9){
		drawButton(x, y, BTN_WIDTH_LARGE, BTN_HEIGHT, RED, colorTxt[menuIdx], txt[menuIdx]);
	} else {
		drawButton(x, y, BTN_WIDTH_SMALL, BTN_HEIGHT, RED, colorTxt[menuIdx], txt[menuIdx]);
	}
}

//Draw Main Menu
void Menu_Main(void){
	fillScreen(BLACK);
	ST7735_WriteString(BTN_OFFSET, 0, "Soaking", Font_7x10, YELLOW, BLACK);
	// Item 0
	drawMenu(0);
	// Item 1
	drawMenu(1);
	// Item 2
	drawMenu(2);
	ST7735_WriteString(BTN_OFFSET, 38, "Reflow", Font_7x10, YELLOW, BLACK);
	// Item 3
	drawMenu(3);
	// Item 4
	drawMenu(4);
	// Item 5
	drawMenu(5);
	// Item 6
	drawMenu(6);
	// Item 7
	drawMenu(7);
	// Item 8
	drawMenu(8);
	// Item 9
	drawMenu(9);
}
// Cooling mode
void coolMode(void){
	char tmp[15];
	// Clear screen
	fillScreen(BLACK);
	// Start ADC
	startADC();
	ST7735_WriteString(5, 12, "Temp:", Font_7x10, GREEN, BLACK);
	while(crtTemp > 10){
		sprintf(tmp, "%.2f", crtTemp);
		ST7735_WriteString(40, 12, tmp, Font_7x10, GREEN, BLACK);
		getKeys();
		if(crtKey == key_press){
			break;
		} else {
				FAN_ON;
				HAL_Delay(20);
		}
	}
	endProcess();
}

//The ReWork/Reflow menu
void workMode(uint8_t mode) {
	uint8_t X, X1, Y;
	char tmp[15];
	float targetTemp;
	uint32_t crtTime;
	// Temp            /----\
	// T2|            /|    |\
	//   |           / |    | \
	//   |          /  |    |  \
	//   |	 /-----/   |    |   \
	// T1|  /|     |   |    |    \
	//   | / |     |   |    |     \
	//   |/  |     |   |    |      \
	// T0|----------------------------time
	//  t0	  t1    t2  t3   t4     t5   

	// Calculate times, temperatures and scaling factors
	computeData();
	// Display parameters
	ST7735_WriteString(X_MIN, Y_MIN, "SO:", Font_7x10, GREEN, BLACK);
	sprintf(tmp, "%d", menuVals[tempSoak]);
	strcat(tmp, txt[tempSoak]);
	ST7735_WriteString(22, Y_MIN, tmp, Font_7x10, GREEN, BLACK);

	if(mode == modeRework){
		ST7735_WriteString(X_MIN, 12, "GS:", Font_7x10, GREEN, BLACK);
		sprintf(tmp, "%d", menuVals[rampToSoak]);
		strcat(tmp, txt[rampToSoak]);
		ST7735_WriteString(22, 12, tmp, Font_7x10, GREEN, BLACK);
	} else {
		ST7735_WriteString(57, Y_MIN, "GS:", Font_7x10, GREEN, BLACK);
		X = 78;
		Y = Y_MIN;

		ST7735_WriteString(X_MIN, 12, "PE:", Font_7x10, GREEN, BLACK);
		sprintf(tmp, "%d", menuVals[tempPeak]);
		strcat(tmp, txt[tempPeak]);
		ST7735_WriteString(21, 12, tmp, Font_7x10, GREEN, BLACK);
		
		ST7735_WriteString(56, 12, "GP:", Font_7x10, GREEN, BLACK);
		sprintf(tmp, "%d", menuVals[rampToSoak]);
		strcat(tmp, txt[rampToPeak]);
		ST7735_WriteString(77, 12, tmp, Font_7x10, GREEN, BLACK);

		ST7735_WriteString(X_MIN, 24, "GC:", Font_7x10, GREEN, BLACK);
		sprintf(tmp, "%d", menuVals[rampToCool]);
		strcat(tmp, txt[rampToCool]);
		ST7735_WriteString(21, 24, tmp, Font_7x10, GREEN, BLACK);
	}
	//
	X = myPoints[startSoak].Time * scalingX;
	if(mode == modeRework){
		// Draw ramp to soak line
		drawLine(X_MIN, Y_MAX, X, COORD_28, WHITE);
		// Draw soak line
		drawLine(X, COORD_28, X_MAX, COORD_28, WHITE);
	} else {
		Y = (myPoints[startSoak].Temperature - myPoints[startPoint].Temperature) * scalingY;
		// Draw ramp to soak line
		drawLine(X_MIN, Y_MAX, X, Y, WHITE);
		// Draw soak line
		X1 = myPoints[startRamp].Time * scalingX;
		drawLine(X, Y, X1, Y, WHITE);
		//draw ramp to peak line
		X = myPoints[startPeak].Time * scalingX;
		drawLine(X1, Y, X, COORD_28, WHITE);
		//draw peak line
		X1 = myPoints[startCool].Time * scalingX;
		drawLine(X, COORD_28, X1, COORD_28, WHITE);
		//draw cool ramp line
		drawLine(X1, COORD_28, X_MAX, Y_MAX, WHITE);
	}


	targetTemp = myPoints[startPoint].Temperature;
	TemperatureControl(targetTemp);
	X = 0;
	while(crtKey != key_press){
		// Force exit loop
		crtTime = HAL_GetTick();
		if (mode == modeRework && crtTime > myPoints[startRamp].Time) break;  // Rework
        if (mode == modeReflow && crtTime > myPoints[endCool].Time) break;  // Reflow
		getKeys();
		
		// We are in ramp to soak phase
		if(crtTime < myPoints[startSoak].Time){
			targetTemp = myPoints[startPoint].m * crtTime + myPoints[startPoint].B;
		// We are in soak phase	
		} else if(crtTime < myPoints[startRamp].Time){
			targetTemp = myPoints[startSoak].Temperature;
		// We are in ramp to peak phase
		} else if(crtTime < myPoints[startPeak].Time){
			targetTemp = myPoints[startRamp].m * crtTime + myPoints[startRamp].B;
		// We are in peak phase
		} else if(crtTime < myPoints[startCool].Time){
			targetTemp = myPoints[startPeak].Temperature;
		// We are in cool down phase	
		}	else {
			targetTemp = myPoints[startCool].m * crtTime + myPoints[startCool].B;
		}
		// Update PID with new temperature target
		TemperatureControl(targetTemp);
		if(crtTime >= updateTime){
			drawGraph(crtTime);
			updateTime = crtTime + pixelUpdatePeriod;
		}
	}
	endProcess();
}

//Draws the actual temperature on the screen
void drawGraph(uint32_t time){
	char tmp[9];
	uint8_t pX, pY;
	const uint8_t cY = 12;
	//Display curent temperature
	sprintf(tmp, "%.1f", crtTemp);
	strcat(tmp, txt[tempSoak]);
	ST7735_WriteString(X_118, Y_MIN, tmp, Font_7x10, RED, BLACK);
	//Display curent progress
	sprintf(tmp, "%.1f", TEMP_SCALE*((float)(time - myPoints[startPoint].Time)/(float)myPoints[endCool].Time));
	strcat(tmp, "%");
	ST7735_WriteString(X_118, cY, tmp, Font_7x10, YELLOW, BLACK);
	if(crtTemp <= myPoints[startPoint].Temperature){
		pY = Y_MAX;
	} else {
		pY = Y_MAX - (crtTemp - myPoints[startPoint].Temperature) * scalingY;
	}
	pX = (time - myPoints[startPoint].Time) * scalingX;
	ST7735_DrawPixel(pX, pY, YELLOW);
}





