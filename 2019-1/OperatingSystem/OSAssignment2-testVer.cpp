#include<fstream>
#include<iostream>
#include<thread>
#include<mutex>
#include<Windows.h>
using namespace std;

int buffer[5];//5개를 저장할수있는 버퍼 생성
int empty = 5, full = 0;//버퍼에 들어있는 개수, 빈 개수
int cnt1 = 0, cnt2 = 0;//버퍼에서 값을 읽을/쓸 위치
int balance = 0;//잔액
int hanging = 0;//실행되지 않은 transaction 또는 deposit
int action = 0;//명령 카운터
mutex m;
HANDLE Sem;
ofstream outFile("output.txt");

void producer()
{
   char type;
   int amount;
   ifstream inFile("input.txt");
   while (!inFile.eof())//파일이 끝날때까지 반복합니다
   {
      inFile >> type;//한 글자를 읽어옵니다(d 또는 c)
      if (type == 'd')
      {
         //읽은 글자가 d인 경우 세마포어를 wait상태로 만듭니다
         WaitForSingleObject(Sem, 0);
         if (empty == 0)
         {
            //버퍼가 deposit 값으로 꽉 찼을 경우 수행할 수 있는 작업이 없으므로 세마포어를 signal상태로 만들고 다시 최대 0.5초를 기다린 후 wait상태로 만듭니다
            //이후에도 버퍼가 deposit 값으로 꽉 찼을 경우 더이상 수행할 작업이 없는 상태이므로 프로그램이 종료됩니다.
            ReleaseSemaphore(Sem, 1, NULL);
            WaitForSingleObject(Sem, 500);
         }
         if (empty != 0 && !inFile.eof())
         {
            //버퍼에 빈 공간이 있을 경우 mutex lock을 걸어준 후 deposit 값을 버퍼에 저장합니다.
            //현재 action 번호와 요청한 값, 대기중인 action 갯수가 출력됩니다.
            //output.txt파일에는 cmd 창에 출력된 내용과 동일한 내용이 저장됩니다.
            m.lock();
            inFile >> amount;
            cout << "(action " << ++action << ")" << endl << "(deposit request) : " << amount << endl << "(hanging actions) : ";
            outFile << "(action " << action << ")" << endl << "(deposit request) : " << amount << endl << "(hanging actions) : ";
            hanging++;
            if (hanging >= 0)
            {
               cout << hanging << endl << endl;
               outFile << hanging << endl << endl;
            }
            else
            {
               cout << -hanging << endl << endl;
               outFile << -hanging << endl << endl;
            }
            buffer[cnt1++ % 5] = amount;
            empty--;
            full++;
            //다시 mutex의 lock을 풀고 세마포어를 signal상태로 만듭니다.
            m.unlock();
            ReleaseSemaphore(Sem, 1, NULL);
         }
      }
   }
   inFile.close();
}
void consumer()
{
   char type;
   int trash, check = 0;

   ifstream inFile("input.txt");
   while (!inFile.eof())//파일이 끝날때까지 반복합니다.
   {
      inFile >> type;//한 글자를 읽어옵니다(d 또는 c)
      if (type == 'c')
      {
         //읽은 글자가 c인 경우 세마포어를 wait상태로 만듭니다
         WaitForSingleObject(Sem, 0);
         if (full == 0)
         {
            //버퍼가 비어 있을 경우 수행할 수 있는 작업이 없으므로 세마포어를 signal상태로 만들고 다시 최대 0.5초를 기다린 후 wait상태로 만듭니다
            //이후에도 버퍼가 비어있을 경우 더이상 수행할 작업이 없는 상태이므로 프로그램이 종료됩니다.
            ReleaseSemaphore(Sem, 1, NULL);
            WaitForSingleObject(Sem, 500);
         }
         if (full != 0 && !inFile.eof())
         {
            //버퍼에 값이 있을 경우 mutex lock을 걸어준 후 버퍼의 값을 balance에 더합니다.
            //현재 action 번호와 요청한 값, 대기중인 action 갯수가 출력됩니다.
            //output.txt파일에는 cmd 창에 출력된 내용과 동일한 내용이 저장됩니다.
            m.lock();
            cout << "(action " << ++action << ")" << endl << "(transaction) : " << buffer[cnt2 % 5] << endl;
            outFile << "(action " << action << ")" << endl << "(transaction) : " << buffer[cnt2 % 5] << endl;
            balance += buffer[cnt2++ % 5];
            cout << "(balance) : " << balance << endl << "(hanging actions) : ";
            outFile << "(balance) : " << balance << endl << "(hanging actions) : ";
            hanging--;
            empty++;
            full--;
            if (hanging >= 0)
            {
               cout << hanging << endl << endl;
               outFile << hanging << endl << endl;
            }
            else
            {
               cout << -hanging << endl << endl;
               outFile << -hanging << endl << endl;
            }
         }
         else if (full == 0)
         {
            //버퍼에 값이 없을 경우 더 이상 수행할 작업이 없으므로 transaction이 실패했다는 메시지를 출력합니다.
            m.lock();
            if (!inFile.eof() && hanging > -5)
            {
               cout << "(action " << ++action << ")" << endl << "(transaction failed)" << endl << "(balance) : " << balance << endl << "hanging : " << hanging << endl << "(hanging actions) : ";
               outFile << "(action " << action << ")" << endl << "(transaction failed)" << endl << "(balance) : " << balance << endl << "hanging : " << hanging << endl << "(hanging actions) : ";

               hanging--;
               if (hanging >= 0)
               {
                  cout << hanging << endl << endl;
                  outFile << hanging << endl << endl;
               }
               else
               {
                  cout << -hanging << endl << endl;
                  outFile << -hanging << endl << endl;
               }
            }
         }
         //다시 mutex의 lock을 풀고 세마포어를 signal상태로 만듭니다.
         m.unlock();
         ReleaseSemaphore(Sem, 1, NULL);
      }
      else//읽어온 값이 d인경우 정수값을 읽어온 후 버립니다.
         inFile >> trash;
   }
   inFile.close();
}
int  main(){
   //세마포어 생성
   Sem = CreateSemaphore(NULL, 1, 5, NULL);
   //쓰레드 생성
   thread producer(producer);
   thread consumer(consumer);

   producer.join();
   consumer.join();

   outFile.close();
   system("pause");
   return 0;
}