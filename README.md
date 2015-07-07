### GameBoy Emulator Source Code


The main reference used in building this emulator (almost exclusively) was the Pan Document or "Everything You Always Wanted To Know About GAMEBOY*".
This provided information about the registers, interrupts, cartridges, and memory layout of the GameBoy. 
Additional references were used to understand the GameBoy processor, mainly Z80 instruction references.
I'm sure there were also some other documents that I found that helped along the way.


The features of Greg's GameBoy Emulator are that it is written in C and compiles natively to many platforms. It uses SDL and can easily be ported.
And there is also a JavaScript implementation that can be used in the browser. Compatibility has not been widely tested but I will say that it
can emulate most original GameBoy games, excluding games that were compatible with both the GameBoy and the GameBoy color. GameBoy Color support
has not been fully implemented yet.


The emulator has no sound output although it binds an SDL audio interface for providing sound emulation in the future. It can be built for either
Windows or Linux (or some other platform that uses GCC).


Some caveats: There may be discrepencies on how this emulator draws a frame from what an actual GameBoy draws. For example, when there are overlapping
sprites it is hard to tell what the correct behavior should be. There also seems to be a vertical scanline that needs to be adjusted if not in the
normal emulator then in the web implementation.


There is nothing stopping implementing better features like Super GameBoy support, GameBoy Color support, etc., but I would say that this project was
mainly a first attempt at writing a portable emulator and not intended to be the end all be all of GameBoy emulators. There are certainly plenty of
options out there that support everything and everything else. I intend to add sound support and possible color and that's probably the end of the
line. Then I'd like to move on to other platforms to work on for something more exciting.


The license this software is provided under what I would term "Minimal Rights Reserved." The software is free to use for any purpose and there is no
need to include a notice when reproducing or modifying the software. There is also no warranty.


Cheers,
Greg