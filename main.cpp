#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <memory>

#include <chrono>    // std::chrono::seconds
#include <iostream>  // std::cout
#include <thread>    // std::thread, std::this_thread::sleep_for
#include <mutex>          // std::mutex, std::lock_guard, std::adopt_lock
#include <future>

#include <cstdlib>
#include <condition_variable>
//#include <windows.h>
#include <stdio.h>
#include<regex>

#include <cmath>

#include "TimerCpp.h"

using namespace std;

int g_constructCount = 0;
int g_copyConstructCount = 0;
int g_destructCount = 0;
//struct A
//{
//    A() {
//        cout << "construct: " << ++g_constructCount << endl;
//    }
//
//    A(const A& a)
//    {
//        cout << "copy construct: " << ++g_copyConstructCount << endl;
//    }
//    ~A()
//    {
//        cout << "destruct: " << ++g_destructCount << endl;
//    }
//};
//


//class A
//{
//public:
//    A() :m_ptr(new int(0)) {}
//    A(const A& a) :m_ptr(new int(*a.m_ptr)) //深拷贝的拷贝构造函数
//    {
//        cout << "copy construct" << endl;
//    }
//    A(A&& a) :m_ptr(a.m_ptr)
//    {
//        a.m_ptr = nullptr;
//        cout << "move construct" << endl;
//    }
//    ~A() { delete m_ptr; }
//private:
//    int* m_ptr;
//};
//
//A GetA(bool value)
//{
//    return A();
//}

class A
{
public:
    A(int num):mNum(num)
    {
        std::cout << "constructed A" << std::endl;
    }

    A(A&& a) noexcept : mNum(a.mNum)
    {
        a.mNum = 0;
    }

    A &operator= (A&& a) noexcept
    {
        if (this != &a)
        {
            //free();
            mNum = a.mNum;
        }
        return *this;
    }

    A(const A& a)
    {
        std::cout << "A(const A& a)" << std::endl;
        mNum = a.mNum;
    }

    ~A()
    {
        std::cout << "deconstructed A" << std::endl;
    }

    A &operator=(const A& a)
    {
        std::cout << "赋值操作" << std::endl;
        mNum = a.mNum;
        return *this;
    }

public:
    int mNum;
};

void show1(A& a)
{
    cout << a.mNum << endl;
}
void show2(A a)
{
    cout << a.mNum << endl;
}

using StrSharedPtr = std::shared_ptr<std::string>;
void showInfo()
{
    std::shared_ptr<std::string> sp2 = std::make_shared<std::string>("Hello c++");
    StrSharedPtr sp1 = std::make_shared<std::string>("Hello world!");
    auto sp = std::make_shared<const char*>("Hello World!");

    std::vector<std::string> strVec = { {"AA", "AB", "AC"} };
    strVec.push_back("BB");
    auto pStr = std::make_shared<std::vector<std::string>>(strVec);

    std::cout << *sp2 << std::endl;
    std::cout << *sp << std::endl;
    std::cout << *sp1 << std::endl;

    std::cout << strVec.at(1) << std::endl;
    std::cout << pStr.use_count() << std::endl;
    for (auto it=pStr->begin(); it!=pStr->end(); ++ it)
    {
        std::cout << *it << std::endl;
    }

    cout << sp2.use_count() << endl;
}

void thread_task(int n) {
    std::this_thread::sleep_for(std::chrono::seconds(n));
    std::cout << "hello thread "
        << std::this_thread::get_id()
        << " paused " << n << " seconds" << std::endl;
}

std::mutex mtx;           // mutex for critical section

void print_thread_id(int id) {
    mtx.lock();
    std::lock_guard<std::mutex> lck(mtx, std::adopt_lock);
    std::cout << "thread #" << id << '\n';
}

void print_thread_id_2(int id) {
    std::lock_guard<std::mutex> lck(mtx);
    std::cout << "thread #" << id << '\n';
}

void print_int(std::future<int>& fut) {
    int x = fut.get(); // 获取共享状态的值.
    std::cout << "value: " << x << '\n'; // 打印 value: 10.
}

bool is_prime(int x)
{
    for (int i = 2; i < x; ++i)
        if (x % i == 0)
            return false;
    return true;
}

std::vector<std::string> vec = { "name", "Jack", "David" };
//////////////////////////////////////////////////////////////////
static const int kItemRepositorySize = 4; // Item buffer size.
static const int kItemsToProduce = 10;   // How many items we plan to produce.

