/* ************************************************************************
> File Name:     stress_test.cpp
> Author:        程序员Carl
> 微信公众号:    代码随想录
> Created Time:  Sun 16 Dec 2018 11:56:04 AM CST
> Description:   
 ************************************************************************/

#include <iostream>
#include <chrono>
#include <cstdlib>
#include <pthread.h>
#include <time.h>
#include "../skiplist.h"
/*头文件引入了必要的库，包括跳表的头文件 skiplist.h。
NUM_THREADS 定义了线程数，TEST_COUNT 定义了测试的操作次数。
创建了一个跳表实例 skipList，最大层数为18。*/
#define NUM_THREADS 1
#define TEST_COUNT 100000
SkipList<int, std::string> skipList(18);
//每个线程执行插入操作，将随机生成的键值对插入到跳表中。tmp 变量确定每个线程插入的元素数量。
void *insertElement(void* threadid) {
    long tid; 
    tid = (long)threadid;
    std::cout << tid << std::endl;  
    int tmp = TEST_COUNT/NUM_THREADS; 
	for (int i=tid*tmp, count=0; count<tmp; i++) {
        count++;
		skipList.insert_element(rand() % TEST_COUNT, "a"); 
	}
    pthread_exit(NULL);
}
/*每个线程执行查找操作，尝试查找随机生成的键。
查找操作被注释掉了，可以用来测试查找性能。*/
void *getElement(void* threadid) {
    long tid; 
    tid = (long)threadid;
    std::cout << tid << std::endl;  
    int tmp = TEST_COUNT/NUM_THREADS; 
	for (int i=tid*tmp, count=0; count<tmp; i++) {
        count++;
		skipList.search_element(rand() % TEST_COUNT); 
	}
    pthread_exit(NULL);
}
//主函数创建了指定数量的线程，执行插入操作并计算插入操作的时间。
int main() {
    srand (time(NULL));  //srand(time(NULL)) 初始化随机数生成器种子，以确保每次运行程序时生成的随机数序列不同。
    {

        pthread_t threads[NUM_THREADS];//创建一个线程数组 threads 来保存线程标识符
        int rc;
        int i;

        auto start = std::chrono::high_resolution_clock::now();

        for( i = 0; i < NUM_THREADS; i++ ) {
            std::cout << "main() : creating thread, " << i << std::endl;
            //pthread_create 函数的第四个参数是线程函数的参数。这里传递了线程的 ID (void *)i。
            rc = pthread_create(&threads[i], NULL, insertElement, (void *)i);//使用 pthread_create 创建线程，每个线程执行 insertElement 函数。

            if (rc) {
                std::cout << "Error:unable to create thread," << rc << std::endl;
                exit(-1);
            }
        }

        void *ret;
        //使用 pthread_join 等待所有线程完成执行。pthread_join 会阻塞主线程直到对应的线程结束。
        for( i = 0; i < NUM_THREADS; i++ ) {
            if (pthread_join(threads[i], &ret) !=0 )  {
                perror("pthread_create() error"); 
                exit(3);
            }
        }
        //计算线程执行的时间，并输出结果。
        auto finish = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double> elapsed = finish - start;
        std::cout << "insert elapsed:" << elapsed.count() << std::endl;
    }
    // skipList.displayList();

    // {
    //     pthread_t threads[NUM_THREADS];
    //     int rc;
    //     int i;
    //     auto start = std::chrono::high_resolution_clock::now();

    //     for( i = 0; i < NUM_THREADS; i++ ) {
    //         std::cout << "main() : creating thread, " << i << std::endl;
    //         rc = pthread_create(&threads[i], NULL, getElement, (void *)i);

    //         if (rc) {
    //             std::cout << "Error:unable to create thread," << rc << std::endl;
    //             exit(-1);
    //         }
    //     }

    //     void *ret;
    //     for( i = 0; i < NUM_THREADS; i++ ) {
    //         if (pthread_join(threads[i], &ret) !=0 )  {
    //             perror("pthread_create() error"); 
    //             exit(3);
    //         }
    //     }

    //     auto finish = std::chrono::high_resolution_clock::now(); 
    //     std::chrono::duration<double> elapsed = finish - start;
    //     std::cout << "get elapsed:" << elapsed.count() << std::endl;
    // }
    //pthread_exit(NULL) 用于退出主线程，确保所有线程都完成。
	pthread_exit(NULL);
    return 0;//return 0; 标志程序正常结束

}
