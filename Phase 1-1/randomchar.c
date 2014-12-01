#include <stdio.h>

int main(){
	int temp =0;
	FILE *ran;
	ran = fopen("/dev/random","r");
	fread(&temp,sizeof(temp),1,ran);
	fclose(ran);

	printf("%d\n",temp );
}