#pragma once
#include <mutex> //锁
#include <queue>
#include <iostream>
using namespace std;

template <typename T>
class SafeQueue{
	private:
		queue<T> m_queue; //队列
		mutex m_mutex; //锁
	
	public:
		SafeQueue(){} //空构造函数
		SafeQueue(SafeQueue& other){ //拷贝构造函数 
			m_queue = other.mqueue;
		}
		~SafeQueue(){} //析构函数 
//		查看操作 
		bool empty(){
			unique_lock<mutex> lock(m_mutex); //智能锁，离开作用域后自动解锁
			return m_queue.empty();
		}
		int size(){
			unique_lock<mutex> lock(m_mutex);
			return m_queue.size();
		}
//		修改操作 
		void enqueue(T& t){
			unique_lock<mutex> lock(m_mutex);
			cout << t << endl;
			m_queue.push(t);
		}
		bool dequeue(T &t){
			unique_lock<mutex> lock(m_mutex);
			if(m_queue.empty()){
				return false;
			}
			t = m_queue.front();
			cout << t << endl;
			m_queue.pop();
			return true;
		}
};
