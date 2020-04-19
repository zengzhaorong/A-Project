#include <iostream>
#include <unistd.h>


using namespace std;


int main(int argc, char* argv[])
{
	int sec = 0;
	
	(void)argc;
	(void)argv;

	cout << "hello background_app" << endl;

	while(1)
	{
		printf("sec: %d\n", sec++);
		sleep(1);
	}

	return 0;
}
