/*
The MIT License (MIT)

Copyright (c) 2020 Anna Brondin and Marcus Nordström

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "detectionStage.h"
#include "config.h"
#include "postProcessingStage.h"
#if ESP_LOG_PEDOMETER
#include "esp_log.h"
static const char *TAG = "Detection stage";
#else
static const char *TAG = "";
#endif

#ifdef DUMP_FILE
#include <stdio.h>
static FILE *detectionFile;
static FILE *detectionFile;
#endif

static ring_buffer_t *inBuff;
static ring_buffer_t *outBuff;
static void (*nextStage)(void);

static magnitude_t mean = 0;
static accumulator_t std_acc = 0;
static time_accel_t count = 0;
static int16_t threshold_int = 0;
static int16_t threshold_frac = 6;

void initDetectionStage(ring_buffer_t *pInBuff, ring_buffer_t *peakBufIn,
						void (*pNextStage)(void))
{
	inBuff = pInBuff;
	outBuff = peakBufIn;
	nextStage = pNextStage;

#ifdef DUMP_FILE
	detectionFile = fopen(DUMP_DETECTION_FILE_NAME, "w+");
#endif
}

void detectionStage(void)
{
	if (!ring_buffer_is_empty(inBuff)) {
		accumulator_t oMean = mean;
		data_point_t dataPoint;
		ring_buffer_dequeue(inBuff, &dataPoint);
		count++;
		if (count == 1) {
			mean = dataPoint.magnitude;
			std_acc = 0;
		} else if (count == 2) {
			mean = (mean + dataPoint.magnitude) / 2;
			std_acc = sqrt(((dataPoint.magnitude - mean) *
							(dataPoint.magnitude - mean)) +
						   ((oMean - mean) * (oMean - mean))) /
					  2;
		} else {
			mean = (dataPoint.magnitude + ((count - 1) * mean)) / count;
			accumulator_t part1 =
				((std_acc * std_acc) / (count - 1)) * (count - 2);
			accumulator_t part2 = ((oMean - mean) * (oMean - mean));
			accumulator_t part3 =
				((dataPoint.magnitude - mean) * (dataPoint.magnitude - mean)) /
				count;
			std_acc = (accumulator_t)sqrt(part1 + part2 + part3);
		}
		if (count > 15) {
			if ((dataPoint.magnitude - mean) >
				(std_acc * threshold_int + (std_acc / threshold_frac))) {
                    LOG_PEDOMETER_FMT(TAG, "dataPoint.time: %u", dataPoint.time);
				// This is a peak
				ring_buffer_queue(outBuff, dataPoint);
				(*nextStage)();
#ifdef DUMP_FILE
				if (detectionFile) {
					if (!fprintf(detectionFile, "%lld, %lld\n", dataPoint.time,
								 dataPoint.magnitude))
						puts("error writing file");
					fflush(detectionFile);
				}
#endif
			}
		}
	}
}

void resetDetection(void)
{
	std_acc = 0;
	mean = 0;
	count = 0;
}

void changeDetectionThreshold(int16_t whole, int16_t frac)
{
	threshold_int = whole;
	threshold_frac = frac;
}