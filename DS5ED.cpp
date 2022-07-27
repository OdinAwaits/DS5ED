#include <Windows.h>
#include <iostream>
#include "ControllerHandler.h"

int main(int argc, char** argv) 
{
	ControllerHandler c = ControllerHandler();
	c.main();

	return 0;
}