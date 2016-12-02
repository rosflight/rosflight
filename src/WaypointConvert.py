#!/usr/bin/python
# Taylor Pool
# November 9, 2016

# This script contains a function that converts given GPS coordinates into waypoints for ROS.

from math import pi
from math import cos
from math import sqrt

#constant
EARTH_RADIUS = 6371008.0							#in meters

#Function decimal_degrees
#Pre: string is in format [N/E or S/W]DD-MM-SS.SS
#Post: Returns GPS component in long decimal format
def decimal_degrees (string):
	a = 0

	firstLetter = string[0]
	if firstLetter == 'N' or firstLetter == 'E':
		a = 1
	elif firstLetter == 'S' or firstLetter == 'W':
		a = -1

	lessString = string.strip("NSEW ")

	values = lessString.split('-', 2)

	d = float(values[0])
	m = float(values[1])
	s = float(values[2])

	decimal = a*(d+(m/60.0)+(s/3600.0))

	return decimal


#Function meter_convert
#Takes in long decimal GPS format and outputs the destination in meters
#Pre: Lat and Long are given in decimal degrees, and altitude is in meters
#Post: Returns destination in tuple format [latChange, longChange, newAlt, distance]
def meter_convert(originLat, originLong, originAlt, newLat, newLong, newAlt):

	#Find CrossSectional Radius of Earth for Given Latitude Degree
	crossRadius = cos(originLat*pi/180.0)*EARTH_RADIUS

	GPSdestination = [newLat, newLong, -newAlt]	#Given in terms of N, E, D

	#Convert Change in GPS Coordinates to Change in Meters
	latChange = ((newLat - originLat)*pi/180.0)*EARTH_RADIUS		#in meters

	longChange = ((newLong - originLong)*pi/180.0)*crossRadius

	altitudeChange = newAlt - originAlt

	#Compute Total Distance to Fly
	distance = sqrt(pow(latChange, 2.0) + pow(longChange, 2.0) + pow(altitudeChange, 2.0))

	#New Waypoint Given in Terms of [North, East, Down]. In meters.
	destination = [latChange, longChange, newAlt, distance]

	return destination


#Function to_meters
#Pre: input of all the data needed to fly to a waypoint
#flag: tells whether altitude is in meters or feet
#		0: meters		else: feet
#Post: outputs destination given in meters that ROS can understand.
def to_meters(originLat, originLong, originAlt, newLat, newLong, newAlt, flag):

	#Altitude in Feet needs to be converted to Meters
	if (flag != 0):
		originAlt = .305*originAlt
		newAlt = .305*newAlt

	GPSorigin = [originLat, originLong, originAlt]
	GPSdestination = [newLat, newLong, newAlt]

	values = [str(originLat), str(originLong), str(newLat), str(newLong)]
	newValues = []

	for value in values:
		if ("N" in value) or ("S" in value) or ("E" in value) or ("W" in value) == 1:
			print "Degrees Minutes Seconds Format"
			print value
			newValues.append(decimal_degrees(value))
			print decimal_degrees(value)
		else:
			print "Long Decimal Format"
			newValues.append(float(value))

	destination = meter_convert(newValues[0], newValues[1], originAlt, newValues[2], newValues[3], newAlt)

	#Test Output
	print("Origin in GPS Coordinates: " + str(GPSorigin))
	print("\n")
	print("Destination in GPS Coordinates: " + str(GPSdestination))
	print("\n")
	print("Destination Coordinates (Meters) with Distance: " + str(destination))

	return destination



#######################################################################################################


#Test
test = to_meters("N90-90-76.45", "W45-67-23.54", 20.0, -40.257049176511316, 111.65421836078167, 20.0, 0)

#if __name__ == __main__:
#	toMeters(40.25787274333326, -111.65480308234692, -20.0, 40.257049176511316, -111.65421836078167, -20.0, 0)
