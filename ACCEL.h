#ifndef ACCEL_H
#define ACCEL_H

#include "Arduino.h"
#include "Wire.h"

//define the addresses and set them to golbal variables for use throughout the code
#define ACCEL_ADDR         0x68
#define ACCEL_SMPLRT_DIV   0x19
#define ACCEL_CONFIG       0x1a
#define ACCEL_GYRO_CONFIG  0x1b
#define MPU_ACCEL_CONFIG   0x1c
#define ACCEL_WHO_AM_I     0x75
#define ACCEL_PWR_MGMT_1   0x6b
#define ACCEL_TEMP_H       0x41
#define ACCEL_TEMP_L       0x42

//create a class called accelerometer
class ACCEL
{
  public:

  ACCEL(TwoWire &w) //Constructor
  {
    wire = &w;
    accCoef = 0.02f;  //single precision floating point number that is the accelerometer coeficient found online
    gyroCoef = 0.98f; //single precision floating point number that is the gyroscope coeficient found online
  }
  ACCEL(TwoWire &w, float accelCo, float gyroCo) //Initilizer of Constructor
  {
    wire = &w;
    accCoef = accelCo;
    gyroCoef = gyroCo;
  }

  void begin() //begin function which primaruli writes the addresses associated from the Uno to the Accelerometer
  {
    writeACCEL(ACCEL_SMPLRT_DIV, 0x00);
    writeACCEL(ACCEL_CONFIG, 0x00);
    writeACCEL(ACCEL_GYRO_CONFIG, 0x08);
    writeACCEL(MPU_ACCEL_CONFIG, 0x00);
    writeACCEL(ACCEL_PWR_MGMT_1, 0x01);
    this->update();
    angleGyroX = 0;
    angleGyroY = 0;
    angleX = this->getAccAngleX();
    angleY = this->getAccAngleY();
    preInterval = millis();
  }

  void setGyroOffsets(float x, float y, float z)
  {
    gyroXoffset = x;
    gyroYoffset = y;
    gyroZoffset = z;
  }
//Read and Write Equations set for Accelerometer data and registers
  void writeACCEL(byte reg, byte data)
  {
    wire->beginTransmission(ACCEL_ADDR);
    wire->write(reg);
    wire->write(data);
    wire->endTransmission();
  }
  byte readACCEL(byte reg)
  {
    wire->beginTransmission(ACCEL_ADDR);
    wire->write(reg);
    wire->endTransmission(true);
    wire->requestFrom(ACCEL_ADDR, 1);
    byte data =  wire->read();
    return data;
  }
  //Setters and Getters Defined Below
  int16_t getRawAccX()
  { 
    return rawAccX; 
  };

  int16_t getRawAccY()
  { 
    return rawAccY; 
  };

  int16_t getRawAccZ()
  { 
    return rawAccZ; 
  };

  int16_t getRawGyroX()
  { 
    return rawGyroX; 
  };

  int16_t getRawGyroY()
  { 
    return rawGyroY; 
  };

  int16_t getRawGyroZ()
  { 
    return rawGyroZ; 
  };

  float getAccX()
  { 
    return accX; 
  };

  float getAccY()
  { 
    return accY; 
  };

  float getAccZ()
  { 
    return accZ; 
  };

  float getGyroX()
  { 
    return gyroX; 
  };

  float getGyroY()
  { 
    return gyroY; 
  };

  float getGyroZ()
  { 
    return gyroZ; 
  };

	void calcGyroOffsets(bool console = false, uint16_t delayBefore = 1000, uint16_t delayAfter = 3000)
  {
    float x = 0, y = 0, z = 0;
	  int16_t rx, ry, rz;

    delay(delayBefore);
	  if(console)
    {
      Serial.println();
      Serial.println("========================================");
      Serial.println("Calculating gyro offsets");
      Serial.print("DO NOT MOVE MPU6050");
    }
    for(int i = 0; i < 3000; i++)
    {
      if(console && i % 1000 == 0)
      {
        Serial.print(".");
      }
      wire->beginTransmission(ACCEL_ADDR);
      wire->write(0x43);
      wire->endTransmission(false);
      wire->requestFrom((int)ACCEL_ADDR, 6);

      rx = wire->read() << 8 | wire->read();
      ry = wire->read() << 8 | wire->read();
      rz = wire->read() << 8 | wire->read();

      x += ((float)rx) / 65.5;
      y += ((float)ry) / 65.5;
      z += ((float)rz) / 65.5;
    }
    gyroXoffset = x / 3000;
    gyroYoffset = y / 3000;
    gyroZoffset = z / 3000;

    if(console)
    {
    Serial.println();
    Serial.println("Done!");
    Serial.print("X : ");Serial.println(gyroXoffset);
    Serial.print("Y : ");Serial.println(gyroYoffset);
    Serial.print("Z : ");Serial.println(gyroZoffset);
    Serial.println("Program will start after 3 seconds");
    Serial.print("========================================");
		delay(delayAfter);
	  }
  }

