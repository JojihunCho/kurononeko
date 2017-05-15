#include<stdio.h>

int main(void){
	
	int input, i, j;
	
	printf("가장 큰 숫자는?(한 자리 권장)\n");
	
	scanf("%d", &input);
	
	for(i = 1; i <= input ; i++){
		for(j = 0; j <= input - i; j++){
			printf(" ");
		}
		for(j = 1; j <= i; j++){
			printf("%d", j);
		}
		for(; j > 1;){
			j = j - 1;
			printf("%d", j);
		}
		printf("\n");
	}
	
	i = i - 1;
	
	for(; i > 0; i--){
		for(j = 0; j <= input - i; j++){
			printf(" ");
		}
		for( j = 1; j <= i; j++){
			printf("%d",j);
		}
		for(; j > 1;){
			j = j - 1;
			printf("%d", j);
		}
		printf("\n");
	}
	return 0;
}
