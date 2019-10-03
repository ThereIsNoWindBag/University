#define BUFFER_SIZE 5

#include <windows.h>
#include <process.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <deque>

using namespace std;

HANDLE _producer;
HANDLE _consumer;
HANDLE _full;
HANDLE _empty;
HANDLE _mutex;

deque<int> buffer;
deque<int> deposit;

int transaction{ 0 }, balance{ 0 }, cnt{ 1 };

ifstream openFile;
ofstream writeFile;

stringstream print_status()
{
	stringstream ss;

	ss << "\n<buffer>" << endl;
	ss << "----------------------------------------------" << endl;

	if (!buffer.empty())
	{
		ss << "|";
		for (int i : buffer)
		{
			ss << setw(5) << i << setw(4) << "|";
		}
	}
	ss << "\n----------------------------------------------" << endl;

	if (deposit.size() > 0)
	{
		ss << "<remaining deposits> : " << deposit.size() << endl;
	}

	if (transaction > 0)
	{
		ss << "<remaining transactions> : " << transaction <<  endl;
	}

	ss << endl;

	return ss;
}

void WINAPI attempting_producer(LPVOID para)
{
	int* result = (int*) para;
	DWORD state;

	state = WaitForSingleObject(_empty, 0L);

	if (state != WAIT_OBJECT_0)
	{
		string s = "ERROR : 버퍼가 모두 찬 상태입니다.\n버퍼에 빈 공간이 생기면 이 예금 " + to_string(deposit.back()) + "은(는) 자동으로 버퍼에 들어갑니다.";
		cout << s << endl;
		writeFile << setw(2) << cnt << "번째 시도에서 에러 발생\n" << s << '\n' <<  endl;
		*result = -1;
		_endthread();
	}

	WaitForSingleObject(_mutex, INFINITE);

	cout << "예금 " << deposit.front() << "이(가) 입력되었습니다." << endl;
	buffer.push_back(deposit.front());
	deposit.pop_front();

	ReleaseSemaphore(_mutex, 1, NULL);
	ReleaseSemaphore(_full, 1, NULL);

	*result = 0;
}
void WINAPI attempting_consumer(LPVOID para)
{
	int* result = (int*) para;
	DWORD state;

	state = WaitForSingleObject(_full, 0L);

	if (state != WAIT_OBJECT_0)
	{
		string s = "ERROR : 빈 버퍼에서 인출을 시도했습니다.\n다음 예금이 들어오면 그 예금은 자동으로 인출됩니다.";
		cout << s << endl;
		writeFile << setw(2) << cnt << "번째 시도에서 에러 발생\n" << s << '\n' << endl;
		*result = -1;
		_endthread();
	}

	WaitForSingleObject(_mutex, INFINITE);

	balance += buffer.front();
	cout << buffer.front() << "이(가) 인출되었습니다." << endl;
	buffer.pop_front();
	transaction--;

	ReleaseSemaphore(_mutex, 1, NULL);
	ReleaseSemaphore(_empty, 1, NULL);

	*result = 0;
}

void WINAPI waiting_producer(LPVOID para)
{
	WaitForSingleObject(_empty, INFINITE);
	WaitForSingleObject(_mutex, INFINITE);

	buffer.push_back(deposit.front());
	cout << "대기중이던 예금 " << deposit.front() << "이(가) 입력되었습니다." << endl;
	deposit.pop_front();

	ReleaseSemaphore(_mutex, 1, NULL);
	ReleaseSemaphore(_full, 1, NULL);
}
void WINAPI waiting_consumer(LPVOID para)
{
	WaitForSingleObject(_full, INFINITE);
	WaitForSingleObject(_mutex, INFINITE);

	cout << "대기중이던 인출이 실행되었습니다.\n" << buffer.front() << "이(가) 인출되었습니다." << endl;
	balance += buffer.front();
	buffer.pop_front();
	transaction--;

	ReleaseSemaphore(_mutex, 1, NULL);
	ReleaseSemaphore(_empty, 1, NULL);
}

int insert_item(int item)
{
	int result = -1;
	deposit.push_back(item);
	cout << item << "을(를) 예금 시도합니다." << endl;
	WaitForSingleObject((HANDLE)_beginthread(attempting_producer, 0, (LPVOID)&result), INFINITE);
	if (result)
	{
		_beginthread(waiting_producer, 0, NULL);
	}
	return result;
}
int remove_item()
{
	int result = -1;
	transaction++;
	cout << "인출을 시도합니다." << endl;
	WaitForSingleObject((HANDLE)_beginthread(attempting_consumer, 0, (LPVOID)&result), INFINITE);
	if (result)
	{
		_beginthread(waiting_consumer, 0, NULL);
	}
	return result;
}

int main(int argc, char *argv[])
{
	string inputPath = argv[1]; // "text.txt";
	string logPath = argv[2]; //"log.txt"

	_full = CreateSemaphore(NULL, 0, BUFFER_SIZE, NULL);
	_empty = CreateSemaphore(NULL, BUFFER_SIZE, BUFFER_SIZE, NULL);
	_mutex = CreateSemaphore(NULL, 1, 1, NULL);

	writeFile.open(logPath);
	if (writeFile.is_open())
	{
		writeFile << "<<에러메세지>>\n" << endl;

		openFile.open(inputPath);
		if (openFile.is_open())
		{
			int item;
			char c = ' ';

			while (true)
			{
				openFile >> c;
				if (openFile.eof()) break;

				cout << "[" << setw(2) << cnt << "회 시도]\n" << endl;
				switch (c)
				{
				case 'd':
					openFile >> item;
					insert_item(item);
					break;

				case 'c':
					remove_item();
					break;
				default:
					cnt--;
					break;
				}
				WaitForSingleObject(_mutex, INFINITE);
				cout << print_status().str();
				ReleaseSemaphore(_mutex, 1, NULL);
				cnt++;
			}
			cout << "총 계좌 금액 : " << balance << "$" << endl;
			openFile.close();
		}
		writeFile << "<<Brokerage 내부 상태>>" << endl;
		writeFile << print_status().str();

		writeFile << "<<총 계좌 내 금액 (Balance in the saving)>>" << endl;
		writeFile << "$" << balance;

		writeFile.close();
	}

	CloseHandle(_full);
	CloseHandle(_empty);
	CloseHandle(_mutex);

	std::system("pause");
	return 0;
}