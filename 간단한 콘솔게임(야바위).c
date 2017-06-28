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
	printf("����������������������������������������������������\n");
	printf("��                                                ��\n");
	printf("�� ��  ��  ��    ������    ��   �� �� ����������  ��\n");
	printf("��  ����  ����   ��   ��  ����  �� �� ����        ��\n");
	printf("��   ��  ��  ��  ������  ��  �� �� �� ����������  ��\n");
	printf("��   �� �������� ��   ������������ �� ����        ��\n");
	printf("��   �� ��    �� ������ ��    �� ��  �� ��������  ��\n");
	printf("��                                                ��\n");
	printf("��                              MADE BY Ennisiko  ��\n");
    printf("����������������������������������������������������\n");
	printf("    ��Ģ���� = 1  �����ϱ� = �ƹ�Ű --> ");
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
	printf("����������������������������������������������������\n");
	printf("��  ���� �а����� !! �� ���� �� �Ա� !!!!!        ��\n");
	printf("��  Ȯ���� ���� 1/3 !!!!!!!!!!!!!!!               ��\n");
	printf("��  ó�� ������ coin 500������ �����մϴ�.        ��\n");
	printf("��  coin�� �ּ� 1������ �ִ� ������ ���θ� �� ��  ��\n");
	printf("��  �ֽ��ϴ�. ������ coin�� 0���� �Ǹ� �ڵ�����   ��\n");
	printf("��  ������ ���ᰡ �Ǹ�, ������ 3���Դϴ�.         ��\n");
	printf("��                                                ��\n");
	printf("��                              MADE BY Ennisiko  ��\n");
    printf("����������������������������������������������������\n");
	printf("    �����ϱ� = 1  �ڷΰ��� = �ƹ�Ű --> ");

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
	printf("\n\n�����Ͻ� coin�� �Է��ϼ��� : ");
	scanf_s("%d", &betting);
	printf("\n");
	
	if ((betting >= 1 ) && (betting <= coin)) {
		
		game();
	}else{
		system("cls");
		printf("coin�� Ȯ���� �ּ���.\n���� ����: %d\n", coin);
		
		return insert();
	}
}

int game(void) {
	srand( (unsigned int) time((time_t *)NULL));
	box = (rand() %3) +1;

	printf("1�� 2�� 3�� �� �ƹ��ų� �����ϼ���~");
	scanf_s("%d", &pick);
	system("cls");
	if ((pick < 0 ) || (pick > 4)) {
		printf("\n\n����� ����~!");
		
		return game();
	}else if (box == pick) {
		
			coin= coin + (betting * 2);
			printf("\n\n�Ϳ� ~! ���߼˾��. �������� %d ���� �����Ǿ����ϴ�.\n ���� coin�� %d �� �Դϴ�.\n", betting*2, coin);
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
		printf("\n��Ÿ���׿� ...�����Ͻ� ���� %d��! ������ ���� %d���̿���� ...\n",pick, box);
		coin = coin - betting;
		printf("���� coin�� %d�� �Դϴ�.\n" ,coin);
		
		if (coin ==0) {
			
			system("cls");
			printf("coin�� �� �������׿�. ó������ ���ư��ϴ�.\n");
			
			return main();
		}
		return insert();
	}
}


