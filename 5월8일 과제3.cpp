#include<stdio.h>

int main(void){
	char input;
	
	scanf("%c", &input);
	
	if(input >= '0' && input <= '9'){
		printf("숫자");
	}else if(input >= 'A' && input <= 'Z'){
		printf("대문자");
	}else if(input >= 'a' && input <= 'z'){
		printf("소문자");
	}else{
		printf("잘못된 입력입니다.");
	}
	return 0;
}
