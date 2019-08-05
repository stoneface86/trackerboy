
#include "tables.h"

namespace gbsynth {

const float SAMPLE_TABLE[16] = {
    -1.0f,
    -7.0f / 8.0f,
    -6.0f / 8.0f,
    -5.0f / 8.0f,
    -4.0f / 8.0f,
    -3.0f / 8.0f,
    -2.0f / 8.0f,
    -1.0f / 8.0f,
    0.0f,
    1.0f / 7.0f,
    2.0f / 7.0f,
    3.0f / 7.0f,
    4.0f / 7.0f,
    5.0f / 7.0f,
    6.0f / 7.0f,
    1.0f
};

const float ENV_TABLE[16] = {
    0.0f,
    1.0f / 15.0f,
    2.0f / 15.0f,
    3.0f / 15.0f,
    4.0f / 15.0f,
    5.0f / 15.0f,
    6.0f / 15.0f,
    7.0f / 15.0f,
    8.0f / 15.0f,
    9.0f / 15.0f,
    10.0f / 15.0f,
    11.0f / 15.0f,
    12.0f / 15.0f,
    13.0f / 15.0f,
    14.0f / 15.0f,
    1.0f
};

const float VOLUME_TABLE[8] = {
    0.125f,
    0.25f,
    0.375f,
    0.5f,
    0.625f,
    0.75f,
    0.875f,
    1.0f,
};

}