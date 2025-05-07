Pisces
======
Pisces is a program to explore the possibilities of musical scales

Example: generate all 7 note scales with no more than 4 semitones in a row
./pisces 7 4


The Problem
-----------
Within a 12 tone system, enumerate all 1 octave unique scales of N notes with 
no more than M notes in a row.

A unique scale is a scale which cannot be formed by rotating any scale which
has already been enumerated.

Notes are in a row when they are connected by consecutive semitones eg
A, A#, B, C are 4 notes in a row.


Scale Representation
--------------------
Here we are representing scales using a pitch class bitfield. For example
111111111111 represents the chromatic scale and 101101010110 represents
the Dorian scale. This notation means the leftmost bit is always 1 since that
is the first note of the scale. 

This representation allows for simple and collision free hashing by treating
the bitfield as a binary number with a LSB on the left and converting it to
a decimal value. For example the chromatic scale hashes to 4095.

111100000000 has 4 notes in a row and so does 110000000011.

(X) 101101010110 and (Y) 101011010101 are considered as modes of the same scale 
because Y can be reached by rotating X 2 places to the right so only 1 or the 
other should be counted in the enumeration.