  float getGyroXoffset()
  { 
    return gyroXoffset; 
  };

  float getGyroYoffset()
  { 
    return gyroYoffset; 
  };

  float getGyroZoffset()
  { 
    return gyroZoffset; 
  };

  void update()
  {
    wire->beginTransmission(ACCEL_ADDR);
	  wire->write(0x3B);
	  wire->endTransmission(false);
	  wire->requestFrom((int)ACCEL_ADDR, 14);

    rawAccX = wire->read() << 8 | wire->read();
    rawAccY = wire->read() << 8 | wire->read();
    rawAccZ = wire->read() << 8 | wire->read();
    rawTemp = wire->read() << 8 | wire->read();
    rawGyroX = wire->read() << 8 | wire->read();
    rawGyroY = wire->read() << 8 | wire->read();
    rawGyroZ = wire->read() << 8 | wire->read();

    temp = (rawTemp + 12412.0) / 340.0;

    accX = ((float)rawAccX) / 16384.0; //raw values of X, Y and Z from MPU read up to 16,384
    accY = ((float)rawAccY) / 16384.0; //reset all raw values to zero and caluclate the angle in degrees
    accZ = ((float)rawAccZ) / 16384.0;

    angleAccX = (atan2(accY, sqrt(accZ * accZ + accX * accX)) * 360 / 2.0 / PI);
    angleAccY = (atan2(accX, sqrt(accZ * accZ + accY * accY)) * 360 / -2.0 / PI);

    gyroX = ((float)rawGyroX) / 65.5; //same process as above rather the gyro reads are capped at 65.5
    gyroY = ((float)rawGyroY) / 65.5;
    gyroZ = ((float)rawGyroZ) / 65.5;

    gyroX -= gyroXoffset;
    gyroY -= gyroYoffset;
    gyroZ -= gyroZoffset;

    interval = (millis() - preInterval) * 0.001; //interval between reads in milliseconds minus the desired interval between values

    angleGyroX += gyroX * interval;
    angleGyroY += gyroY * interval;
    angleGyroZ += gyroZ * interval;

    angleX = (gyroCoef * (angleX + gyroX * interval)) + (accCoef * angleAccX);
    angleY = (gyroCoef * (angleY + gyroY * interval)) + (accCoef * angleAccY);
    angleZ = angleGyroZ;

    preInterval = millis();
  }

  float getAccAngleX()
  { 
    return angleAccX; 
  };
  float getAccAngleY()
  { 
    return angleAccY; 
  };

  float getGyroAngleX()
  { 
    return angleGyroX; 
  };
  float getGyroAngleY()
  { 
    return angleGyroY; 
  };
  float getGyroAngleZ()
  { 
    return angleGyroZ; 
  };

  float getAngleX()
  { 
    return angleX; 
  };

  float getAngleY()
  { 
    return angleY; 
  };

  float getAngleZ()
  { 
    return angleZ; 
  };

  private:

  TwoWire *wire;

  int16_t rawAccX, rawAccY, rawAccZ, rawTemp,
  rawGyroX, rawGyroY, rawGyroZ;

  float gyroXoffset, gyroYoffset, gyroZoffset;

  float temp, accX, accY, accZ, gyroX, gyroY, gyroZ;

  float angleGyroX, angleGyroY, angleGyroZ,
  angleAccX, angleAccY, angleAccZ;

  float angleX, angleY, angleZ;

  float interval;
  long preInterval;

  float accCoef, gyroCoef;
};

#endif
