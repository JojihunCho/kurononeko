#include<stdio.h>

int main() {
	int input, i, j, temp;
	int even = 0, odd = 9;
	int arr[10] = {};
	
	for (i = 0; i < 10; i++) {
		printf("[%d] Number?\t", i + 1);
		scanf("%d", &input);
		
		if (input % 2 == 0) {
			arr[even] = input;
			even++;
		}else{
			arr[odd] = input;
			odd--;
		}
	}
	for (i = 0; i < 10; i++) {
		printf("[%d] ", arr[i]);
	}
	
	printf("\n");
	
	for (i = 9; i > 0; i--) {
		for (j = 0; j < i; j++) {
			if (arr[j] >= arr[j + 1]) {
				temp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = temp;
			}
		}
	}
	
	for (i = 0; i < 10; i++) {
		printf("[%d] ", arr[i]);
	}
	
	return 0;
}
