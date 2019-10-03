#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>

using namespace std;

//상수 선언부
//CPU burst 최대 20 + I/O burst 최대 19 + pid + arrival time + priority == 42
constexpr int FOURTY_TWO = 42;
//RR time quantum == 10
constexpr int TIME_QUANTUM = 10;

//PCB에 쓰일 state 열거형
enum state { hold, ready, running, waiting, terminated };

class PCB
{
public:
	state state;

	int pid;
	int arrivalTime;
	int priority;

	int waitingTime = 0;
	int remainingTime = 0;

	//CPU, I/O burst값이 들어갈 vector와 이터레이터
	vector<int> v;
	vector<int>::iterator iter;

	PCB(int arr[FOURTY_TWO])
	{
		//프로세스 상태 초기화
		state = hold;
		//멤버 변수 값 초기화
		pid = arr[0];
		arrivalTime = arr[1];
		priority = arr[2];

		for (int i = 3; i < FOURTY_TWO; ++i)
		{
			if (arr[i] == NULL)
				break;
			v.push_back(arr[i]);
		}
		//이터레이터 초기화
		iter = v.begin();
		remainingTime = *iter;
	}
};

//p[]의 모든 프로세스가 NULL(=terminated)일때 false 반환
bool check(PCB**);

//원본 데이터 값을 이용해 p를 다시 할당
void PCBInit(int**);

//FIFO와 RR에서 쓰임
bool compare(PCB*, PCB*);
bool compareSJF(PCB*, PCB*);
bool comparePR(PCB*, PCB*);

void move(deque<PCB*>&);
void moveWithRR(deque<PCB*>&, int q);

void allocate();
void allocateWithRR(int& q);

void progress();
void progressWithRR(int& q);

void print();

//PCB를 담을 readyQueue
deque<PCB*> readyQueue;

//동적으로 프로세스를 받을 p변수
PCB **p = NULL;

//waiting time과 turnaround time의 결과값이 저장될 string
string *s;

//현재 CPU가 새로 할당 가능한 상태인지 알려주는 bool값
bool isCPUFree = true;

unsigned int time = 0;
int pnum = 0;

int main()
{
	//같은 데이터 값이 총 4번 쓰이기 때문에, 값을 따로 저장할 필요가 있다.
	#pragma region ORIGINAL
	int tmp;
	cin >> pnum;

	//데이터 값의 원본이 저장되는 더블포인터 변수
	int **process = new int*[pnum];

	for (int i = 0; i < pnum; ++i)
	{
		process[i] = new int[FOURTY_TWO];

		for (int j = 0; j < FOURTY_TWO; ++j)
		{
			process[i][j] = NULL;
		}
	}

	for (int i = 0; i < pnum; ++i)
	{
		process[i][0] = i; // pid
		cin >> tmp;
		process[i][1] = tmp; // arrivalTime
		cin >> tmp;
		process[i][2] = tmp; // priority
		for (int j = 3; j < FOURTY_TWO; ++j) // get bursts
		{
			cin >> tmp;
			process[i][j] = tmp;
			if (getc(stdin) != ' ')
				break;
		}
	}
	#pragma endregion

	//FIFO와 RR방식에서는 같은 time에 ReadyQueue에 도착하는 프로세스를 다시 pid순으로 정렬해야하기 때문에 readyQueue에 가기 전 임시로 이 deque t에 저장된다. 
	deque<PCB*> t;

	#pragma region FIFO
	cout << "(FIFO)\n";
	PCBInit(process);
	while (check(p))
	{
		//자리변경
		move(t);

		sort(t.begin(), t.end(), compare);
		unsigned int t_size = t.size();
		for (unsigned int i = 0; i < t_size; ++i)
		{
			readyQueue.push_front(t.back());
			t.pop_back();
		}

		allocate();
		progress();
		time++;
	}
	print();
	#pragma endregion

	#pragma region SJF
	cout << "(SJF)\n";
	PCBInit(process);
	while (check(p))
	{
		//자리변경
		move(readyQueue);

		//t 벡터 정렬 및 전송
		sort(readyQueue.begin(), readyQueue.end(), compareSJF);

		//running 할당 & 정산
		allocate();
		progress();

		time++;
	}
	print();
	#pragma endregion

	#pragma region PR
	cout << "(PRIORITY)\n";
	PCBInit(process);
	while (check(p))
	{
		//자리변경
		move(readyQueue);

		//t 벡터 정렬 및 전송
		sort(readyQueue.begin(), readyQueue.end(), comparePR);

		//running 할당
		allocate();

		//정산
		progress();
		time++;
	}
	print();
	#pragma endregion

	#pragma region RR
	cout << "(RR)\n";
	PCBInit(process);

	t.clear();
	int q = TIME_QUANTUM;

	while (check(p))
	{
		//자리변경
		moveWithRR(t, q);

		//t 벡터 정렬 및 전송
		sort(t.begin(), t.end(), compare);
		unsigned int t_size = t.size();
		for (unsigned int i = 0; i < t_size; ++i)
		{
			readyQueue.push_front(t.back());
			t.pop_back();
		}

		//running 할당
		allocateWithRR(q);
		progressWithRR(q);

		time++;
	}
	print();
	#pragma endregion
	getchar();
	return 0;
}

