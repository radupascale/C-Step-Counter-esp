#include <inttypes.h>

// choose sqrt instead of math for a non-float dependent implementation
#include "sqrt.h"
// #include <math.h>

typedef int32_t accel_t;
typedef int64_t time_t;
// magnitude should be bigger than accel_t
typedef int64_t magnitude_t;

// accumulator of magnitude, shuold be bigger than magnitude
typedef int64_t accumulator_t;

// steps count type
typedef int32_t steps_t;

/* Uncomment this line to disable interpolation */
#define STEP_INTERPOLATION_DISABLE

// skip filtering step
#define SKIP_FILTER

// use this to allow dumping each stage on file, useful for debugging
// #define DUMP_FILE
#define DUMP_MAGNITUDE_FILE_NAME "magnitude.csv"
#define DUMP_INTERPOLATED_FILE_NAME "interpolated.csv"
#define DUMP_FILTERED_FILE_NAME "filtered.csv"
#define DUMP_SCORING_FILE_NAME "scoring.csv"
#define DUMP_DETECTION_FILE_NAME "detection.csv"
#define DUMP_POSTPROC_FILE_NAME "postproc.csv"