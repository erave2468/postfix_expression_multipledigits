#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#define MALLOC(p,s)\
	if (!((p) = malloc(s))) {\
		fprintf(stderr, "Insufficient memory");\
		exit(EXIT_FAILURE);\
	}

char* input; //입력받을 문자열
char output[100]; //출력할 문자열
typedef enum precedence { lparen, rparen, plus, minus, times, divide, mod, eos, operand, ten, endnum }precedence;
int isp[] = { 0, 19, 12, 12, 13, 13, 13, 0 };
int icp[] = { 20, 0, 12, 12, 13, 13, 13, 0 };

typedef struct element {
	precedence type;
	int num;
} element;
typedef struct node* nodeLink;
typedef struct node {
	element data;
	int key;
	nodeLink link;
} stack;
nodeLink top[10]; //0번 변환용, 1번 계산용
element queue[100]; //큐를 위한 배열
int rear, front;
void addq(element item) {
	rear = (rear + 1) % 100;
	if (front == rear) {
		fprintf(stderr, "Queue is full\n");
		exit(EXIT_FAILURE);
	}
	queue[rear] = item;
}
element deleteq() {
	element item;
	if (front == rear) {
		return (element) { endnum, -1 };
	}
	front = (front + 1) % 100;
	return queue[front];
}
void push(element item, int n) { //스택 삽입 함수
	nodeLink temp;
	MALLOC(temp, sizeof(*temp));
	temp->data = item;
	temp->link = top[n];
	top[n] = temp; // 새 노드를 스택의 맨 위에 추가
}
element pop(int n) {
	element item;
	nodeLink temp = top[n]; //꺼낼 노드는 스택 맨 위 노드
	if (!temp) {
		return (element) { endnum, -1 }; // 스택이 비었으면 
	}
	item = top[n]->data;
	top[n] = top[n]->link; // 스택의 맨 위 노드를 다음 노드로 변경
	free(temp); //꺼낸 노드 메모리 해제
	return item;
}
precedence getToken(char* symbol, int* n) { //문자열을 토큰으로 변환
	*symbol = input[(*n)++];
	switch (*symbol) {
	case '(': return lparen;
	case ')': return rparen;
	case '+': return plus;
	case '-': return minus;
	case '*': return times;
	case '/': return divide;
	case '%': return mod;
	case 't': return ten; // 't'는 10을 나타내는 토큰
	case 'd': return endnum; // 'd'는 숫자의 종료를 나타내는 토큰
	case '\0': return eos; //문자열 끝
	default: return operand; //피연산자
	};
}
char printToken(precedence token) { //토큰을 문자로 변환
	switch (token) {
	case lparen: return '(';
	case rparen: return ')';
	case plus: return '+';
	case minus: return '-';
	case times: return '*';
	case divide: return '/';
	case mod: return '%';
	case eos: return '\0'; //문자열 끝
	default: return 'e'; //에러
	}
}
void postfix() {
	char symbol;
	precedence token;
	push((element) { eos, 0 }, 0); // 스택에 eos 삽입
	int n = 0; // 입력 문자열의 인덱스
	int i = 0; // 출력 문자열의 인덱스
	element temp;
	for (token = getToken(&symbol, &n); token != eos; token = getToken(&symbol, &n)) {
		if (token == operand) { //피연산자면
			temp = (element){ operand, symbol - '0' }; //피연산자 원소 생성
			addq(temp); //큐에 삽입
		}
		else if (token == rparen) { //오른쪽 괄호면
			while (front != rear) { //큐에 있는 피연산자들을 출력 문자열에 삽입
				output[i++] = deleteq().num + '0'; //문자열에 삽입
				for (int k = (rear - front + 100) % 100;k > 0;k--)//2개 이상 쌓여있으면 t삽입
					output[i++] = 't';
			}
			output[i++] = 'd'; //한개의 숫자 종료
			while ((top[0]->data.type != lparen) && (top[0]->data.type != eos)) {
				output[i++] = printToken(top[0]->data.type); //스택에서 꺼내서 출력 문자열에 삽입
				pop(0);
			}
			pop(0); //왼쪽 괄호는 출력하지 않음
		}
		else { //연산자면
			while (front != rear) {
				output[i++] = deleteq().num + '0';
				for (int k = (rear - front + 100) % 100;k > 0;k--)
					output[i++] = 't';
			}
			output[i++] = 'd';
			while (isp[top[0]->data.type] >= icp[token]) { //우선순위가 크거나 같으면
				output[i++] = printToken(top[0]->data.type); //스택에서 꺼내서 출력 문자열에 삽입
				pop(0);
			}
			push((element) { token, symbol }, 0); //스택에 삽입
		}
	}
	while (front != rear) { //큐에 남아있는 피연산자들을 출력 문자열에 삽입
		output[i++] = deleteq().num + '0';
		for (int k = (rear - front + 100) % 100;k > 0;k--)
			output[i++] = 't';
	}
	output[i++] = 'd';
	while (top[0]->data.type != eos) { //스택에 남아있는 연산자들을 출력 문자열에 삽입
		output[i++] = printToken(top[0]->data.type);
		pop(0); //스택에서 꺼내기
	}
	if (i > 0 && output[i - 1] != '\0') { //출력 문자열이 비어있지 않으면
		output[i++] = '\0'; //문자열 끝에 널 문자 추가
	}
}
int eval() {
	precedence token;
	char symbol;
	element op1, op2;
	int n = 0, tempnum = 0;
	int stackRem = 0;//하나의 수로 만들 스택의 높이
	input = output;
	token = getToken(&symbol, &n);
	while (token != eos) {
		if (token == operand) { //피연산자면
			push((element) { operand, symbol - '0' }, 1); //스택에 삽입
			stackRem++;
		}
		else if (token == ten) {
			top[1]->data.num *= 10; // 't'는 *10을 나타내므로 스택에 10을 곱함
		}
		else if (token == endnum) {
			for (;stackRem > 0;stackRem--) {
				tempnum += pop(1).num; //스택에 있는 숫자를 모두 더함
			}
			push((element) { operand, tempnum }, 1); //스택에 결과 삽입
			tempnum = 0; //다음 숫자를 위해 초기화
		}
		else { //연산자면
			op2.num = pop(1).num;
			op1.num = pop(1).num; //스택에서 두 개의 피연산자 꺼내기
			switch (token) {
			case plus: //덧셈
				push((element) { operand, op1.num + op2.num }, 1);
				break;
			case minus: //뺄셈
				push((element) { operand, op1.num - op2.num }, 1);
				break;
			case times: //곱셈
				push((element) { operand, op1.num* op2.num }, 1);
				break;
			case divide: //나눗셈
				if (op2.num == 0) { //0으로 나누기 에러 처리
					fprintf(stderr, "0으로 나눌 수 없습니다.\n");
					exit(EXIT_FAILURE);
				}
				push((element) { operand, op1.num / op2.num }, 1);
				break;
			case mod: //나머지
				if (op2.num == 0) { //0으로 나누기 에러 처리
					fprintf(stderr, "0으로 나눌 수 없습니다.\n");
					exit(EXIT_FAILURE);
				}
				push((element) { operand, op1.num% op2.num }, 1);
				break;
			}
		}
		token = getToken(&symbol, &n); //다음 토큰 읽기
	}
	return pop(1).num; //스택에서 최종 결과 반환
}
int main() {
	precedence one;
	char symbol;
	char put[100];

	while (1) {
		printf("후위 표기법으로 변환할 수식을 입력하세요 (종료하려면 'exit' 입력): ");
		scanf_s("%s", put, 100); //문자열 입력
		if (strcmp(put, "exit") == 0) {
			break; // 'exit' 입력 시 종료
		}
		input = put; //입력 문자열 설정
		postfix(); //후위 표기법으로 변환
		printf("후위 표기법: ");
		printf("%s\n", output); //출력 문자열 출력
		int result = eval(); //후위 표기법 계산
		printf("결과: %d\n", result); //결과 출력

	}

}
