#include<stdio.h>

int main(void){
	char input;
	
	scanf("%c", &input);
	
	if(input >= '0' && input <= '9'){
		printf("����");
	}else if(input >= 'A' && input <= 'Z'){
		printf("�빮��");
	}else if(input >= 'a' && input <= 'z'){
		printf("�ҹ���");
	}else{
		printf("�߸��� �Է��Դϴ�.");
	}
	return 0;
}
