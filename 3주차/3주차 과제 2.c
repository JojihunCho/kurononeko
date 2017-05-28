#include<stdio.h>
#include<stdlib.h>

void addArr(int **arr, int *index) {
	int *temp, newIndex, i;
	newIndex = (*index) + 3;
	temp = (int*)malloc(sizeof(int) * newIndex);
	for (i = 0; i < (*index); i++){
		temp[i] = (*arr)[i];
	}
	free(*arr);
	*arr = temp;
	*index = newIndex;
}

int main(void) {
	int index = 5, *arry, k = 0, i;
	arry = (int*)malloc(sizeof(int) * index);
	
	while(1){
		scanf("%d", &arry[k]);
		if (arry[k] == -1){
			break;
		}
		if (k == index - 1) {
			addArr(&arry, &index);
		}
		k++;
	}
	for(i = 0; i <= k; i++){
		printf("%d", arry[i]);
	}
	return 0;
}
