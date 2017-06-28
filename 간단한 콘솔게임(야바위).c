#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int rule(void);
int insert(void);
int game(void);
int coin;
static int betting;
static int box;
static int pick;

int main(void){
	int first;
	coin = 500;
	printf("┏━━━━━━━━━━━━━━━━━━━━━━━━┓\n");
	printf("┃                                                ┃\n");
	printf("┃ ┃  ┃  ┃    ┏━┓    ┃   ┃ ┃ ┃┏━━━  ┃\n");
	printf("┃  ┃┃  ┃┃   ┃   ┃  ┃┃  ┃ ┃ ┃┃        ┃\n");
	printf("┃   ┃  ┃  ┃  ┣━┫  ┃  ┃ ┃ ┃ ┃┣━━━  ┃\n");
	printf("┃   ┃ ┣━━┫ ┃   ┃┣━━┫┃ ┃ ┃┃        ┃\n");
	printf("┃   ┃ ┃    ┃ ┗━┛ ┃    ┃ ┃  ┃ ┗━━━  ┃\n");
	printf("┃                                                ┃\n");
	printf("┃                              MADE BY Ennisiko  ┃\n");
    printf("┗━━━━━━━━━━━━━━━━━━━━━━━━┛\n");
	printf("    규칙보기 = 1  종료하기 = 아무키 --> ");
	scanf_s("%d", &first);

	if (first == 1) {
		system("cls");
		rule();
	}else{
		return 0;
	}
}
	
int rule(void) {
	int second;
	printf("┏━━━━━━━━━━━━━━━━━━━━━━━━┓\n");
	printf("┃  본격 패가망신 !! 돈 놓고 돈 먹기 !!!!!        ┃\n");
	printf("┃  확률은 무려 1/3 !!!!!!!!!!!!!!!               ┃\n");
	printf("┃  처음 시작은 coin 500점으로 시작합니다.        ┃\n");
	printf("┃  coin은 최소 1점부터 최대 보유액 전부를 걸 수  ┃\n");
	printf("┃  있습니다. 보유한 coin이 0원이 되면 자동으로   ┃\n");
	printf("┃  게임이 종료가 되며, 배율은 3배입니다.         ┃\n");
	printf("┃                                                ┃\n");
	printf("┃                              MADE BY Ennisiko  ┃\n");
    printf("┗━━━━━━━━━━━━━━━━━━━━━━━━┛\n");
	printf("    시작하기 = 1  뒤로가기 = 아무키 --> ");

	scanf_s("%d", &second);
	if (second == 1) {
		system("cls");
		insert();
	}else{
		system("cls");
		main();
	}
}

int insert(void) {
	printf("\n\n배팅하실 coin을 입력하세요 : ");
	scanf_s("%d", &betting);
	printf("\n");
	
	if ((betting >= 1 ) && (betting <= coin)) {
		
		game();
	}else{
		system("cls");
		printf("coin을 확인해 주세요.\n남은 코인: %d\n", coin);
		
		return insert();
	}
}

int game(void) {
	srand( (unsigned int) time((time_t *)NULL));
	box = (rand() %3) +1;

	printf("1번 2번 3번 중 아무거나 선택하세요~");
	scanf_s("%d", &pick);
	system("cls");
	if ((pick < 0 ) || (pick > 4)) {
		printf("\n\n제대로 골라요~!");
		
		return game();
	}else if (box == pick) {
		
			coin= coin + (betting * 2);
			printf("\n\n와우 ~! 맞추셧어요. 보상으로 %d 원이 적립되었습니다.\n 현재 coin은 %d 원 입니다.\n", betting*2, coin);
			return insert();
	}else {
		if (box > 3) {
			if (pick == 1){
				box = 2;
			}else if (pick == 2){
				box = 3;
			}else if (pick == 3){
				box = 1;
			}
		}
		printf("\n안타깝네요 ...선택하신 답은 %d번! 하지만 답은 %d번이였어요 ...\n",pick, box);
		coin = coin - betting;
		printf("남은 coin은 %d원 입니다.\n" ,coin);
		
		if (coin ==0) {
			
			system("cls");
			printf("coin이 다 떨어졌네요. 처음으로 돌아갑니다.\n");
			
			return main();
		}
		return insert();
	}
}


