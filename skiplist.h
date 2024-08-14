/* ************************************************************************
> File Name:     skiplist.h
> Author:        程序员Carl
> 微信公众号:    代码随想录
> Created Time:  Sun Dec  2 19:04:26 2018
> Description:   
 ************************************************************************/

#include <iostream> 
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>

#define STORE_FILE "store/dumpFile"

std::mutex mtx;     // mutex for critical section
std::string delimiter = ":";

//Class template to implement node
template<typename K, typename V> 
class Node {

public:
    
    Node() {} 

    Node(K k, V v, int); 

    ~Node();
    //用于获取和设置节点的键和值。
    K get_key() const;
    V get_value() const;
    void set_value(V);
    
    // Linear array to hold pointers to next node of different level
    Node<K, V> **forward;

    int node_level;

private:
    K key;
    V value;
};

template<typename K, typename V> 
Node<K, V>::Node(const K k, const V v, int level) {
    this->key = k;
    this->value = v;
    this->node_level = level; 

    // level + 1, because array index is from 0 - level
    this->forward = new Node<K, V>*[level+1];
    
	// Fill forward array with 0(NULL) 
    memset(this->forward, 0, sizeof(Node<K, V>*)*(level+1));
};

template<typename K, typename V> 
Node<K, V>::~Node() {
    delete []forward;
};

template<typename K, typename V> 
K Node<K, V>::get_key() const {
    return key;
};

template<typename K, typename V> 
V Node<K, V>::get_value() const {
    return value;
};
template<typename K, typename V> 
void Node<K, V>::set_value(V value) {
    this->value=value;
};

// Class template for Skip list
template <typename K, typename V> 
class SkipList {

public: 
    SkipList(int);//初始化最大层数 _max_level，跳表当前层数 _skip_list_level 为 0，元素数量 _element_count 为 0。
    ~SkipList();//关闭文件流。使用 clear 函数递归删除跳表中的所有节点，最后删除头节点。
    int get_random_level();//生成一个随机层数，使用 rand() 函数决定是否增加层数，返回的层数不能超过 _max_level。
    Node<K, V>* create_node(K, V, int);//创建一个新的节点，并根据给定的层数为其分配 forward 数组。
    int insert_element(K, V);
    void display_list();
    bool search_element(K);
    void delete_element(K);
    void dump_file();//将跳表中的数据（即键值对）持久化到文件 STORE_FILE 中。
    void load_file();//从文件 STORE_FILE 中加载数据到跳表中。
    //递归删除节点
    void clear(Node<K,V>*);//递归删除跳表中的所有节点。
    //返回跳表中的元素数量。
    int size();

private:
//从字符串中提取键和值，使用分隔符 delimiter。
    void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);
    //验证字符串是否符合键值对的格式（通过检查分隔符）
    bool is_valid_string(const std::string& str);

private:    
    // Maximum level of the skip list 
    int _max_level;// 跳表的最大层数

    // current level of skip list 
    int _skip_list_level;

    // pointer to header node 
    Node<K, V> *_header;

    // file operator
    std::ofstream _file_writer;
    std::ifstream _file_reader;

    // skiplist current element count
    int _element_count;
};

// create new node 
template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(const K k, const V v, int level) {
    Node<K, V> *n = new Node<K, V>(k, v, level);
    return n;
}