struct ItemRepository {
    int item_buffer[kItemRepositorySize];
    size_t read_position;
    size_t write_position;
    size_t produced_item_counter;
    size_t consumed_item_counter;
    std::mutex mtx;
    std::mutex produced_item_counter_mtx;
    std::mutex consumed_item_counter_mtx;
    std::condition_variable repo_not_full;
    std::condition_variable repo_not_empty;
} gItemRepository;

typedef struct ItemRepository ItemRepository;


void ProduceItem(ItemRepository *ir, int item)
{
    std::unique_lock<std::mutex> lock(ir->mtx);
    while (((ir->write_position + 1) % kItemRepositorySize)
        == ir->read_position) { // item buffer is full, just wait here.
        std::cout << "Producer is waiting for an empty slot...\n";
        (ir->repo_not_full).wait(lock);
    }

    (ir->item_buffer)[ir->write_position] = item;
    (ir->write_position)++;

    if (ir->write_position == kItemRepositorySize)
        ir->write_position = 0;

    (ir->repo_not_empty).notify_all();
    lock.unlock();
}

int ConsumeItem(ItemRepository *ir)
{
    int data;
    std::unique_lock<std::mutex> lock(ir->mtx);
    // item buffer is empty, just wait here.
    while (ir->write_position == ir->read_position) {
        std::cout << "Consumer is waiting for items...\n";
        (ir->repo_not_empty).wait(lock);
    }

    data = (ir->item_buffer)[ir->read_position];
    (ir->read_position)++;

    if (ir->read_position >= kItemRepositorySize)
        ir->read_position = 0;

    (ir->repo_not_full).notify_all();
    lock.unlock();

    return data;
}

void ProducerTask()
{
    bool ready_to_exit = false;
    while (1) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::unique_lock<std::mutex> lock(gItemRepository.produced_item_counter_mtx);
        if (gItemRepository.produced_item_counter < kItemsToProduce) {
            ++(gItemRepository.produced_item_counter);
            ProduceItem(&gItemRepository, gItemRepository.produced_item_counter);
            std::cout << "Producer thread " << std::this_thread::get_id()
                << " is producing the " << gItemRepository.produced_item_counter
                << "^th item" << std::endl;
        }
        else ready_to_exit = true;
        lock.unlock();
        if (ready_to_exit == true) break;
    }
    std::cout << "Producer thread " << std::this_thread::get_id()
        << " is exiting..." << std::endl;
}

void ConsumerTask()
{
    bool ready_to_exit = false;
    while (1) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::unique_lock<std::mutex> lock(gItemRepository.consumed_item_counter_mtx);
        if (gItemRepository.consumed_item_counter < kItemsToProduce) {
            int item = ConsumeItem(&gItemRepository);
            ++(gItemRepository.consumed_item_counter);
            std::cout << "Consumer thread " << std::this_thread::get_id()
                << " is consuming the " << item << "^th item" << std::endl;
        }
        else ready_to_exit = true;
        lock.unlock();
        if (ready_to_exit == true) break;
    }
    std::cout << "Consumer thread " << std::this_thread::get_id()
        << " is exiting..." << std::endl;
}

void InitItemRepository(ItemRepository *ir)
{
    ir->write_position = 0;
    ir->read_position = 0;
    ir->produced_item_counter = 0;
    ir->consumed_item_counter = 0;
}

void runConsumer()
{
    InitItemRepository(&gItemRepository);
    std::thread producer1(ProducerTask);
    std::thread producer2(ProducerTask);
    std::thread producer3(ProducerTask);
    std::thread producer4(ProducerTask);

    std::thread consumer1(ConsumerTask);
    std::thread consumer2(ConsumerTask);
    std::thread consumer3(ConsumerTask);
    std::thread consumer4(ConsumerTask);

    producer1.join();
    producer2.join();
    producer3.join();
    producer4.join();

    consumer1.join();
    consumer2.join();
    consumer3.join();
    consumer4.join();
}

