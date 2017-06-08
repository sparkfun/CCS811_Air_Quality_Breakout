SparkFun <PRODUCT NAME> Software
=================================

Example software for use with <PRODUCT NAME>.

Possible examples include python scripts, bash scripts, SPICE simulations, etc. 


Python Example
==============

Python Example is tested on Raspberry Pi Zero W. It use *pigpio* library for i2c
 communication which is installed on the RPi by default. Before usage start the pigpio 
 library as a daemon.
 
 > sudo pigpiod
 
 Run example with

> python ccs811.py