// Insert given key and value in skip list 
// return 1 means element exists  
// return 0 means insert successfully
/* 
                           +------------+
                           |  insert 50 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |                      insert +----+
level 3         1+-------->10+---------------> | 50 |          70       100
                                               |    |
                                               |    |
level 2         1          10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 1         1    4     10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 0         1    4   9 10         30   40  | 50 |  60      70       100
                                               +----+

*/
//这个函数用于将一个键值对插入到跳表（SkipList）中。插入过程包括在每一层找到新节点的正确位置、调整指针，以及可能更新跳表的层级。
template<typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value) {
    
    mtx.lock();//首先通过锁定互斥锁来确保线程安全。
    Node<K, V> *current = this->_header;//定义current指针，初始时指向跳表的头节点。

    // create update array and initialize it 
    // update is array which put node that the node->forward[i] should be operated later
    Node<K, V> *update[_max_level+1];//update数组用于保存需要更新指针的节点。数组的大小为_max_level + 1，即跳表的最大层数。这一步通过memset将数组初始化为NULL。
    memset(update, 0, sizeof(Node<K, V>*)*(_max_level+1));  

    // start form highest level of skip list 
    for(int i = _skip_list_level; i >= 0; i--) {
        while(current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i]; 
        }
        update[i] = current;//update[i]保存的是每层中最后一个小于key的节点
    }

    // reached level 0 and forward pointer to right node, which is desired to insert key.
    current = current->forward[0];//current指针指向第0层中大于或等于key的节点

    // if current node have key equal to searched key, we get it
    //如果该节点的键值与插入的key相同，说明key已经存在于跳表中
    if (current != NULL && current->get_key() == key) {
        std::cout << "key: " << key << ", exists" << std::endl;
        mtx.unlock();
        return 1;
    }

    // if current is NULL that means we have reached to end of the level 
    // if current's key is not equal to key that means we have to insert node between update[0] and current node 
    
    if (current == NULL || current->get_key() != key ) {
        
        // Generate a random level for node
        int random_level = get_random_level();//调用get_random_level()函数生成一个随机的层级random_level。

        // If random level is greater thar skip list's current level, initialize update value with pointer to header
        if (random_level > _skip_list_level) {// 如果生成的random_level大于当前的跳表层级_skip_list_level，则需要更新_skip_list_level，并且将update数组中对应位置的节点指向跳表的头节点。
            for (int i = _skip_list_level+1; i < random_level+1; i++) {
                update[i] = _header;
            }
            _skip_list_level = random_level;
        }

        // create new node with random level generated 
        // 使用生成的random_level创建一个新节点，并在每一层中将该节点插入到合适的位置，即更新update[i]的forward[i]指针，使其指向新插入的节点。
        Node<K, V>* inserted_node = create_node(key, value, random_level);
        
        // insert node 
        for (int i = 0; i <= random_level; i++) {
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }
        std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;
        _element_count ++;
    }
    //释放互斥锁，并返回0表示成功插入
    mtx.unlock();
    return 0;
}

