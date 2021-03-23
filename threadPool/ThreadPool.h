#pragma once
#include<functional> //函数包
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
		bool m_shutdown; //线程池是否关闭
		SafeQueue<function<void()>> m_queue; //任务队列
		vector<thread> m_threads; //线程队列
		mutex m_conditional_mutex; //给线程池加锁
		condition_variable m_conditional_lock; //线程环境锁,可以使线程休眠或唤醒 
		class ThreadWorker{ // 内置线程工作类 
			private:
				int m_id; //线程ID 
				ThreadPool *m_pool; //线程所属线程池
			public:
				ThreadWorker(ThreadPool *pool, const int id) :m_pool(pool), m_id(id){}
//				重载()操作
				void operator()(){
					function<void()> func; // 定义基础函数类func 
//					取代传统的函数指针，优点：可读性好，可接受lambda 
//					https://shengyu7697.github.io/blog/2020/09/02/std-function/
					
					bool dequeued; //是否正在取出任务队列中的元素
					
//					判断线程池是否关闭，没有关闭就循环提取
					while (!m_pool->m_shutdown){
//						加锁 
						unique_lock<std::mutex> lock(m_pool->m_conditional_mutex);
						
//						任务队列空，阻塞当前线程
//						没有任务,上锁下班 
						if(m_pool->m_queue.empty()){ 
							m_pool->m_conditional_lock.wait(lock);
						}

//						取出任务队列的元素
						dequeued =  m_pool->m_queue.dequeue(func);
						
//						成功取出,执行工作函数 
						if(dequeued){
							func();
						} 
					}
				} 
		};
	
	public:
//		构造函数,参数为线程的个数 
		ThreadPool(const int n_threads) :m_threads(vector<thread>(n_threads)), m_shutdown(false){}
		
//		delete禁用默认生成的函数
		ThreadPool(const ThreadPool &) = delete;
		ThreadPool(ThreadPool &&) = delete;
		ThreadPool& operator=(const ThreadPool &) = delete;
		ThreadPool& operator=(ThreadPool &&) = delete;
		
//		初始化
		void init(){
			for(int i = 0; i < m_threads.size(); i++){
//				ThreadWorker对象重载了(),为可调用对象 
				m_threads[i] = thread(ThreadWorker(this, i));
			}
		}
		
		void shutdown(){
			m_shutdown = true;
//			唤醒?? 
			m_conditional_lock.notify_all();
			
			for(int i = 0; i < m_threads.size(); i++){
				if(m_threads[i].joinable()){
//					将线程加入等待队列 
					m_threads[i].join();
				}
			}
		}
		
		template<typename F, typename...Args> // 多参数
//		返回类型后置:https://blog.csdn.net/craftsman1970/article/details/80185781
//		https://blog.csdn.net/yockie/article/details/51731281
//		为什么使用右值?? 
//		future:访问异步操作结果,从将来时间获取结果，可以通过查询状态获知执行情况，也可以阻塞获取异步执行的结果
		auto submit(F&& f, Args&&... args) -> future<decltype f(args...)>{
//			bind:把某种形式的参数列表与已知的函数进行绑定，形成新的函数
//			forward:完美转发,按照参数原来的类型转发到另一个函数 
			function<decltype(f(args...)())> func = bind(forward<F>(f), forward<Args>(args)...);
//			?????
			
//			涉及到智能指针
//			make_shared:在动态内存中初始化一个对象，返回指向该对象的shared_ptr
//			packaged_task:对function<F>这可调对象(如函数、lambda表达式等)进行包装，简化使用方法。将这可调对象的返回结果传递给关联的future对象。
//			https://blog.csdn.net/godmaycry/article/details/72868559
//			任务指针 
			auto task_ptr = make_shared<packaged_task<decltype(f(args...))()>>(func);
			
//			正则表达式,返回一个函数对象
//			????,不懂正则表达式 
			function<void()> wrapper_func = [task_ptr]() {
      			(*task_ptr)(); 
    		};
    		
//    		把任务函数压入任务队列 
    		m_queue.enqueue(wrapper_func);
			
//			唤醒一个线程??
			m_conditional_lock.notify_one();
			
//			返回之前注册的任务指针??
			return task_ptr->get_future(); 
		}
};
