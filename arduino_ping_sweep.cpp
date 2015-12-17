/*
*
* Code by Carlos Capriotti
* Play nice: if you use the code, mention the author.
*
* Arduino, servo motor, PING sensor
*
* Uses a ultrasound sensor to sweep the environment, detect movement, and follow the 
* target
*/


#include <Ultrasonic.h>

#include <Servo.h>

Servo svr1; 
Ultrasonic us1(12,11);

int pos, maxScan, scanStep, scanGate, mainScanStart, mainScanEnd = 0;    // variable to store the servo position 
int initialScan = 80; // start of scanning position
long int curDist, maxDist, minDist = 0;
int loopCount, curPos, maxPos, minPos = 0;     // these variables will hold servo positions for current, min and max distances)
long int ranges[180];
 
void setup() 
{ 
  long int tempDist, antDist, auxPos, auxPos2 = 0;
  Serial.begin(115200);
  Serial.println();
  Serial.println("Initializing hardware in 5 seconds");
  Serial.println(); 
  delay(5000);
  svr1.attach(10);   
  Serial.println("Moving servo to zero position (middle of travel)"); 
  svr1.writeMicroseconds(1500);
  delay(3000);
  initialScan = 5; 
  maxScan = 175; 
  scanStep = 5;
  Serial.println("Running ititial scan of the environment");
  for(pos=0; pos <=179; pos++) { ranges[pos]=0;}              // Initializes the array
  
  
  // populates the array with the distance reading (typical) for the local
  // NOTE: positions 0 and 179 HAVE to have a value of 0
  
  for(pos = 4; pos <= 174; pos += 1)
  {                                   
    svr1.write(pos);                
    delay(30);
    loopCount = 0;
    minDist = 3500; 
    do                  // Reads 3 times each position, to make sure we are not reading garbage. Consider the biggest read
    {
      Serial.print(us1.Ranging(CM));
      Serial.print(" - ");
      curDist = us1.Ranging(CM);
      if (curDist < minDist) { minDist = curDist; }
      loopCount++;
    }
    while(loopCount < 3);
    
    ranges[pos] = minDist;     // might want to average the best of 3 readings or so, here.
    //ranges[pos] = us1.Ranging(CM);
    Serial.println();
    Serial.println(ranges[pos]);    
  }   
  
                                  // This loop sanitizes the ranges, excluding values ranging 35000 (infinite, or, no reading)
  for (pos=0; pos <=178; pos++)   // smooths readings, since the expected range is a semi-circle
  {                               // this is to avoid areas with max readings (+- 35000). (objects too far, or that do not echo.
    if (ranges[pos] >= 3400)
    {
      auxPos = pos;
      
      do
      {
        auxPos++;
      }
      while (ranges[auxPos] >= 3400);
      ranges[pos] = (ranges[pos-1]+ranges[auxPos])/2;  // fils the position with the average between the two lst positions with valid values.
    }
  }
  
  
  Serial.println("Ends hardware setup. Sweep starts in 3 seconds.");
  mainScanStart = 55;
  mainScanEnd = 105; 
  
  delay(3000);
} 
 
 
void loop() 
{ 
  
  for(pos = mainScanStart; pos<(mainScanStart+30); pos+=5)     
  {                                
    svr1.write(pos); 
    //Serial.println(pos);    
    delay(30);
    curPos = us1.Ranging(CM); 
    Serial.print(curPos);
    Serial.print("- dist vs stored - ");
    Serial.print(ranges[pos]);
    Serial.print(" - array position = ");
    Serial.println(pos);
    
    if (abs(curPos - ranges[pos]) > 5)
    {
      Serial.println("Target found");
    }
    delay(250);
  }
  
}




long int sweep(int vGate, int vStep, int startScan)    // returns the min dist. vGate is the aperture to scan in degrees, 
                                                       // vStep is the resolution (motor steps per movement) 
                                                       // startScan is the position, from 5 to 175 (for this servo motor)
                                                       // that vGate will be swept.
{
  minDist = 35000;       // bear with me. These are not inverted.
  maxDist = 0;
  if (startScan-vStep < 5) {startScan = 5; }   // to protect this servo
  
  if (vGate > 175) { scanGate = 175; }
  else { scanGate=vGate; }
  
  if ((scanGate/2) < vStep) { initialScan = vStep; }
  else { initialScan = startScan - (scanGate/2);  }
  
  for(pos = initialScan; pos <= (scanGate+initialScan); pos += vStep)
  {                                  // in steps of 1 degree 
    svr1.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(25);
    curDist = us1.Ranging(CM);
    if (maxDist < curDist) 
    { 
      maxDist = curDist;
      maxPos = pos;
      /*
      Serial.println("Max found");
      Serial.println(maxPos);
      Serial.println(curDist);
      */
    }
    if (minDist > curDist) 
    { 
      minPos = pos; 
      minDist = curDist;
      /*
      Serial.println("Min found");
      Serial.println(minPos);
      Serial.println(curDist);
      */
    }
    delay(100);                       
  } 
  
  for(pos = (initialScan+scanGate-vStep); pos>(initialScan); pos-=vStep)     // goes from 180 degrees to 0 degrees 
  {                                
    svr1.write(pos);               
    delay(25);
    curDist = us1.Ranging(CM);
    if (maxDist < curDist) 
    { 
      maxDist = curDist;
      maxPos = pos;
    }
    if (minDist > curDist) 
    { 
      minPos = pos; 
      minDist = curDist;
    }
    delay(100);                        
  }   
return minDist; 
}
