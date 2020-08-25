// -*- mode: C++ -*-
/*
 * gpsMT3339.h
 *
 *  Created on: Jun 11, 2020
 *      Author: Tilman Glötzner
 */

#ifndef GPSMT3339_H_
#define GPSMT3339_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#define LEN_GPS_READ_BUFFER 255
#define LEN_GPS_INFO_BUFFER 20
#define LEN_GPS_SENTENCE_BUFFER 100

// reading the serial port may eventually lock up.
// MAX_EMPTY_READ determines how many consecutive reads yielding no results, i.e.
// nofCharRead = 0, may occur before the program tries to reopen the gps device
#define MAX_EMPTY_READ 15
#define GPS_DEVICE "/dev/ttyS0"



class gps_MT3339 {
public:
	gps_MT3339(const char* SerialDevice,pthread_mutex_t* mutex);
	virtual ~gps_MT3339();
	void*  read_gps(void* ptr);
	char* getLatitude();
	char* getLongitude();
	char* getTimestamp();
	void stop();


private:
	char *strnstr(const char *haystack, const char *needle, size_t len);
	char longitude[LEN_GPS_INFO_BUFFER]="NoInit";
	char latitude[LEN_GPS_INFO_BUFFER]="NoInit";
	char timestamp[LEN_GPS_INFO_BUFFER]="NoInit";
	bool stop_reading = false;
	char *gpsDevice;
	pthread_mutex_t* mutex;
};

#endif /* GPSMT3339_H_ */