bool check(PCB **p)
{
	for (int i = 0; i < pnum; ++i)
	{
		if (p[i] != NULL)
			return true;
	}
	return false;
}

void PCBInit(int** process)
{
	if (p != NULL)
	{
		for (int i = 0; i < pnum; ++i)
		{
			delete p[i];
			p[i] = NULL;
		}
		delete p;
		p = NULL;
	}

	p = new PCB*[pnum];
	for (int i = 0; i < pnum; ++i)
	{
		p[i] = new PCB(process[i]);
	}

	time = 0;
	isCPUFree = true;

	delete[] s;
	s = new string[pnum];
}

bool compare(PCB *a, PCB *b)
{
	return a->pid > b->pid;
}
bool compareSJF(PCB *a, PCB *b)
{
	if (a->remainingTime == b->remainingTime)
		return a->pid > b->pid;
	else
		return a->remainingTime > b->remainingTime;
}
bool comparePR(PCB *a, PCB *b)
{
	if (a->priority == b->priority)
		return a->pid > b->pid;
	else
		return a->priority > b->priority;
}

void move(deque<PCB*>& t)
{
	for (int i = 0; i < pnum; ++i)
	{
		if (p[i] == NULL) continue;
		switch (p[i]->state)
		{
		case hold:
			//to ready
			if (p[i]->arrivalTime == time)
			{
				p[i]->state = ready;
				t.push_front(p[i]);
			}
			break;
		case waiting:
			//to ready
			if (p[i]->remainingTime == 0)
			{
				p[i]->iter++;
				p[i]->remainingTime = *(p[i]->iter);
				p[i]->state = ready;
				t.push_front(p[i]);
			}
			break;
		case running:
			if (p[i]->remainingTime == 0)
			{
				p[i]->iter++;
				//to terminated
				if (p[i]->v.end() == p[i]->iter)
				{
					p[i]->state = terminated;
				}
				//to waiting
				else
				{
					p[i]->remainingTime = *(p[i]->iter);
					p[i]->state = waiting;
				}
				isCPUFree = true;
			}
			break;
		}
	}
}
void allocate()
{
	if (isCPUFree && !readyQueue.empty())
	{
		readyQueue.back()->state = running;
		cout << time << ' ' << readyQueue.back()->pid << '\n';
		readyQueue.pop_back();
		isCPUFree = false;
	}
}
void progress()
{
	for (int i = 0; i < pnum; ++i)
	{
		if (p[i] == NULL)
			continue;
		if (p[i]->state == ready)
		{
			p[i]->waitingTime++;
		}
		else if (p[i]->state == waiting || p[i]->state == running)
		{
			p[i]->remainingTime--;
		}
		else if (p[i]->state == terminated)
		{
			s[i] = "process " + to_string(p[i]->pid) + " waiting time : " + to_string(p[i]->waitingTime) + " turnaround time : " + to_string(time - p[i]->arrivalTime) + '\n';
			delete p[i];
			p[i] = NULL;
		}
	}
}

void moveWithRR(deque<PCB*>& t, int q)
{
	for (int i = 0; i < pnum; ++i)
	{
		if (p[i] == NULL) continue;
		switch (p[i]->state)
		{
		case hold:
			//to ready
			if (p[i]->arrivalTime == time)
			{
				p[i]->state = ready;
				t.push_front(p[i]);
			}
			break;
		case waiting:
			//to ready
			if (p[i]->remainingTime == 0)
			{
				p[i]->iter++;
				p[i]->remainingTime = *(p[i]->iter);
				p[i]->state = ready;
				t.push_front(p[i]);
			}
			break;
		case running:
			if (p[i]->remainingTime == 0)
			{
				p[i]->iter++;
				//to terminated
				if (p[i]->v.end() == p[i]->iter)
				{
					p[i]->state = terminated;
				}
				//to waiting
				else
				{
					p[i]->remainingTime = *(p[i]->iter);
					p[i]->state = waiting;
				}
				isCPUFree = true;
			}
			//while relinquish(running to ready)
			else if (q == 0)
			{
				p[i]->state = ready;
				t.push_front(p[i]);
				isCPUFree = true;
			}
			break;
		}
	}
}
void allocateWithRR(int& q)
{
	if (isCPUFree && !readyQueue.empty())
	{
		readyQueue.back()->state = running;
		cout << time << ' ' << readyQueue.back()->pid << '\n';
		readyQueue.pop_back();
		isCPUFree = false;
		q = TIME_QUANTUM;
	}
}
void progressWithRR(int& q)
{
	for (int i = 0; i < pnum; ++i)
	{
		if (p[i] == NULL)
			continue;
		if (p[i]->state == ready)
		{
			p[i]->waitingTime++;
		}
		else if (p[i]->state == waiting)
		{
			p[i]->remainingTime--;
		}
		else if (p[i]->state == running)
		{
			p[i]->remainingTime--;
			q--;
		}
		else if (p[i]->state == terminated)
		{
			s[i] = "process " + to_string(p[i]->pid) + " waiting time : " + to_string(p[i]->waitingTime) + " turnaround time : " + to_string(time - p[i]->arrivalTime) + '\n';
			delete p[i];
			p[i] = NULL;
		}
	}
}

void print()
{
	for (int i = 0; i < pnum; ++i)
	{
		cout << s[i];
	}
}