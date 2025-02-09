
#pragma once

#include <QString>

//
// Converts the given tempo to a human readable string.
// Typically used for setting the text of a QLabel.
// The tempo is suffixed with units BPM (beats per minute)
// 
// ie 159.75f -> "159.75 BPM"
//
QString tempoToString(float tempo);

//
// Converts the given speed to a human readable string.
// Typically used for setting the text of a QLabel.
// The speed is suffixed with units FPR (frames per row)
// 
// ie 4.125f -> "4.125 FPR"
//
QString speedToString(float speed);
