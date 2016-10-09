#define _FILE_OFFSET_BITS 64

#include "NcqDevice.h"
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>

NcqDevice::NcqDevice(const char* pzDevNode)
{
    mDevice = open(pzDevNode,O_RDONLY);
    assert(mDevice>=0);

    /* Set up free list */
    for (int i=0; i<BLOCKS; i++) {
        mMappings[i].ready = false;  /* idle */
        mMappings[i].next_free = i - 1;
    }
    mFreeMap = BLOCKS - 1;

    /* Init semaphore */
    sem_init( &mMutex, 0, 1 );

    /* Start worker thread */
    mRunning = true;
    mDevC = pzDevNode[7]; // Drive letter
    pthread_create(&mWorker, NULL, thread_stub, (void*)this);
}

    
NcqDevice::~NcqDevice()
{
    mRunning = false;
    pthread_join(mWorker, NULL);
    close(mDevice);
    sem_destroy(&mMutex);
}

void* NcqDevice::thread_stub(void* arg)
{
    if (arg) {
        NcqDevice* nd = (NcqDevice*)arg;
        nd->WorkerThread();
    }
    return NULL;
}
 
void NcqDevice::Request(class NcqRequestor* req, uint64_t blockno)
{
    sem_wait(&mMutex);
    request_t qr;
    qr.req = req;
    qr.blockno = blockno;
    mRequests.push(qr);
    sem_post(&mMutex);
}

void NcqDevice::Cancel(class NcqRequestor*)
{
}

void NcqDevice::WorkerThread()
{
    while (mRunning) {
        usleep(500);
        sem_wait(&mMutex);
        /* schedule requests */
        while ((mFreeMap>=0)&&(mRequests.size()>0))
        {
            int free = mFreeMap;
            mFreeMap = mMappings[free].next_free;
            request_t qr = mRequests.front();
            mRequests.pop();
            mMappings[free].req = qr.req;
            mMappings[free].blockno = qr.blockno;
			lseek(mDevice, qr.blockno * 4096, SEEK_SET);
			size_t r = read(mDevice , mMappings[free].addr, 4096);
			mMappings[free].ready = true;
			assert(r == 4096);
        }
        sem_post(&mMutex);

        /* Check request */
        for (int i=0; i<BLOCKS; i++) {
            if (mMappings[i].ready) {
				mMappings[i].req->processBlock(mMappings[i].addr);
				mMappings[i].ready = false;
				
				sem_wait(&mMutex);
				mMappings[i].next_free = mFreeMap;
                mFreeMap = i;
                sem_post(&mMutex);
            }
        }
    }
}



