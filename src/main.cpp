#include <Arduino.h>
#include <Wire.h>
#include "paj7620.h"
#include <Adafruit_NeoPixel.h>

enum Color
{
	RED,
	GREEN,
	BLUE,
	YELLOW,
	PURPLE,
	CYAN,
	WHITE,
	OFF
};

enum DisplayMode
{
	ANTI_CLOCKWISE,
	CLOCKWISE,
	COLOR
};

enum InteractionMode
{
	ACTIVE,
	WAITING
};
/*
Notice: When you want to recognize the Forward/Backward gestures, your gestures' reaction time must less than gestureEntryTime(0.8s).
		You also can adjust the reaction time according to the actual circumstance.
*/
const int gestureReactionTime = 500; // You can adjust the reaction time according to the actual circumstance.
const int gestureEntryTime = 800;	 // When you want to recognize the Forward/Backward gestures, your gestures' reaction time must less than gestureEntryTime(0.8s).
const int gestureQuitTime = 1000;
const int ledRGBPin = 26;
const int rgbPinAmount = 12;
const int defaultBrightnessLevel = 100;
const int maxBrightnessLevel = 200;
const int minBrightnessLevel = 0;
const int waitingDelay = 75;
const int ledSwitchDelay = 150;
const int gestureWaitDelay = 100;

const int amountOfTurns = 2;

const int southStartLED = 10;
const int westStartLED = 1;
const int northStartLED = 4;
const int eastStartLED = 7;

const int distancePin = 34;

InteractionMode interactionMode = WAITING;
InteractionMode lastInteractionMode = WAITING;
int waitingStep = 0;
Color waitingColor = OFF;
const int activeLoopsDuration = 10;
int activeStep = 0;

Color currentColor = GREEN;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(rgbPinAmount, ledRGBPin, NEO_GRB + NEO_KHZ800);

Color getNextColor(Color color)
{
	switch (color)
	{
	case WHITE:
		return RED;
	case RED:
		return GREEN;
	case GREEN:
		return BLUE;
	case BLUE:
		return YELLOW;
	case YELLOW:
		return PURPLE;
	case PURPLE:
		return CYAN;
	default:
		return WHITE;
	}
}

Color getPreviousColor(Color color)
{
	switch (color)
	{
	case WHITE:
		return CYAN;
	case CYAN:
		return PURPLE;
	case PURPLE:
		return YELLOW;
	case YELLOW:
		return BLUE;
	case BLUE:
		return GREEN;
	case GREEN:
		return RED;
	default:
		return WHITE;
	}
}

void setPixelColor(int pixel, Color color)
{
	switch (color)
	{
	case RED:
		strip.setPixelColor(pixel, strip.Color(255, 0, 0));
		break;
	case GREEN:
		strip.setPixelColor(pixel, strip.Color(0, 255, 0));
		break;
	case BLUE:
		strip.setPixelColor(pixel, strip.Color(0, 0, 255));
		break;
	case YELLOW:
		strip.setPixelColor(pixel, strip.Color(255, 255, 0));
		break;
	case PURPLE:
		strip.setPixelColor(pixel, strip.Color(255, 0, 255));
		break;
	case CYAN:
		strip.setPixelColor(pixel, strip.Color(0, 255, 255));
		break;
	case WHITE:
		strip.setPixelColor(pixel, strip.Color(255, 255, 255));
		break;
	case OFF:
		strip.setPixelColor(pixel, strip.Color(0, 0, 0));
		break;
	}
}

void setPixelColorAndShow(int pixel, Color color)
{
	setPixelColor(pixel, color);

	strip.show();
}



void setStripRGBValues(int red, int green, int blue)
{
	for (int i = 0; i < rgbPinAmount; i++)
	{
		strip.setPixelColor(i, strip.Color(red, green, blue));
	}
}

void setStripColor(Color color)
{
	for (int i = 0; i < rgbPinAmount; i++)
	{
		setPixelColor(i, color);
	}
	strip.show();
}

