#include "Arachne.h"
#include <stdio.h>



void printInteger(int i) {
   printf("%d\n", i);
}
int main(){
    // Initialize the library
    Arachne::threadInit();       

    // Add some work
    Arachne::createTask([](){ printInteger(5); });
    Arachne::createTask([](){ printInteger(4); });
    Arachne::createTask([](){ printInteger(3); });
    Arachne::createTask([](){ printInteger(2); });
    Arachne::createTask([](){ printInteger(1); });
    
    // Must be the last call
    Arachne::mainThreadJoinPool();
}

