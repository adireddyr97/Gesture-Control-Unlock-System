

#include <mbed.h>
#include "MPU6050.h"

float sum = 0;
uint32_t sumCount = 0;
MPU6050 mpu6050;
Timer t;
Serial pc(USBTX, USBRX);
DigitalOut led(LED1);
DigitalIn button(PA15);

int main()
{
  pc.baud(9600);
  i2c.frequency(400000);
  t.start();
  uint8_t whoami = mpu6050.readByte(MPU6050_ADDRESS, WHO_AM_I_MPU6050);
  pc.printf("I AM 0x%x\n\r", whoami);
  pc.printf("I SHOULD BE 0x68\n\r");
  // Connfigure the accelerometer.
  if (whoami == 0x68)
  {
    pc.printf("MPU6050 is online...\n");
    wait(1);

    mpu6050.MPU6050SelfTest(SelfTest);
    pc.printf("x-axis self test: acceleration trim within : ");
    pc.printf("%f", SelfTest[0]);
    pc.printf("% of factory value \n\r");
    pc.printf("y-axis self test: acceleration trim within : ");
    pc.printf("%f", SelfTest[1]);
    pc.printf("% of factory value \n\r");
    pc.printf("z-axis self test: acceleration trim within : ");
    pc.printf("%f", SelfTest[2]);
    pc.printf("% of factory value \n\r");
    wait(1);

    if (SelfTest[0] < 1.0f && SelfTest[1] < 1.0f && SelfTest[2] < 1.0f)
    {
      mpu6050.resetMPU6050();
      mpu6050.calibrateMPU6050(gyroBias, accelBias);
      mpu6050.initMPU6050();
      pc.printf("MPU6050 initialized for active data mode....\n\r");
      wait(2);
    }
    else
    {
      pc.printf("Device did not the pass self-test!\n\r");
    }
  }
  else
  {
    pc.printf("Could not connect to MPU6050: \n\r");
    pc.printf("%#x \n", whoami);
    while (1)
      ;
  }

  int vals[30];  // array for recorded sequence
  int vals2[30]; // array for unlock sequence
  pc.printf("Single press to record sequence.\n Long press to Unlock device.\n");
  while (1)
  {

    if (button == 0)
    {
      wait(1); // wait to detect second click
      if (button == 0)
      {
        // long press detected
        pc.printf("Unlock initiated....\n\n");

        for (int i = 0; i < 30; i += 1)
        {
          wait(0.1);
          // If data ready bit set, all data registers have new data
          if (mpu6050.readByte(MPU6050_ADDRESS, INT_STATUS) & 0x01)
          { // check if data ready interrupt

            mpu6050.readAccelData(accelCount); // Read the x/y/z adc values
            mpu6050.getAres();

            // Now we'll calculate the accleration value into actual g's
            ax = (float)accelCount[0] * aRes - accelBias[0]; // get actual g value, this depends on scale being set
            ay = (float)accelCount[1] * aRes - accelBias[1];
            az = (float)accelCount[2] * aRes - accelBias[2];
            //append ax , ay, az in vals2 array
            vals2[i] = ax;
            vals2[i + 1] = ay;
            vals2[i + 2] = az;
          }
        }
        int diff[30];
        count = 0;
        for (int i = 0; i < 30; i++)
        {
          diff[i] = vals[i] - vals2[i];
          if (abs(diff[i]) > 0.1)
          {
            pc.printf("Incorrect Sequence. Device is Locked\n");
            count = 1;
            while (button == 1)
            {
              led = 1;
              wait(0.1);
              led = 0;
              wait(0.1);
            }
            break;
          }
        }
        if (count == 0)
        {
          pc.printf("Unclocked\n");
        }
      }

      else
      {

        pc.printf("Recording Sequence......\n\n");
        for (int i = 0; i < 30; i += 1)
        {
          wait(0.1);
          //switch is pressed once)
          // If data ready bit set, all data registers have new data
          if (mpu6050.readByte(MPU6050_ADDRESS, INT_STATUS) & 0x01)
          { // check if data ready interrupt

            mpu6050.readAccelData(accelCount); // Read the x/y/z adc values
            mpu6050.getAres();

            // Now we'll calculate the accleration value into actual g's
            ax = (float)accelCount[0] * aRes - accelBias[0]; // get actual g value, this depends on scale being set
            ay = (float)accelCount[1] * aRes - accelBias[1];
            az = (float)accelCount[2] * aRes - accelBias[2];
            // append ax,ay and az values for record sequence..
            vals[i] = ax;
            vals[i + 1] = ay;
            vals[i + 2] = az;
          }
        }
        pc.printf("Sequence Recorded.\n\n");
      }
    }
  }
}
// check if sequence is same or different..