void setStripRainbowClockwise()
{
	Color startColor = currentColor;
	for (int i = 0; i < rgbPinAmount * amountOfTurns; i++)
	{
		Color color = startColor;
		for (int j = 0; j < rgbPinAmount; j++)
		{
			setPixelColor(j, color);
			color = getNextColor(color);
		}
		strip.show();
		delay(ledSwitchDelay);
		startColor = getNextColor(startColor);
	}
}

void setStripRainbowAntiClockwise()
{
	Color startColor = currentColor;
	for (int i = rgbPinAmount; i >= 0; i--)
	{
		Color color = startColor;
		for (int j = 0; j < rgbPinAmount; j++)
		{
			setPixelColor((j + rgbPinAmount) % rgbPinAmount, color);
			color = getPreviousColor(color);
		}
		strip.show();
		delay(ledSwitchDelay);
		startColor = getPreviousColor(startColor);
	}
}

void cascadeColorsLeftToRight(Color initialColor, Color targetColor)
{
	for (int i = 0; i < rgbPinAmount / 2; i++)
	{
		setPixelColorAndShow(i, targetColor);
		setPixelColorAndShow(rgbPinAmount - 1 - i, targetColor);
		strip.show();
		delay(ledSwitchDelay);
	}
	currentColor = targetColor;
}

void cascadeColorsRightToLeft(Color initialColor, Color targetColor)
{
	for (int i = 0; i < rgbPinAmount / 2; i++)
	{
		setPixelColorAndShow(rgbPinAmount / 2 - i - 1, targetColor);
		setPixelColorAndShow(rgbPinAmount / 2 + i, targetColor);
		strip.show();
		delay(ledSwitchDelay);
	}
	currentColor = targetColor;
}

void setOnlyLEDArea(int startLED, Color color)
{
	setStripColor(currentColor);
	for (int i = 0; i < 4; i++)
	{
		setPixelColorAndShow((startLED + i) % 12, color);
	}
}

void setLEDArea(int startLED, Color color)
{
	for (int i = 0; i < 4; i++)
	{
		setPixelColorAndShow((startLED + i) % 12, color);
	}
}

void setStripQuartersToDifferentColors()
{
	setLEDArea(northStartLED, currentColor);
	setLEDArea(eastStartLED, getNextColor(currentColor));
	setLEDArea(southStartLED, getNextColor(getNextColor(currentColor)));
	setLEDArea(westStartLED, getNextColor(getNextColor(getNextColor(currentColor))));
}

void upGesture()
{
	Serial.println("Up Gesture");
	setOnlyLEDArea(northStartLED, getNextColor(currentColor));
	currentColor = getNextColor(currentColor);
}

void downGesture()
{
	Serial.println("Down Gesture");
	setOnlyLEDArea(southStartLED, getNextColor(currentColor));
	currentColor = getNextColor(currentColor);
}

void leftGesture()
{
	Serial.println("Left Gesture");
	setOnlyLEDArea(westStartLED, getNextColor(currentColor));
	currentColor = getNextColor(currentColor);
}

void rightGesture()
{
	Serial.println("Right Gesture");
	setOnlyLEDArea(eastStartLED, getNextColor(currentColor));
	currentColor = getNextColor(currentColor);
}

void forwardGesture()
{
	Serial.println("Forward Gesture");
	setStripColor(OFF);
}

void backwardGesture()
{
	Serial.println("Backward Gesture");
	setStripColor(currentColor);
}

void waveGesture()
{
	Serial.println("Wave Gesture");
	currentColor = getNextColor(currentColor);
	setStripQuartersToDifferentColors();
}

void clockwiseGesture()
{
	Serial.println("Clockwise Gesture");
	setStripRainbowClockwise();
}

void anticlockwiseGesture()
{
	Serial.println("Anticlockwise Gesture");
	setStripRainbowAntiClockwise();
}

