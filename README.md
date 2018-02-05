# C64-RC - a Raycasting Engine for Commodore 64

This project is an attempt to render 2.5 graphics on C64. I am trying to achieve
this with C compiled with cc65 so far, but am gradually rewriting different
code parts in assembly to speed things up.

There are 3 branches currently:

*   https://github.com/sborodavkin/c64-rc/tree/master - master, contains the
    16-bit implementation.
*   https://github.com/sborodavkin/c64-rc/tree/twofivesix - 100% 8-bit
    implementation without any calculations that would exceed 8 bit.
*   https://github.com/sborodavkin/c64-rc/tree/parametric - a fork that renders
    world map defined not as a 2D array, but as a list of walls represented
    by their coordinates (most likely with eventual BSP tree implementation).

(C) Sergey Borodavkin, 2018.
