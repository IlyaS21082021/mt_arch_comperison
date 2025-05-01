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

struct TaskStructQueue
{
    std::queue<TaskStruct> qTask;
    std::unique_ptr<std::mutex> m;
    TaskStructQueue() : m(std::make_unique<std::mutex>()) {}
};

class TMclass
{
    std::vector<char> buf1;
    std::vector<char> buf2;
    std::vector<TaskStructQueue> taskV1;
    std::vector<TaskStructQueue> taskV2;
    std::vector<std::thread> threadV;
    size_t dataSize;
    std::condition_variable CVstart;
    bool qready = false;
    std::mutex mtx;

    void ThreadFun(int a);

public:                                  
    TMclass(unsigned int threads, unsigned int bufsize);
    void Run();
};

#endif // MCLASS_H
