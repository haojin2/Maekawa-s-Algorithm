#include <vector>
#include <queue>
#include <utility>
#include <unordered_map>
#include <map>
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <string>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "message.cpp"

#define REQUEST 1
#define REPLY 2
#define RELEASE 3

#define RELEASED 0
#define WANTED 1
#define HELD 2

using namespace std;

volatile int option=0;
volatile bool leave=false;
volatile int states[10]={RELEASED};
mutex queue_lock;
int seperate_counts[10]={0};
mutex state_lock[10];
queue<message> channels[10];
int cs_count=0;

void voting_set(int node_id,std::vector<int> & voters)
{
	voters.push_back(node_id);
	switch(node_id)
	{
		case 1:
		{
			voters.push_back(2);
			voters.push_back(3);
			voters.push_back(4);
			voters.push_back(7);
			break;
		}
		case 2:
		{
			voters.push_back(1);
			voters.push_back(3);
			voters.push_back(5);
			voters.push_back(8);
			break;
		}
		case 3:
		{
			voters.push_back(1);
			voters.push_back(2);
			voters.push_back(6);
			voters.push_back(9);
			break;
		}
		case 4:
		{
			voters.push_back(1);
			voters.push_back(5);
			voters.push_back(6);
			voters.push_back(7);
			break;
		}
		case 5:
		{
			voters.push_back(2);
			voters.push_back(4);
			voters.push_back(6);
			voters.push_back(8);
			break;
		}
		case 6:
		{
			voters.push_back(3);
			voters.push_back(4);
			voters.push_back(5);
			voters.push_back(9);
			break;
		}
		case 7:
		{
			voters.push_back(1);
			voters.push_back(4);
			voters.push_back(8);
			voters.push_back(9);
			break;
		}
		case 8:
		{
			voters.push_back(2);
			voters.push_back(5);
			voters.push_back(7);
			voters.push_back(9);
			break;
		}
		case 9:
		{
			voters.push_back(3);
			voters.push_back(6);
			voters.push_back(7);
			voters.push_back(8);
			break;
		}
	}
}

void request_thread(std::pair<int,int> input)
{
	int node_id=input.first;
	int next_req=input.second;
	std::vector<int> voters;
	voting_set(node_id,voters);
	//usleep(rand()%10+1);
	while(1){
		if (leave)
		{
			return;
		}
		state_lock[node_id].lock();
		if (states[node_id]==HELD || states[node_id]==WANTED)
		{
			state_lock[node_id].unlock();
			continue;
		}
		state_lock[node_id].unlock();
		usleep(next_req);
		message m;
		state_lock[node_id].lock();
		states[node_id]=WANTED;
		state_lock[node_id].unlock();
		queue_lock.lock();
		for (int i = 1; i < 5; ++i)
		{
			m.setContent(REQUEST,node_id,voters[i]);
			channels[voters[i]].push(m);
		}
		queue_lock.unlock();
		
	}
}

void node_thread(std::pair<int,int> input)
{
	int node_id=input.first;
	int cs_int=input.second;
	bool voted=false;
	int count=1;
	std::vector<int> voters;
	std::queue<message> requests;
	voting_set(node_id,voters);
	while(1)
	{
		if (leave)
		{
			return;
		}
		//queue_lock.lock();
		//state_lock[node_id].lock();
		if (channels[node_id].empty())
		{
			//state_lock[node_id].unlock();
			//queue_lock.unlock();
			continue;
		}
		message m;
		queue_lock.lock();
		while(!channels[node_id].empty()){
			//state_lock[node_id].lock();
			m=channels[node_id].front();
			channels[node_id].pop();
			//state_lock[node_id].unlock();
			//queue_lock.unlock();
			if (option)
			{
				time_t timestamp=std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
				//print_lock.lock();
				cout<<ctime(&timestamp)<<" "<<node_id<<" "<<m.src;
			}
			switch(m.command)
			{
				case RELEASE:
				{
					if (option)
					{
						cout<<" RELEASE"<<endl;
						//print_lock.unlock();
					}
					if (!requests.empty())
					{
						message tmp1=requests.front();
						requests.pop();
						message tmp2(REPLY,node_id,tmp1.src);
						//queue_lock.lock();
						channels[tmp1.src].push(tmp2);
						//queue_lock.unlock();
						voted=true;
					}
					else
					{
						voted=false;
					}
					break;
				}
				case REPLY:
				{
					if (option)
					{
						cout<<" REPLY"<<endl;
					}
					count++;
					if (count==5)
					{
						count=1;
						state_lock[node_id].lock();
						states[node_id]=HELD;
						state_lock[node_id].unlock();
						cs_count+=1;
						seperate_counts[node_id]+=1;
						time_t timestamp=std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
						cout<<ctime(&timestamp);
						cout<<" "<<node_id<<" ";
						for (int i = 0; i < 5; ++i)
						{
							cout<<voters[i]<<" ";
						}
						cout<<endl;
						//print_lock.unlock();
						queue_lock.unlock();
						usleep(cs_int);
						state_lock[node_id].lock();
						states[node_id]=RELEASED;
						state_lock[node_id].unlock();
						message tmp;
						queue_lock.lock();
						for (int i = 1; i < 5; ++i)
						{
							tmp.setContent(RELEASE,node_id,voters[i]);
							channels[voters[i]].push(tmp);
						}
					}
					/*else
					{
						print_lock.unlock();
					}*/
					break;
				}
				case REQUEST:
				{
					if (option)
					{
						cout<<" REQUEST"<<endl;
						//print_lock.unlock();
					}
					if (states[node_id] == HELD || voted )
					{
						requests.push(m);
					}
					else
					{
						message tmp(REPLY,node_id,m.src);
						//queue_lock.lock();
						channels[m.src].push(tmp);
						//queue_lock.unlock();
						voted=true;
					}
					break;
				}
			}
		}
		queue_lock.unlock();
	}
}

int main(int argc,char** argv)
{
	if (argc!=5)
	{
		cout<<"format: ./mutex <cs_int> <next_req> <total_exec_time> <option>"<<endl;
		return 1;
	}
	srand(time(NULL));
	cs_count=0;
	cout<<"cs_int: "<<stoi(argv[1])<<" next_req: "<<stoi(argv[2])<<" total_exec_time: "<<stoi(argv[3])<<" option: "<<stoi(argv[4])<<endl;
	option=stoi(argv[4]);
	std::vector<thread> node_threads;
	std::vector<thread> request_threads;
	time_t timestamp=std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
	cout<<"starting point: "<< ctime(&timestamp);
	for (int i = 1; i < 10; ++i)
	{
		std::pair<int,int> tmp(i,stoi(argv[1]));
		request_threads.push_back(thread(request_thread,tmp));
	}
	for (int i = 1; i < 10; ++i)
	{
		std::pair<int,int> tmp(i,stoi(argv[2]));
		node_threads.push_back(thread(node_thread,tmp));
	}
	int total_exec_time=stoi(argv[3]);
	
	sleep(total_exec_time);
	leave=true;
	for (int i = 0; i < 9; ++i)
	{
		request_threads[i].join();
		node_threads[i].join();
	}
	timestamp=std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
	cout<<"end point: "<<ctime(&timestamp);
	cout<<"total cs numbers: "<<cs_count<<endl;
	for (int i = 1; i < 10; ++i)
	{
		cout<<"Node "<<i<<" got "<<seperate_counts[i]<<" critical sections.\n";
	}
	return 0;
}