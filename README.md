# rooster-controller 

Version Number
==============
`V <no args>`

Information on the version of the software.

Drive Motor
===========
`M <number [0: 100]>`

Toggle the motor state. 
This will change the duty cycle of the PWM (460Hz) pin that drives the motor.
The motor will remain on for MAX_TIME_TO_IDLE millisecods. 
After this time the resulting state will be as if `M 0` was used

Params:
------- 
* `number [0: 100]` _Duty cycle of the motor driver._ 

Switch Lights
=============
`L <number [0: 100]>`

Tottgle the light state. 
This will change the light state for MAX_TIME_TO_IDLE millisecods. 
After this time the resulting state will be ad if `L 0` was used.

Params
------
* `number [0: 100]` _Duty cycle of the light controller._

List Parametrs
====
`S <no args>`

List all the parameters of the device.

* MAX_TIME_TO_IDLE [read/write] [default: 5000]
* MAX_MESSAGE_LENGTH [read]
* IDLE_BLINK_RATE [read/write] [default: 2000]
* IDLE_DUTY_CYCLE [read/write] [default: 10]
* BT_BAUD_RATE [read/write]
* BT_PIN [read/write]

Change Parameters
=================
`S <string> <anything>`

Params
------
* `string` _Name of the parameter._
* `anything` _Value for the parameter. If outside of a valid range will ignore input._

Factory Reset
=============
`R <no args>` two times

This will reset all the parameter values to its default value.