// Display skip list 
//这个display_list函数用于展示跳表中的元素。它逐层遍历跳表，并将每一层的节点键值对输出到控制台。
template<typename K, typename V> 
void SkipList<K, V>::display_list() {

    std::cout << "\n*****Skip List*****"<<"\n"; 
    for (int i = 0; i <= _skip_list_level; i++) {//遍历跳表的每一层，从0层到最高层_skip_list_level。
        Node<K, V> *node = this->_header->forward[i]; //获取当前层的第一个节点：通过头节点_header的forward[i]指针获取当前层的第一个节点。
        std::cout << "Level " << i << ": ";//输出当前层号例如输出Level 0: 来标识当前正在输出第0层的节点。
        while (node != NULL) {//内层循环：在当前层中，沿着forward[i]指针依次遍历每个节点，直到NULL为止。每个节点的键值对通过get_key()和get_value()方法获取，
            std::cout << node->get_key() << ":" << node->get_value() << ";";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}

// Dump data in memory to file 
//这个dump_file函数的目的是将跳表中的所有元素（键值对）保存到一个文件中，并在控制台上输出这些元素。这对于数据的持久化和调试非常有用。
template<typename K, typename V> 
void SkipList<K, V>::dump_file() {

    std::cout << "dump_file-----------------" << std::endl;
    _file_writer.open(STORE_FILE);//这里使用open方法打开一个文件STORE_FILE（假设STORE_FILE是一个常量或成员变量，表示文件路径），并准备将跳表中的内容写入这个文件。
    Node<K, V> *node = this->_header->forward[0]; //获取第一个节点：通过头节点_header的forward[0]指针获取最低层（Level 0）的第一个节点。

    while (node != NULL) {//使用while循环遍历最低层的所有节点，直到NULL，表示遍历结束。
        _file_writer << node->get_key() << ":" << node->get_value() << "\n";//将当前节点的键和值按照key:value格式写入文件，每对键值之间以换行符分隔。
        std::cout << node->get_key() << ":" << node->get_value() << ";\n";//将同样的内容输出到控制台，用于调试和确认写入操作
        node = node->forward[0];
    }

    _file_writer.flush();//使用flush方法确保所有缓冲区中的数据都写入到文件中
    _file_writer.close();//使用close方法关闭文件，确保资源被正确释放。
    return ;
}

// Load data from disk
//load_file 函数的作用是从存储文件中读取之前保存的键值对，并将它们重新插入到跳表中，从而恢复跳表的状态。这个过程通常在程序启动时进行，以从持久化的文件中加载数据。
template<typename K, typename V> 
void SkipList<K, V>::load_file() {

    _file_reader.open(STORE_FILE);//打开文件STORE_FILE，准备从中读取数据。STORE_FILE应该是一个定义好的文件路径（例如在dump_file函数中使用的文件路径）。
    std::cout << "load_file-----------------" << std::endl;
    std::string line;//line 用于保存从文件读取的一行内容
    std::string* key = new std::string();//key 和 value 是用来存储解析出来的键和值的动态字符串对象。
    std::string* value = new std::string();
    while (getline(_file_reader, line)) {//读取每一行：使用 getline 方法逐行读取文件内容，每一行表示一个键值对。
        get_key_value_from_string(line, key, value);//解析键值对：调用 get_key_value_from_string 函数将字符串 line 解析成键和值，并分别存储在 key 和 value 中。
        if (key->empty() || value->empty()) {//跳过空值：如果 key 或 value 为空字符串，则跳过该行。
            continue;
        }
        // Define key as int type
        //此处假设 key 是 int 类型，因此使用 stoi 将字符串转换为整数。
        insert_element(stoi(*key), *value);//插入元素：使用 insert_element 函数将解析出来的键值对插入到跳表中。
        std::cout << "key:" << *key << "value:" << *value << std::endl;//输出键和值，以确认插入操作的正确性。
    }
    delete key;//在使用完 key 和 value 后，通过 delete 释放动态分配的内存，防止内存泄漏。
    delete value;
    _file_reader.close();//完成文件读取操作后，使用 close 方法关闭文件，释放文件资源
}

// Get current SkipList size
template<typename K, typename V> 
int SkipList<K, V>::size() { 
    return _element_count;
}
//get_key_value_from_string 函数用于从一个字符串中解析出键值对。该字符串通常是存储在文件中的键值对表示形式
template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value) {

    if(!is_valid_string(str)) {//检查传入的字符串 str 是否有效（例如是否符合预期的格式）。如果字符串无效，函数直接返回，不做任何处理。
        return;
    }
    *key = str.substr(0, str.find(delimiter));//使用 str.substr(0, str.find(delimiter)) 获取字符串 str 中第一个分隔符 delimiter 之前的部分，并将其赋值给 key
    *value = str.substr(str.find(delimiter)+1, str.length());//这行代码提取分隔符 delimiter 之后的部分，并将其赋值给 value。str.find(delimiter)+1 找到分隔符之后的第一个字符位置，str.substr(..., str.length()) 提取从该位置到字符串末尾的子字符串。
}

template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str) {

    if (str.empty()) {
        return false;
    }
    if (str.find(delimiter) == std::string::npos) {
        return false;
    }
    return true;
}

