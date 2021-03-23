#pragma once
#include <mutex> //��
#include <queue>
#include <iostream>
using namespace std;

template <typename T>
class SafeQueue{
	private:
		queue<T> m_queue; //����
		mutex m_mutex; //��
	
	public:
		SafeQueue(){} //�չ��캯��
		SafeQueue(SafeQueue& other){ //�������캯�� 
			m_queue = other.mqueue;
		}
		~SafeQueue(){} //�������� 
//		�鿴���� 
		bool empty(){
			unique_lock<mutex> lock(m_mutex); //���������뿪��������Զ�����
			return m_queue.empty();
		}
		int size(){
			unique_lock<mutex> lock(m_mutex);
			return m_queue.size();
		}
//		�޸Ĳ��� 
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
