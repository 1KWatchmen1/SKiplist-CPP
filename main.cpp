/* ************************************************************************
> File Name:     main.cpp
> Author:        程序员Carl
> 微信公众号:    代码随想录
> Created Time:  Sun Dec  2 20:21:41 2018
> Description:   
 ************************************************************************/
#include <iostream>
#include "skiplist.h"
// #define FILE_PATH "./store/dumpFile"

int main() {

    // 键值中的key用int型，如果用其他类型，需要自定义比较函数
    // 而且如果修改key的类型，同时需要修改skipList.load_file函数
    SkipList<int, std::string> skipList(6);// // 创建一个最大层数为6的SkipList实例
	skipList.insert_element(1, "学"); 
	skipList.insert_element(3, "算法"); 
	skipList.insert_element(7, "认准"); 
	skipList.insert_element(8, "微信公众号：代码随想录"); 
	skipList.insert_element(9, "学习"); 
	skipList.insert_element(19, "算法不迷路"); 
	skipList.insert_element(19, "赶快关注吧你会发现详见很晚！"); 
    // // 输出SkipList的大小
    std::cout << "skipList size:" << skipList.size() << std::endl;
    // // 输出SkipList的大小
    skipList.dump_file();
    // 从文件中加载SkipList
    // skipList.load_file();
    // 查找键为9和18的元素
    skipList.search_element(9);
    skipList.search_element(18);

    // 展示当前的SkipList内容
    skipList.display_list();
    // 删除键为3和7的元素
    skipList.delete_element(3);
    skipList.delete_element(7);
     // 输出SkipList的大小
    std::cout << "skipList size:" << skipList.size() << std::endl;

    skipList.display_list();
}