void asyncTest()
{
    std::future<int> f1 = std::async(std::launch::async, [](){ return 8; });

    cout << f1.get() << endl; //output: 8

    std::future<int> f2 = std::async(std::launch::async, []() {
        cout << 88 << endl;
        return 88;
    });

    f2.wait(); //output: 8

    std::future<int> future = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return 8;
    });

    std::cout << "waiting...\n";
    std::future_status status;
    do {
        status = future.wait_for(std::chrono::seconds(1));
        if (status == std::future_status::deferred) {
            std::cout << "deferred\n";
        }
        else if (status == std::future_status::timeout) {
            std::cout << "timeout\n";
        }
        else if (status == std::future_status::ready) {
            std::cout << "ready!\n";
        }
    } while (status != std::future_status::ready);

    std::cout << "result is " << future.get() << '\n';
}

void regexTest()
{
    std::string pattern("[^c]ei");
    pattern = "[[:alpha:]]*" + pattern + "[[:alpha:]]*";
    regex r(pattern);
    smatch result;

    string test_str = "receipt freind theif receive";
    //if (regex_search(test_str, result, r))
    //{
    //    std::cout << result.str() << std::endl;
    //}
    string::const_iterator iterStart = test_str.begin();
    string::const_iterator iterEnd = test_str.end();
    sregex_iterator it(iterStart, iterEnd, r), end_it;
    std::string temp;

    //while (regex_search(iterStart, iterEnd, result, r))
    //{
    //    temp = result[0];
    //    cout << temp << endl;
    //    iterStart = result[0].second; //更新搜索起始位置
    //}
    for (; it!=end_it; ++it)
    {
        std::cout << it->str() << std::endl;
    }

    //std::string pattern("[^c]ei");
    //pattern = "[[:alpha:]]*" + pattern + "[[:alpha:]]*";
    //std::regex r(pattern, regex::icase); //忽略大小写
    std::string str("Ruby Carolinei biubiubiu Weindy SpikeI Winnceiy");
    for (sregex_iterator it(str.begin(), str.end(), r), end_it; it != end_it; ++it)
    {
        std::cout << it->str() << std::endl;
    }

}

void forTest()
{
    std::vector<int> vec = { 2,3,6,1,7,4 };
    std::sort(vec.begin(), vec.end(), [](auto i, auto j) { std::cout << i << " " << j <<std::endl; return std::abs(i) < std::abs(j); });

    std::cout << "------"<< std::endl;
    for_each(vec.begin(), vec.end(), [](int a) {std::cout << a << std::endl; });
}

std::string getName()
{
    return std::string("ABC");
}

std::string& doString()
{
    std::string str("doString");
    return str;
}

void printString(std::string &str)
{
    std::cout << str << std::endl;
}

void f(int &a)
{
    cout << "f(" << a << ") is being called" << endl;
}

void g(const int &a)
{
    cout << "g(" << a << ") is being called" << endl;
}


///////////////////////////////////////////////////
bool scanUnsignedInteger(const char** str);
bool scanInteger(const char** str);

// the form of exponent is like A[.[B]][e|EC] or .B[e|EC]
// where A and C is an integer with or without sign, and B is an unsigned integer
bool isNumeric(const char* str)
{
    if (str == NULL)
        return false;

    bool numeric = scanInteger(&str);

    // for floats
    // you can have no integer part such as .123 means 0.123
    // in the meanwhile, you can have no decimal part such as 233. means 233.0
    // what's more, 233.666 is OK also.
    if (*str == '.') {
        ++str;
        numeric = scanUnsignedInteger(&str) || numeric;
    }

    // for exponent
    if (*str == 'e' || *str == 'E') {
        ++str;
        numeric = scanInteger(&str) && numeric;
    }

    return numeric && *str == '\0';
}

bool scanUnsignedInteger(const char** str)
{
    const char* pBefore = *str;
    while (**str != '\0' && **str >= '0' && **str <= '9')
        ++(*str);

    // return true when there are some digits in str
    return *str > pBefore;
}

// an integer's form is like [+|-]B, where B is an unsigned integer
bool scanInteger(const char** str)
{
    if (**str == '+' || **str == '-')
        ++(*str);
    return scanUnsignedInteger(str);
}

// ==================== Test Code ====================

void Test(const char* testName, const char* str, bool expected)
{
    if (testName != NULL)
        printf("%s begins: ", testName);

    if (isNumeric(str) == expected)
        printf("Passed.\n");
    else
        printf("FAILED.\n");
}

