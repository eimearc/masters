#include "flags.h"

#include <math.h>

static bool ValidateNumCubes(const char* flagname, int value) {
    if (value <= 1)
    {
        printf("--%s must be greater than 1. Got %d.\n", flagname, value);
        return false;
    }
    int sr = sqrt(value);
    if (sr*sr != value)
    {
        printf("--%s must be a perfect square. Got %d.\n", flagname, value);
        return false;
    }
    return true;
}
DEFINE_int32(num_cubes, 16, "Number of cubes to render. Must be a square number greater than 1.");
DEFINE_validator(num_cubes, &ValidateNumCubes);

static bool ValidateNumThreads(const char* flagname, int value) {
    if (value < 1 || value > 4)
    {
        printf("--%s must be between 1 and 4 inclusive. Got %d.\n", flagname, value);
        return false;
    }
    return true;
}
DEFINE_int32(num_threads, 1, "Number of threads to use. Must be between 1 and 4 inclusive.");
DEFINE_validator(num_threads, &ValidateNumThreads);

DEFINE_bool(overwrite, false, "If true, overwrite the old CSV file for the benchmark.");
DEFINE_int32(num_frames, 0, "Number of frames to render.");
DEFINE_string(file,"time.csv","The path to the file where the output should be logged.");