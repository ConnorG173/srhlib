My Collection of Header-Only Libraries for Robotics

To Use:
1. Put the header you want to use in your project's directory
2. #include the header wherever you want to use the library interface
3. In ONE .c or .cpp file, first #define (The Name of the Header in ALL CAPS)_IMPLEMENTATION
4. Then, #include the header
5. Compile that .c or .cpp file and link it in with the others


Use Example

in all other .c/.cpp/.h files:  
#include "sr_lookuptable.h"  
//Code using that header's interface  

in ONE .c/.cpp file:  
#define SR_LOOKUPTABLE_IMPLEMENTATION  
#include "sr_lookuptable.h"  