void TestCase()
{
    Test("Test1", "100", true);
    Test("Test2", "123.45e+6", true);
    Test("Test3", "+500", true);
    Test("Test4", "5e2", true);
    Test("Test5", "3.1416", true);
    Test("Test6", "600.", true);
    Test("Test7", "-.123", true);
    Test("Test8", "-1E-16", true);
    Test("Test9", "1.79769313486232E+308", true);

    printf("\n\n");

    Test("Test10", "12e", false);
    Test("Test11", "1a3.14", false);
    Test("Test12", "1+23", false);
    Test("Test13", "1.2.3", false);
    Test("Test14", "+-5", false);
    Test("Test15", "12e+5.4", false);
    Test("Test16", ".", false);
    Test("Test17", ".e1", false);
    Test("Test18", "+.", false);
}
////////////////////////////////////////////////////

bool equal(double num1, double num2)
{
    if ((num1 - num2 > -0.0000001) && (num1 - num2 < 0.0000001))
        return true;
    else
        return false;
}

void charTest()
{
    char s[2] = { 'A' + char(5) };
    //char Index = 5 + 'A';
    //snprintf(s, sizeof(s), "%c", Index);
    std::cout<<"length:" << strlen(s) <<"," << s << std::endl;

    std::string str;
    str = 'A' + char(6);
    std::cout << str << std::endl;

    std::string str2("AAA is word!");
    std::cout << str2.size()<<", " << str2.length() << std::endl;
    str2.erase(1,2);
    //str2.clear();
    std::cout << str2 << std::endl;
    std::cout << ((str2=="") ? "true" : "false") << std::endl;

    bool isEqual = equal(0.0, 0.02);
}

void  pfun(int  **p) {
    int  b = 100;
    *p = &b;
}

void fun1(char* str)
{
    str = new char[15];
    //strcpy(str, "test string");
}

void fun2(char** str)
{
    *str = new char[15];
    strcpy_s(*str, strlen(*str), "test string");
}

void pfunTest()
{
    int  a = 10;
    int  *q;
    q = &a;
    printf("%d\n", *q);
    pfun(&q);
    printf("%d\n", *q);

    char *s = nullptr;
    fun2(&s);
    std::cout << s << std::endl;
}

//////////////////
void ReorderOddEven_1(int *pData, unsigned int length)
{
    if (pData == nullptr || length == 0)
        return;

    int *pBegin = pData;
    int *pEnd = pData + length - 1;

    while (pBegin < pEnd)
    {
        // 向后移动pBegin，直到它指向偶数
        while (pBegin < pEnd && (*pBegin & 0x1) != 0)
            pBegin++;

        // 向前移动pEnd，直到它指向奇数
        while (pBegin < pEnd && (*pEnd & 0x1) == 0)
            pEnd--;

        if (pBegin < pEnd)
        {
            int temp = *pBegin;
            *pBegin = *pEnd;
            *pEnd = temp;
        }
    }
}

//2
void Reorder(int *pData, unsigned int length, bool(*func)(int))
{
    if (pData == nullptr || length == 0)
        return;

    int *pBegin = pData;
    int *pEnd = pData + length - 1;

    while (pBegin < pEnd)
    {
        // 向后移动pBegin
        while (pBegin < pEnd && !func(*pBegin))
            pBegin++;

        // 向前移动pEnd
        while (pBegin < pEnd && func(*pEnd))
            pEnd--;

        if (pBegin < pEnd)
        {
            int temp = *pBegin;
            *pBegin = *pEnd;
            *pEnd = temp;
        }
    }
}

bool isEven(int n)
{
    return (n & 1) == 0;
}
void ReorderOddEven_2(int *pData, unsigned int length)
{
    Reorder(pData, length, isEven);
}

// ====================测试代码====================
void PrintArray(int numbers[], int length)
{
    if (length < 0)
        return;

    for (int i = 0; i < length; ++i)
        printf("%d\t", numbers[i]);

    printf("\n");
}

void TestOdd(const char* testName, int numbers[], int length)
{
    if (testName != nullptr)
        printf("%s begins:\n", testName);

    int* copy = new int[length];
    for (int i = 0; i < length; ++i)
    {
        copy[i] = numbers[i];
    }

    printf("Test for solution 1:\n");
    PrintArray(numbers, length);
    ReorderOddEven_1(numbers, length);
    PrintArray(numbers, length);

    printf("Test for solution 2:\n");
    PrintArray(copy, length);
    ReorderOddEven_2(copy, length);
    PrintArray(copy, length);

    delete[] copy;
}

