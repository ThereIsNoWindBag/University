#include<fstream>
#include<iostream>
#include<thread>
#include<mutex>
#include<Windows.h>
using namespace std;

int buffer[5];//5���� �����Ҽ��ִ� ���� ����
int empty = 5, full = 0;//���ۿ� ����ִ� ����, �� ����
int cnt1 = 0, cnt2 = 0;//���ۿ��� ���� ����/�� ��ġ
int balance = 0;//�ܾ�
int hanging = 0;//������� ���� transaction �Ǵ� deposit
int action = 0;//��� ī����
mutex m;
HANDLE Sem;
ofstream outFile("output.txt");

void producer()
{
   char type;
   int amount;
   ifstream inFile("input.txt");
   while (!inFile.eof())//������ ���������� �ݺ��մϴ�
   {
      inFile >> type;//�� ���ڸ� �о�ɴϴ�(d �Ǵ� c)
      if (type == 'd')
      {
         //���� ���ڰ� d�� ��� ������� wait���·� ����ϴ�
         WaitForSingleObject(Sem, 0);
         if (empty == 0)
         {
            //���۰� deposit ������ �� á�� ��� ������ �� �ִ� �۾��� �����Ƿ� ������� signal���·� ����� �ٽ� �ִ� 0.5�ʸ� ��ٸ� �� wait���·� ����ϴ�
            //���Ŀ��� ���۰� deposit ������ �� á�� ��� ���̻� ������ �۾��� ���� �����̹Ƿ� ���α׷��� ����˴ϴ�.
            ReleaseSemaphore(Sem, 1, NULL);
            WaitForSingleObject(Sem, 500);
         }
         if (empty != 0 && !inFile.eof())
         {
            //���ۿ� �� ������ ���� ��� mutex lock�� �ɾ��� �� deposit ���� ���ۿ� �����մϴ�.
            //���� action ��ȣ�� ��û�� ��, ������� action ������ ��µ˴ϴ�.
            //output.txt���Ͽ��� cmd â�� ��µ� ����� ������ ������ ����˴ϴ�.
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
            //�ٽ� mutex�� lock�� Ǯ�� ������� signal���·� ����ϴ�.
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
   while (!inFile.eof())//������ ���������� �ݺ��մϴ�.
   {
      inFile >> type;//�� ���ڸ� �о�ɴϴ�(d �Ǵ� c)
      if (type == 'c')
      {
         //���� ���ڰ� c�� ��� ������� wait���·� ����ϴ�
         WaitForSingleObject(Sem, 0);
         if (full == 0)
         {
            //���۰� ��� ���� ��� ������ �� �ִ� �۾��� �����Ƿ� ������� signal���·� ����� �ٽ� �ִ� 0.5�ʸ� ��ٸ� �� wait���·� ����ϴ�
            //���Ŀ��� ���۰� ������� ��� ���̻� ������ �۾��� ���� �����̹Ƿ� ���α׷��� ����˴ϴ�.
            ReleaseSemaphore(Sem, 1, NULL);
            WaitForSingleObject(Sem, 500);
         }
         if (full != 0 && !inFile.eof())
         {
            //���ۿ� ���� ���� ��� mutex lock�� �ɾ��� �� ������ ���� balance�� ���մϴ�.
            //���� action ��ȣ�� ��û�� ��, ������� action ������ ��µ˴ϴ�.
            //output.txt���Ͽ��� cmd â�� ��µ� ����� ������ ������ ����˴ϴ�.
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
            //���ۿ� ���� ���� ��� �� �̻� ������ �۾��� �����Ƿ� transaction�� �����ߴٴ� �޽����� ����մϴ�.
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
         //�ٽ� mutex�� lock�� Ǯ�� ������� signal���·� ����ϴ�.
         m.unlock();
         ReleaseSemaphore(Sem, 1, NULL);
      }
      else//�о�� ���� d�ΰ�� �������� �о�� �� �����ϴ�.
         inFile >> trash;
   }
   inFile.close();
}
int  main(){
   //�������� ����
   Sem = CreateSemaphore(NULL, 1, 5, NULL);
   //������ ����
   thread producer(producer);
   thread consumer(consumer);

   producer.join();
   consumer.join();

   outFile.close();
   system("pause");
   return 0;
}