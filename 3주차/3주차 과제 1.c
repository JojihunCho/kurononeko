#include<stdio.h>
#include<stdlib.h>

int main(void) {
	int **arr, i, j, input;
	
	scanf("%d", &input);
	
	arr = (int**)malloc(sizeof(int*) * input);
	
	for (i = 0; i < input; i++) {
		arr[i] = (int*)malloc(sizeof(int) * (i + 1));
	}
	
	for (i = 0; i < input; i++) {
		for (j = 0; j <= i; j++) {
			arr[i][j] = 0;
			printf("%d", arr[i][j]);
		}
		printf("\n");
	}
	
	return 0;
}
