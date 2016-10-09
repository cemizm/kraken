#ifndef NCQ_DEVICE_H
#define NCQ_DEVICE_H

#include <stdint.h>
#include <queue>
#include <semaphore.h>

#ifdef __APPLE__
#include "AppleSema.h"
#endif

#define BLOCKS	32

#if defined(__APPLE__)
#define mmap64          mmap
#endif /* __APPLE__ */

using namespace std;

class NcqDevice;

class NcqRequestor {
public:
    NcqRequestor() {};
    virtual ~NcqRequestor() {};

private:
    friend class NcqDevice;
    virtual void processBlock(const void* pDataBlock) = 0;
};

class NcqDevice {
public:
    NcqDevice(const char* pzDevNode);
    ~NcqDevice();

    void Request(class NcqRequestor*, uint64_t blockno);
    void Cancel(class NcqRequestor*);

    typedef struct {
        class NcqRequestor* req;
        uint64_t            blockno;
    } request_t;

    typedef struct {
        class NcqRequestor* req;
        uint64_t            blockno;
        unsigned char       addr[4096];
        int                 next_free;
		bool				ready;
    } mapRequest_t;

    static void* thread_stub(void* arg);
    void WorkerThread();

private:
    int mDevice;
    mapRequest_t mMappings[BLOCKS];
    queue< request_t > mRequests;
    int mFreeMap;
    sem_t mMutex;
    pthread_t mWorker;
    bool mRunning;
    char mDevC;
};


#endif