// Delete element from skip list 
//delete_element 函数用于从跳表（SkipList）中删除指定键的节点。这个函数通过遍历跳表的层级结构，找到目标节点并删除它，然后调整跳表的结构以保持其正确性。
template<typename K, typename V> 
void SkipList<K, V>::delete_element(K key) {

    mtx.lock();//避免多个线程同时操作跳表导致数据不一致。
    Node<K, V> *current = this->_header; //current 指针从跳表的头节点 _header 开始遍历，update 数组用于记录在每个层级上，需要更新的前置节点（即当前节点的前一个节点）。
    Node<K, V> *update[_max_level+1];
    memset(update, 0, sizeof(Node<K, V>*)*(_max_level+1));

    // start from highest level of skip list
    //从跳表的最高层级开始向下遍历，查找目标键所在的位置。每当移动到下一个节点时，都会更新 update 数组以记录在当前层级上的前置节点。
    for (int i = _skip_list_level; i >= 0; i--) {
        while (current->forward[i] !=NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }
    //在最底层（层级0），检查当前节点是否是要删除的节点。如果是，继续执行删除操作。
    current = current->forward[0];
    if (current != NULL && current->get_key() == key) {
       
        // start for lowest level and delete the current node of each level
        //从最底层到最高层，逐层删除目标节点，并更新前置节点的 forward 指针以跳过目标节点。
        for (int i = 0; i <= _skip_list_level; i++) {

            // if at level i, next node is not target node, break the loop.
            if (update[i]->forward[i] != current) 
                break;

            update[i]->forward[i] = current->forward[i];
        }

        // Remove levels which have no elements
        //如果最高层级上的节点被删除且此层级已空，则减少跳表的层级，以确保跳表的有效层级不会包含空层。
        while (_skip_list_level > 0 && _header->forward[_skip_list_level] == 0) {
            _skip_list_level --; 
        }

        std::cout << "Successfully deleted key "<< key << std::endl;
        delete current;//释放被删除节点的内存，并减少跳表的元素计数。
        _element_count --;
    }
    mtx.unlock();//解锁，以允许其他线程访问跳表。
    return;
}

// Search for element in skip list 
/*
                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+------------------>50+           70       100
                                                   |
                                                   |
level 2         1          10         30         50|           70       100
                                                   |
                                                   |
level 1         1    4     10         30         50|           70       100
                                                   |
                                                   |
level 0         1    4   9 10         30   40    50+-->60      70       100
*/
//search_element 函数用于在跳表（SkipList）中查找指定的键。它通过在跳表的层级结构中逐步缩小搜索范围，最终在最底层检查目标键是否存在。
template<typename K, typename V> 
bool SkipList<K, V>::search_element(K key) {

    std::cout << "search_element-----------------" << std::endl;
    Node<K, V> *current = _header;//将 current 指针初始化为跳表的头节点 _header，从这个节点开始搜索。

    // start from highest level of skip list
    //从跳表的最高层级开始，逐级向下进行搜索。在每个层级上，current 指针会向前移动，直到找到一个节点，其键值不再小于目标键 key。
    for (int i = _skip_list_level; i >= 0; i--) {
        while (current->forward[i] && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
    }

    //reached level 0 and advance pointer to right node, which we search
    current = current->forward[0];//到达最低层级（层级0）并检查下一个节点：

    // if current node have key equal to searched key, we get it
    //如果当前节点的键值等于目标键，则表示找到了该键，打印出键和值并返回 true。
    if (current and current->get_key() == key) {
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }
    //如果遍历结束后仍未找到目标键，则打印出“未找到”信息并返回 false
    std::cout << "Not Found Key:" << key << std::endl;
    return false;
}

// construct skip list
//为跳表提供了初始化和清理功能，并且在析构函数中递归删除了跳表的节点。
template<typename K, typename V> 
SkipList<K, V>::SkipList(int max_level) {
    //，初始化了跳表的最大层数、当前层数和元素数量。
    this->_max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;

    // create header node and initialize key and value to null
    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level);//_header 节点的键值对 k 和 v 为空，且初始化了指向 max_level 层的前进指针。
};

template<typename K, typename V> 
SkipList<K, V>::~SkipList() {
    //处理了文件读写器的关闭
    if (_file_writer.is_open()) {
        _file_writer.close();
    }
    if (_file_reader.is_open()) {
        _file_reader.close();
    }

    //使用递归方式清理跳表中的节点。递归方式简单直接，但当跳表中元素较多时，可能会导致栈溢出问题
    if(_header->forward[0]!=nullptr){
        clear(_header->forward[0]);
    }
    delete(_header);
    
}
//该函数通过递归删除节点链条。对于大数据量的跳表，建议改为非递归实现以避免栈溢出。
//递归删除时从最底层开始，确保所有节点都能被正确删除。
template <typename K, typename V>
void SkipList<K, V>::clear(Node<K, V> * cur)
{
    if(cur->forward[0]!=nullptr){
        clear(cur->forward[0]);
    }
    delete(cur);
}

//函数生成随机层级，用于决定新插入节点在跳表中的层数
//这种随机生成层数的方法（1/2 概率增加层级）是 SkipList 的经典实现。
template<typename K, typename V>
int SkipList<K, V>::get_random_level(){

    int k = 1;
    while (rand() % 2) {
        k++;
    }
    k = (k < _max_level) ? k : _max_level;
    return k;
};
// vim: et tw=100 ts=4 sw=4 cc=120
