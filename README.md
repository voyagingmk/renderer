## Quick Start

### Mac

1. mkdir xcode_build

2. cmake .. -G 'Xcode'

3. set the working directory to project's root directory

4. run


### Windows (32 bits)

1. mkdir vs_build

2. cmake ..

3. open [Configuration Properties -> Debugging]

3. set working directory: $(ProjectDir)/../

4. set environment: PATH=./lib/32/;$(Path)

The third party libraries may need to be re-compile:

SOIL.lib

assimp.lib

freetype.lib

SDL2.lib and SDL2main.lib (SDL2-2.0.4)