void setup()
{
	Serial.begin(9600);

	strip.begin();
	strip.setBrightness(maxBrightnessLevel);
	setStripColor(OFF);

	pinMode(distancePin, INPUT);

	uint8_t error = 0;
	Serial.println("\nPAJ7620U2 TEST DEMO: Recognize 9 gestures.");

	error = paj7620Init(); // initialize Paj7620 registers
	if (error)
	{
		Serial.print("INIT ERROR,CODE:");
		Serial.println(error);
	}
	else
	{
		Serial.println("INIT OK");
	}
	Serial.println("Please input your gestures:\n");
}

void loop()
{
	uint8_t data = 0, data1 = 0, error;
	if(interactionMode == WAITING || activeStep % activeLoopsDuration == 0){
	if (digitalRead(distancePin) == LOW)
	{
		interactionMode = ACTIVE;
	}
	else
	{
		interactionMode = WAITING;
	}
}
	if (interactionMode == ACTIVE)
	{
		activeStep++;
		if (lastInteractionMode != interactionMode)
		{
			lastInteractionMode = interactionMode;
			strip.setBrightness(255);
			setStripColor(WHITE);
			delay(1000);
		}
		error = paj7620ReadReg(0x43, 1, &data); // Read Bank_0_Reg_0x43/0x44 for gesture result.
		if (!error)
		{
			switch (data) // When different gestures be detected, the variable 'data' will be set to different values by paj7620ReadReg(0x43, 1, &data).
			{
			case GES_RIGHT_FLAG:
				delay(gestureEntryTime);
				paj7620ReadReg(0x43, 1, &data);
				if (data == GES_FORWARD_FLAG)
				{
					forwardGesture();
					delay(gestureQuitTime);
				}
				else if (data == GES_BACKWARD_FLAG)
				{
					backwardGesture();
					delay(gestureQuitTime);
				}
				else
				{
					rightGesture();
				}
				break;
			case GES_LEFT_FLAG:
				delay(gestureEntryTime);
				paj7620ReadReg(0x43, 1, &data);
				if (data == GES_FORWARD_FLAG)
				{
					forwardGesture();
					delay(gestureQuitTime);
				}
				else if (data == GES_BACKWARD_FLAG)
				{
					backwardGesture();
					delay(gestureQuitTime);
				}
				else
				{
					leftGesture();
				}
				break;
			case GES_UP_FLAG:
				delay(gestureEntryTime);
				paj7620ReadReg(0x43, 1, &data);
				if (data == GES_FORWARD_FLAG)
				{
					forwardGesture();
					delay(gestureQuitTime);
				}
				else if (data == GES_BACKWARD_FLAG)
				{
					backwardGesture();
					delay(gestureQuitTime);
				}
				else
				{
					upGesture();
				}
				break;
			case GES_DOWN_FLAG:
				delay(gestureEntryTime);
				paj7620ReadReg(0x43, 1, &data);
				if (data == GES_FORWARD_FLAG)
				{
					forwardGesture();
					delay(gestureQuitTime);
				}
				else if (data == GES_BACKWARD_FLAG)
				{
					backwardGesture();
					delay(gestureQuitTime);
				}
				else
				{
					downGesture();
				}
				break;
			case GES_FORWARD_FLAG:
				forwardGesture();
				delay(gestureQuitTime);
				break;
			case GES_BACKWARD_FLAG:
				backwardGesture();
				delay(gestureQuitTime);
				break;
			case GES_CLOCKWISE_FLAG:
				clockwiseGesture();
				break;
			case GES_COUNT_CLOCKWISE_FLAG:
				anticlockwiseGesture();
				break;
			default:
				paj7620ReadReg(0x44, 1, &data1);
				if (data1 == GES_WAVE_FLAG)
				{
					waveGesture();
				}
				break;
			}
		}
		delay(gestureWaitDelay);
	}
	else
	{
		if (lastInteractionMode != interactionMode)
		{
			lastInteractionMode = interactionMode;
			setStripColor(OFF);
		}
		{
			if (waitingStep <= 15)
			{
				strip.setBrightness(waitingStep * 17);
			} else {
				strip.setBrightness(255 - (waitingStep % 15) * 17);
			}

			setStripColor(BLUE);
			waitingStep = (waitingStep + 1) % 30;
			delay(waitingDelay);
		}

		setStripColor(OFF);
	}
}