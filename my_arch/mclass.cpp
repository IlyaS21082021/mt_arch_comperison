#include <iostream>
#include <cstdlib>
#include "mclass.h"


TMclass::TMclass(unsigned int threads, unsigned int bufsize)
{
    dataSize = bufsize;
    buf1.resize(bufsize);
    buf2.resize(bufsize);
    taskV1.resize(threads);
    taskV2.resize(threads);
    threadV.resize(threads);

    for (auto& i : buf1)
        i = rand()%100;
}

void TMclass::Run()
{
    char* curPIn = buf1.data();
    char* curPOut = buf2.data();
    char* lastAddr = &buf1.back();
    size_t dataBlocks = 0;

    int t = 0;
    for (auto& i : threadV)
    {
        i = std::thread(&TMclass::ThreadFun, this, t);
        t++;
    }

    bool buf1Exported = false;
    while (!buf1Exported)
    {
        for (auto & i : taskV1)
        {
            if (i.qTask.size() == 10)
                continue;

            if (curPIn + 256 <= lastAddr)
            {
                if (i.m->try_lock())
                {
                    i.qTask.push({{curPIn, curPOut}, 256, XOR});
                    i.m->unlock();
                    curPIn += 256;
                    curPOut += 256;
                    dataBlocks++;
                }
            }
            else
            {
                if (i.m->try_lock())
                {
                    i.qTask.push({{curPIn, curPOut}, static_cast<size_t>(lastAddr - curPIn), XOR});
                    i.m->unlock();
                    dataBlocks++;
                    buf1Exported = true;
                    break;
                }
            }
        }
        if (!qready)
        {
            std::unique_lock<std::mutex> mixLock(mtx);
            qready = true;
            CVstart.notify_all();
        }
    }

    eTaskType taskType;
    while (dataBlocks > 0)
    {
        for (auto& queueTask2 : taskV2)
        {
            while(queueTask2.qTask.size() > 0)
            {
                queueTask2.m->lock();
                auto [inbuf, outbuf] = queueTask2.qTask.front().addrs;
                size_t bytes = queueTask2.qTask.front().bytes;
                taskType = queueTask2.qTask.front().taskType;
                queueTask2.qTask.pop();
                queueTask2.m->unlock();

                if (taskType == XOR)
                {
                    bool cmp = false;
                    while (!cmp)
                    {
                        for (auto &queueTask1 : taskV1)
                        {
                            if (queueTask1.qTask.size() == 10)
                                continue;

                            queueTask1.m->lock();
                            queueTask1.qTask.push({{inbuf, outbuf}, bytes, CMP});
                            queueTask1.m->unlock();
                            cmp = true;
                            break;
                        }
                    }
                }
                else if (taskType == TR)
                {
                    std::cout << "Ok. Address " << outbuf - buf2.data() << "\n";
                    dataBlocks--;
                }
                else if (taskType == FL)
                {
                    std::cout << "Fail. Address " << outbuf - buf2.data() << "\n";
                    dataBlocks--;
                }
            }
        }
    }

    for (auto &queueTask1 : taskV1)
    {
        queueTask1.m->lock();
        queueTask1.qTask.push({{nullptr, nullptr}, 0, TR});
        queueTask1.m->unlock();
    }

    for (auto& i : threadV)
        i.join();
}

void TMclass::ThreadFun(int a)
{
    eTaskType taskType;
    std::unique_lock<std::mutex> mixLock(mtx);
    CVstart.wait(mixLock, [this]{return qready;});
    mixLock.unlock();

    std::cout << "Thread " << a << " started\n";

    while (1)
    {
        if (taskV1[a].qTask.size() > 0)
        {
            taskV1[a].m->lock();
            auto [inbuf, outbuf] = taskV1[a].qTask.front().addrs;
            size_t bytes = taskV1[a].qTask.front().bytes;
            taskType = taskV1[a].qTask.front().taskType;
            taskV1[a].qTask.pop();
            taskV1[a].m->unlock();

            if (!inbuf)
                break;  // STOP

            if (taskType == XOR)
            {
                for (size_t i = 0; i < bytes; i++)
                {
                    outbuf[i] = inbuf[i] ^ 0xF;
                }
                taskV2[a].m->lock();
                taskV2[a].qTask.push({{outbuf, inbuf}, bytes, XOR});
                taskV2[a].m->unlock();
            }
            else if (taskType == CMP)
            {
                eTaskType res = TR;
                for (size_t i = 0; i < bytes; i++)
                {
                    if (outbuf[i] != (inbuf[i] ^ 0xF))
                    {
                        res = FL;
                        break;
                    }
                }
                taskV2[a].m->lock();
                taskV2[a].qTask.push({{outbuf, inbuf}, bytes, res});
                taskV2[a].m->unlock();
            }
        }
    }
}