void Test1()
{
    int numbers[] = { 1, 2, 3, 4, 5, 6, 7 };
    TestOdd("Test1", numbers, sizeof(numbers) / sizeof(int));
}

void Test2()
{
    int numbers[] = { 2, 4, 6, 1, 3, 5, 7 };
    TestOdd("Test2", numbers, sizeof(numbers) / sizeof(int));
}

void Test3()
{
    int numbers[] = { 1, 3, 5, 7, 2, 4, 6 };
    TestOdd("Test3", numbers, sizeof(numbers) / sizeof(int));
}

void Test4()
{
    int numbers[] = { 1 };
    TestOdd("Test4", numbers, sizeof(numbers) / sizeof(int));
}

void Test5()
{
    int numbers[] = { 2 };
    TestOdd("Test5", numbers, sizeof(numbers) / sizeof(int));
}

void Test6()
{
    TestOdd("Test6", nullptr, 0);
}

void testOddEven()
{
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
}

void TimerTest()
{
    Timer t = Timer();

    t.setInterval([&]() {
        cout << "Hey.. After each 1s..." << endl;
    }, 1000);

    t.setTimeout([&]() {
        cout << "Hey.. After 5.2s. But I will stop the timer!" << endl;
        t.stop();
    }, 5200);
}

/////////////////////

int main() {
    TimerTest();

    //testOddEven();

    //charTest();

    //pfunTest();

    //TestCase();

    //runConsumer();

    //asyncTest();

    //regexTest();

    //forTest();

    //std::string strTest("This is text!");
    //printString(strTest);
    //printString(doString());    //error

    //showInfo();

    //// call function asynchronously:
    //std::future < bool > fut = std::async(is_prime, 444444443);
    //// do something while waiting for function to set future:
    //std::cout << "checking, please wait"<<std::endl;
    //std::chrono::milliseconds span(100);
    //while (fut.wait_for(span) == std::future_status::timeout)
    //    std::cout << '.';
    //bool x = fut.get();         // retrieve return value
    //std::cout << "\n444444443 " << (x ? "is" : "is not") << " prime.\n";

    //int &&i = 1;
    //int b = 2;
    //cout << i << endl;
    //i = b;
    //cout << i << endl;

    //A a(10);
    //A b(5);
    //A c(2);
    //c = a;
    //cout << "------" << endl;
    //show1(a);
    //cout << "------" << endl;
    //show2(b);
    //cout << "------" << endl;
    //show2(c);
    //cout << "------" << endl;
    ////存放于容器中
    //std::vector<A> m;
    //m.push_back(a);
    //cout << "------" << endl;
    ////动态分配
    //A *d = new A(5);
    //show2(*d);
    //delete d;

    //std::string str = "Hello";
    //std::vector<std::string> v;
    ////调用常规的拷贝构造函数，新建字符数组，拷贝数据
    //v.push_back(str);
    //std::cout << "After copy, str is \"" << str << "\"\n";
    ////调用移动构造函数，掏空str，掏空后，最好不要使用str
    //v.push_back(std::move(str));
    //std::cout << "After move, str is \"" << str << "\"\n";
    //std::cout << "The contents of the vector are \"" << v[0]
    //    << "\", \"" << v[1] << "\"\n";

    //std::thread threads[5];
    //std::cout << "Spawning 5 threads...\n";
    //for (int i = 0; i < 5; i++) {
    //    threads[i] = std::thread(thread_task, i + 1);
    //}
    //std::cout << "Done spawning threads! Now wait for them to join\n";
    //for (auto& t : threads) {
    //    t.join();
    //}
    //std::cout << "All threads joined.\n";

    //std::thread threads[10];
    //// spawn 10 threads:
    //for (int i = 0; i < 10; ++i)
    //    threads[i] = std::thread(print_thread_id, i + 1);
    //for (auto& th : threads) th.join();

    //std::promise<int> prom; // 生成一个 std::promise<int> 对象.
    //std::future<int> fut = prom.get_future(); // 和 future 关联.
    //std::thread t(print_int, std::ref(fut)); // 将 future 交给另外一个线程t.
    //prom.set_value(10); // 设置共享状态的值, 此处和线程t保持同步.
    //t.join();

    system("pause");
    return 0;
}