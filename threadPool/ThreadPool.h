#pragma once
#include<functional> //������
#include<future>
#include<mutex>
#include<queue>
#include<thread>
#include<utility>
#include<vector>
#include "SafeQueue.h"
using namespace std;

class ThreadPool{
	private:
		bool m_shutdown; //�̳߳��Ƿ�ر�
		SafeQueue<function<void()>> m_queue; //�������
		vector<thread> m_threads; //�̶߳���
		mutex m_conditional_mutex; //���̳߳ؼ���
		condition_variable m_conditional_lock; //�̻߳�����,����ʹ�߳����߻��� 
		class ThreadWorker{ // �����̹߳����� 
			private:
				int m_id; //�߳�ID 
				ThreadPool *m_pool; //�߳������̳߳�
			public:
				ThreadWorker(ThreadPool *pool, const int id) :m_pool(pool), m_id(id){}
//				����()����
				void operator()(){
					function<void()> func; // �������������func 
//					ȡ����ͳ�ĺ���ָ�룬�ŵ㣺�ɶ��Ժã��ɽ���lambda 
//					https://shengyu7697.github.io/blog/2020/09/02/std-function/
					
					bool dequeued; //�Ƿ�����ȡ����������е�Ԫ��
					
//					�ж��̳߳��Ƿ�رգ�û�йرվ�ѭ����ȡ
					while (!m_pool->m_shutdown){
//						���� 
						unique_lock<std::mutex> lock(m_pool->m_conditional_mutex);
						
//						������пգ�������ǰ�߳�
//						û������,�����°� 
						if(m_pool->m_queue.empty()){ 
							m_pool->m_conditional_lock.wait(lock);
						}

//						ȡ��������е�Ԫ��
						dequeued =  m_pool->m_queue.dequeue(func);
						
//						�ɹ�ȡ��,ִ�й������� 
						if(dequeued){
							func();
						} 
					}
				} 
		};
	
	public:
//		���캯��,����Ϊ�̵߳ĸ��� 
		ThreadPool(const int n_threads) :m_threads(vector<thread>(n_threads)), m_shutdown(false){}
		
//		delete����Ĭ�����ɵĺ���
		ThreadPool(const ThreadPool &) = delete;
		ThreadPool(ThreadPool &&) = delete;
		ThreadPool& operator=(const ThreadPool &) = delete;
		ThreadPool& operator=(ThreadPool &&) = delete;
		
//		��ʼ��
		void init(){
			for(int i = 0; i < m_threads.size(); i++){
//				ThreadWorker����������(),Ϊ�ɵ��ö��� 
				m_threads[i] = thread(ThreadWorker(this, i));
			}
		}
		
		void shutdown(){
			m_shutdown = true;
//			����?? 
			m_conditional_lock.notify_all();
			
			for(int i = 0; i < m_threads.size(); i++){
				if(m_threads[i].joinable()){
//					���̼߳���ȴ����� 
					m_threads[i].join();
				}
			}
		}
		
		template<typename F, typename...Args> // �����
//		�������ͺ���:https://blog.csdn.net/craftsman1970/article/details/80185781
//		https://blog.csdn.net/yockie/article/details/51731281
//		Ϊʲôʹ����ֵ?? 
//		future:�����첽�������,�ӽ���ʱ���ȡ���������ͨ����ѯ״̬��ִ֪�������Ҳ����������ȡ�첽ִ�еĽ��
		auto submit(F&& f, Args&&... args) -> future<decltype f(args...)>{
//			bind:��ĳ����ʽ�Ĳ����б�����֪�ĺ������а󶨣��γ��µĺ���
//			forward:����ת��,���ղ���ԭ��������ת������һ������ 
			function<decltype(f(args...)())> func = bind(forward<F>(f), forward<Args>(args)...);
//			?????
			
//			�漰������ָ��
//			make_shared:�ڶ�̬�ڴ��г�ʼ��һ�����󣬷���ָ��ö����shared_ptr
//			packaged_task:��function<F>��ɵ�����(�纯����lambda���ʽ��)���а�װ����ʹ�÷���������ɵ�����ķ��ؽ�����ݸ�������future����
//			https://blog.csdn.net/godmaycry/article/details/72868559
//			����ָ�� 
			auto task_ptr = make_shared<packaged_task<decltype(f(args...))()>>(func);
			
//			������ʽ,����һ����������
//			????,����������ʽ 
			function<void()> wrapper_func = [task_ptr]() {
      			(*task_ptr)(); 
    		};
    		
//    		��������ѹ��������� 
    		m_queue.enqueue(wrapper_func);
			
//			����һ���߳�??
			m_conditional_lock.notify_one();
			
//			����֮ǰע�������ָ��??
			return task_ptr->get_future(); 
		}
};
