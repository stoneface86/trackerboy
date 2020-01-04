#!/usr/bin/env python3
#
# File: gensinc.py
#
# Author: stoneface
#
# Usage: gensinc.py <phases> <setwidth>
#
# Utility script to generate a normalized sinc lookup table. The table is
# generated as a C/C++ 2D float array.
#
# This script can generate a table with a given number of phases
# and number of samples for each phase. The sum of each set is
# guarenteed to be equal to (or close enough to) 1.0. Phases range
# from 1/(n+1) to n/(n+1) where n is the number of phases.
#
# This script was used to generate the trackerboy::Osc::SINC_TABLE array
#

import math
import sys
import ctypes


def sinc(angle):
    """
    Calculates the normalized sinc function value for the given angle.
    When angle is 0, 1.0 is returned 
    """
    if angle == 0.0:
        return 1.0
    else:
        return (math.sin(math.pi * angle)) / (math.pi * angle)


def sincset(phase, width):
    """
    Sample a normalized sinc curve for a given phase and number of samples.
    A list of samples is returned, with the sum being equal to 1.0
    """
    start = width // 2
    end = width - start
    start = -start
    
    sset = [sinc(i + phase) for i in range(start, end)]
    error = 1.0 - sum(sset)
        
    # add the error to make the sum equal to 1
    
    if width & 2:
        # odd, add error to the center index
        sset[width // 2] += error
    else:
        # even, add half error to center indices
        halferror = error * 0.5
        sset[width // 2 - 1] += halferror
        sset[width // 2    ] += halferror

    return sset


def sincsetToArray(sset):
    return "{{ {} }}".format(", ".join(
        ["{:1.9f}f".format(ctypes.c_float(s).value) for s in sset]
    ))

def main(phases, stepwidth):
    sets = []

    # phases range from -1/m to -(m-1)/m
    # where m is the number of phases multiplied by 2
    # the distance between each phase is 1/phases
    # ie, phases = 32, then the range is -1/64 to -63/64

    m = phases * -2
    for i in range(phases):
        phase = ((2 * i) + 1) / m
        sset = sincset(phase, stepwidth)
        sets.append("/* Phase: {:0.3f} */    ".format(phase) + \
                     sincsetToArray(sset))

    print("{")
    print(",\n".join(sets))
    print("};")


if __name__ == "__main__":

    if len(sys.argv) != 3:
        print("usage: gensinc.py <phases> <stepwidth>")
        sys.exit(1)
    
    phases = int(sys.argv[1])
    stepwidth = int(sys.argv[2])

    main(phases, stepwidth)
    
