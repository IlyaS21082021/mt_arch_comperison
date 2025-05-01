#include <iostream>
#include <cstdlib>
#include "mclass.h"


TMclass::TMclass(unsigned int threads, unsigned int bufsize)
{
    dataSize = bufsize;
    buf1.resize(bufsize);
    buf2.resize(bufsize);
    threadV.resize(threads);
    NN = threads*10;

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
        if (qTask1.size() == NN)
            continue;

        if (curPIn + 256 <= lastAddr)
        {
            m1.lock();
            qTask1.push({{curPIn, curPOut}, 256, XOR});
            m1.unlock();
            curPIn += 256;
            curPOut += 256;
            dataBlocks++;
        }
        else  // rest
        {
            m1.lock();
            qTask1.push({{curPIn, curPOut}, static_cast<size_t>(lastAddr - curPIn), XOR});
            m1.unlock();
            dataBlocks++;
            buf1Exported = true;
            break;
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
        while(qTask2.size() > 0)
        {
            m2.lock();
            auto [inbuf, outbuf] = qTask2.front().addrs;
            size_t bytes = qTask2.front().bytes;
            taskType = qTask2.front().taskType;
            qTask2.pop();
            m2.unlock();

            if (taskType == XOR)
            {
                bool cmp = false;
                while (!cmp)
                {
                    if (qTask1.size() == NN)
                        continue;

                    m1.lock();
                    qTask1.push({{inbuf, outbuf}, bytes, CMP});
                    m1.unlock();
                    cmp = true;
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
    for (int i = 0; i < threadV.size(); i++)
    {
        m1.lock();
        qTask1.push({{nullptr, nullptr}, 0, TR});
        m1.unlock();
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
        m1.lock();
        if (qTask1.size() == 0)
            m1.unlock();
        else
        {            
            auto [inbuf, outbuf] = qTask1.front().addrs;
            size_t bytes = qTask1.front().bytes;
            taskType = qTask1.front().taskType;
            qTask1.pop();
            m1.unlock();

            if (!inbuf)
                break;  // STOP

            if (taskType == XOR)
            {
                for (size_t i = 0; i < bytes; i++)
                {
                    outbuf[i] = inbuf[i] ^ 0xF;
                }
                m2.lock();
                qTask2.push({{outbuf, inbuf}, bytes, XOR});
                m2.unlock();
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
                m2.lock();
                qTask2.push({{outbuf, inbuf}, bytes, res});
                m2.unlock();
            }
        }
    }
}
