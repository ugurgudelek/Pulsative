/*
 Name:		Pulsative.ino
 Created:	01-Sep-16 8:51:16 AM
 Author:	Ugur
*/

#define PULSE_PIN A0
#define BAUDRATE 115200
#define PULSED_LOWER_OFFSET 50
#define PULSED_UPPER_OFFSET 500
#define MAX_BPM 200
#define MIN_BPM 40


float output = 0.0;
unsigned long loopPeriod = 0L;
unsigned long necessaryDelayMicro = 10000L;
float oldInput[] = { 0,0,0,0,0,0,0,0,0,0 };
int next = 0;
float bpm[] = { 70,70,70,70,70,70,70,70,70,70 };
int bpmPtr = 0;

enum Mode
{
	stayStill = 0,
	increasing = 1,
	decreasing = 2
};

Mode mode = stayStill;
bool canCalculate = false;
unsigned long lastBpmTickMicros = 0;
unsigned long bpmPeriodMicros = 0;
bool bpmTick = false;

float averageOf(float* arr, int len)
{
	float sum = 0.0f;
	for(int i = 0; i < len; i++)
	{
		sum += arr[i];
	}
	return sum / len;
}

unsigned long bpmPeriod(int input)
{
	bpmTick = false;
	 
	if (input < PULSED_UPPER_OFFSET && input > PULSED_LOWER_OFFSET)
	{
		mode = (averageOf(oldInput, 10) > input) ? decreasing : ((averageOf(oldInput, 10) < input) ? increasing : stayStill);
		oldInput[next++%10] = input;
	}

	if (mode != increasing)
	{
		canCalculate = true;
	}

	if (canCalculate && mode == increasing)
	{
		unsigned long currentBpmTickMicros = micros();
		bpmPeriodMicros = currentBpmTickMicros - lastBpmTickMicros;
		lastBpmTickMicros = currentBpmTickMicros;
		canCalculate = false;
		bpmTick = true;
	}
	
	return bpmPeriodMicros;

}

float* weighted(float* arr, int len)
{
	float average = averageOf(arr, len);


	for(int i = 0; i< len; i++)
	{
		float weight = (arr[i] / average) * 0.7  + (average / arr[i]) * 0.3;
		arr[i] = average * weight;
	}
	return arr;
}

float boundedAndAveraged(float* arr, int len)
{
	float sum = 0.0f;
	int minus = 0;
	for (int i = 0; i < len; i++)
	{
		if((arr[i] > 200)||(arr[i] < 40))
		{
			minus++;
		}
		else
		{
			sum += arr[i];
		}
		
	}
	return sum / (len-minus);
}

float* bounded(float* arr, int len)
{
	float average = averageOf(arr, len);
	for (int i = 0; i < len; i++)
	{
		if ((arr[i] > 200) || (arr[i] < 40))
		{
			arr[i] = average;
		}
	}
	return arr;
}

// the setup function runs once when you press reset or power the board
void setup() 
{
	pinMode(PULSE_PIN, INPUT);
	Serial.begin(BAUDRATE);
}

// the loop function runs over and over again until power down or reset
void loop() 
{
	loopPeriod = micros();

	int reading = analogRead(PULSE_PIN);
	reading = (reading > 50) ? reading : 0;


	float period = float(bpmPeriod(reading));
	float freq = 60.0f*(1000000.0f /period);

	

	//Serial.print("input : "); Serial.print(reading); Serial.print("\t");
	//Serial.print("mode : "); Serial.print(mode); Serial.print("\t");
	
	if(bpmTick)
	{
		bpm[bpmPtr++ % 10] = freq;
		float output = averageOf(weighted(bounded(bpm, 10), 10), 10);
		//Serial.print("period : "); Serial.print(period);Serial.print("\t");
		//Serial.print("freque : "); Serial.print(output);Serial.print("\t");
		//Serial.println();
	}
	//Serial.println();
	Serial.println(reading);
	loopPeriod = micros() - loopPeriod;
	delayMicroseconds(necessaryDelayMicro - loopPeriod);
	
}



