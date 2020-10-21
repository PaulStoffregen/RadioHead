// -*- mode: C++ -*-
/*
 * gpsMT3339.cpp
 *
 *  Created on: Jun 11, 2020
 *      Author: Tilman Glötzner
 */

#include <gpsMT3339.h>
#include <stdio.h>
#include <help_functions.h>
#include <unistd.h>
#include <stdlib.h>



gps_MT3339::gps_MT3339(const char* SerialDevice, pthread_mutex_t* mutex) {
	// TODO Auto-generated constructor stub
	this->mutex = mutex;

	gpsDevice = (char*)malloc(strlen(SerialDevice)*sizeof(char));
	if (gpsDevice == NULL)
		return;
	strcpy(gpsDevice,SerialDevice);
}

gps_MT3339::~gps_MT3339() {
	// TODO Auto-generated destructor stub
	stop_reading = true;
}

char* gps_MT3339::getLatitude() {
	return latitude;
}

char* gps_MT3339::getLongitude() {
	return longitude;
}

char* gps_MT3339::getTimestamp() {
	return timestamp;
}

//function to read gps device of Dragino GPS hat (to be executed by a thread)
char* gps_MT3339::strnstr(const char *haystack, const char *needle, size_t len)
{
        int i;
        size_t needle_len;

        if (0 == (needle_len = strnlen(needle, len)))
                return (char *)haystack;

        for (i=0; i<=(int)(len-needle_len); i++)
        {
                if ((haystack[0] == needle[0]) &&
                        (0 == strncmp(haystack, needle, needle_len)))
                        return (char *)haystack;

                haystack++;
        }
        return NULL;
}

void* gps_MT3339::read_gps(void* ptr)
{
	ssize_t n, gga_len,i, comma_count;
	char* start_ptr;
	char* end_ptr;
	char gps_buffer[LEN_GPS_READ_BUFFER];
	char GPGGA_sentence[LEN_GPS_SENTENCE_BUFFER];
	int gps_fd = -1;
	int nofchar;
	unsigned int empty_read_counter = 0;

	printf("\nGPS READER started\n");
	print_scheduler();
	print_scope();
	memset(gps_buffer,0x00,LEN_GPS_READ_BUFFER);

	while (!stop_reading)
	{
		if (gps_fd < 0)
		{
			// Open the device in non-blocking mode
			gps_fd = open(gpsDevice, O_RDWR | O_NONBLOCK);
			if(gps_fd < 0)
			{
				printf("\nWarning: Could not open device file of GPS\n" );
			}
		}
		else
		{
			n = read(gps_fd, gps_buffer, 200);

			if (n == 0)
			{
				// counter incremented if there was nothing read back
				empty_read_counter++;
			}
			if (n <= 0)
			{
				if (((errno != EAGAIN) and (errno != EWOULDBLOCK))
						|| (empty_read_counter > MAX_EMPTY_READ))
				{
				// on read error close file descriptor and restart
					printf("GPS device being reset\n");
					strcpy(longitude,"NoValue");
					strcpy(latitude,"NoValue");
					strcpy(timestamp,"NoValue");

					close(gps_fd);
					gps_fd = -1;
					empty_read_counter = 0;
					memset(gps_buffer,0x00,LEN_GPS_READ_BUFFER);
				}
			}
			else
			{
				// sucessful read => reset counter tracking empty reads
				empty_read_counter = 0;

				// Scan for sentence read from GPS device containing position
				start_ptr = strnstr (gps_buffer, "$GPGGA", LEN_GPS_READ_BUFFER);
				if( start_ptr != NULL)
				{
					end_ptr = (char*)memchr(start_ptr, '*',
							LEN_GPS_READ_BUFFER - (start_ptr - gps_buffer));

					if (end_ptr != NULL)
					{

						gga_len = (end_ptr-start_ptr) < LEN_GPS_SENTENCE_BUFFER ?
								(end_ptr-start_ptr) : LEN_GPS_SENTENCE_BUFFER - 1;
						memcpy(GPGGA_sentence,gps_buffer,gga_len);
						// make sure that string is delimited
						GPGGA_sentence[gga_len+1] = '\0';
						//printf(GPGGA_sentence);

						// extract time and gps coordinates from GPGGA sentence
						// Sample: GPGGA,193630.000,4846.8772,N,00912.2288,E,1,5,1.57,131.5,M,48.0,M,,
						comma_count = 0;
						//printf("%s\n", GPGGA_sentence);
						start_ptr = strchr(GPGGA_sentence,',') + 1;
						end_ptr=start_ptr;
						pthread_mutex_lock(mutex);
						while (end_ptr != NULL && start_ptr != NULL)
						{
							end_ptr = strchr(start_ptr,',');
							if (end_ptr != NULL)
							{
								nofchar = (end_ptr - start_ptr);
								if (nofchar < LEN_GPS_INFO_BUFFER)
									end_ptr[0] = '\0';
								else
									break;

								int temp = nofchar;
								if (comma_count == 0)
								{
									nofchar = temp < LEN_GPS_INFO_BUFFER ?
											temp : LEN_GPS_INFO_BUFFER - 1;
									strncpy(timestamp,start_ptr,nofchar);
									timestamp[nofchar] = '\0';

								}
								if (comma_count == 1)
								{
									nofchar = temp < LEN_GPS_INFO_BUFFER - 2 ?
											temp : LEN_GPS_INFO_BUFFER - 2;
									strncpy(latitude,start_ptr,nofchar);
									latitude[nofchar] = '\0';

								}
								if (comma_count == 2)
								{
									nofchar = strlen(latitude);
									strncpy(latitude+nofchar,start_ptr,1);
									latitude[nofchar+1] = '\0';

								}
								if (comma_count == 3)
								{
									nofchar = temp < LEN_GPS_INFO_BUFFER - 2 ?
											temp : LEN_GPS_INFO_BUFFER - 2;
									strncpy(longitude,start_ptr,nofchar);
									longitude[nofchar] = '\0';

								}
								if (comma_count == 4)
								{
									nofchar = strlen(longitude);
									strncpy(longitude+nofchar,start_ptr,1);
									longitude[nofchar+1] = '\0';
									// all data extracted => bail out
									end_ptr = NULL;
									start_ptr = NULL;
								}
								start_ptr = end_ptr+1;

								comma_count++;
							}
						}
						pthread_mutex_unlock(mutex);
					}
				}

			}
		}
		pthread_yield();
	}
	free(gpsDevice);
	close(gps_fd);
}

void gps_MT3339::stop() {
	stop_reading = true;
}


