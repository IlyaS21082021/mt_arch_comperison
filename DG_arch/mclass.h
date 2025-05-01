#ifndef MCLASS_H
#define MCLASS_H

#include <thread>
#include <vector>
#include <queue>
#include <condition_variable>
#include <mutex>

enum eTaskType
{
    XOR,
    CMP,
    TR,
    FL
};

struct TaskStruct
{
    std::pair<char*, char*> addrs;
    size_t bytes;
    eTaskType taskType;
};

class TMclass
{
    std::vector<char> buf1;
    std::vector<char> buf2;
    std::queue<TaskStruct> qTask1;
    std::queue<TaskStruct> qTask2;
    std::mutex m1;
    std::mutex m2;
    std::vector<std::thread> threadV;
    size_t dataSize;
    std::condition_variable CVstart;
    bool qready = false;
    std::mutex mtx;
    size_t NN;

    void ThreadFun(int a);

public:                                  
    TMclass(unsigned int threads, unsigned int bufsize);
    void Run();
};

#endif // MCLASS_H
