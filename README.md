# Chip 8 Emulator

## Description
This is a Chip 8 emulator (interpreter) made with C++. Window graphics, user input, and sound are all handled with SDL, with OpenGL used for the video renderer. As of now, there is only support for the basic Chip 8.

The Chip 8 test suite by Timendus is used to verify the performance/behavior of this project. This currently passes all tests except for the Display Wait test in 5-quirks.ch8. A user online notes that to pass this test, you would need to limit the instructions/frame to 8 instructions. An ipf of 12 is used as it feels better and more closely imitates the recommended 700 ips. The third test in 6-keypad.ch8 also doesn't pass, meaning that the input read instruction doesn't wait for a key release. So far, this doesn't seem to dramatically affect game performance.

## How to use.
Run the main file in the terminal, using the ROM name as an argument to load in a ROM.

## Areas for future development
The biggest area to improve would be adding support for different Chip 8 types (SuperCHIP and XOChip). Additionally, the 60 fps display rate is achieved using a static delay of 16 ms, not accounting for the time it takes to execute the instructions.

## Resources used for the project
Good written guides for making the emulator

https://tobiasvl.github.io/blog/write-a-chip-8-emulator/

https://austinmorlan.com/posts/chip8_emulator/

Documentation for SDL2

https://wiki.libsdl.org/SDL2/FrontPage

Queso Fuego's YouTube Playlist on making a Chip 8 emulator in C

https://www.youtube.com/playlist?list=PLT7NbkyNWaqbyBMzdySdqjnfUFxt8rnU_

Games/ROMs for testing

https://github.com/dmatlack/chip8/tree/master

Timendus Test Suite

https://github.com/Timendus/chip8-test-suite

