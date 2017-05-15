#include<stdio.h>

int main(void){
	
	long long int input, output = 0;
	
	scanf("%lld", &input);
	
	while(1){
		output = output * 10 + input % 10;
		input = input / 10;
		
		if(input == 0){
			break;
		}
	}
	
	printf("%lld\n", output);
	
	return 0;
}
