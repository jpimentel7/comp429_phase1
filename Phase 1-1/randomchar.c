#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(){
	char *temp;
	temp = malloc(sizeof(char) * 1000);
	int ran;
	ran = open("/dev/random",O_RDONLY);
	read(ran,temp,1000);
	close(ran);

	printf("%s\n",temp );
}