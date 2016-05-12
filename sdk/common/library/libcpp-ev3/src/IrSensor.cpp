//
// IrSensor.cpp
//
// Copyright (c) 2015-2016 Embedded Technology Software Design Robot Contest
//

#include "IrSensor.h"
using namespace ev3api;


//=============================================================================
// Constructor
IrSensor::IrSensor(ePortS port)
:
Sensor(port, ULTRASONIC_SENSOR)
{
    (void)ev3_ultrasonic_sensor_get_distance(getPort());
}

//=============================================================================
// Destructor
IrSensor::~IrSensor(void)
{
    //uart_sensor_config(getPort(), 3);
}

//=============================================================================
// get distance in cm
int16_t IrSensor::getDistance(void) const
{
    return ev3_ir_sensor_get_distance(getPort());
}

