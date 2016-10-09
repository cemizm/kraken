#ifndef KRAKEN_H
#define KRAKEN_H

#include "NcqDevice.h"
#include "DeltaLookup.h"
#include "Fragment.h"

#include <vector>
#include <list>
#include <utility>
#include <map>
#include <semaphore.h>
#include <queue>
#include <string>
#include <sys/time.h>
#include "ServerCore.h"

#ifdef __APPLE__
#include "AppleSema.h"
#endif


using namespace std;

class Kraken {
public:
    Kraken(const char* config, int server_port=0);
    ~Kraken();

    void Crack(int client, const char* plaintext);
    bool Tick();

    static Kraken* getInstance() { return mInstance; } 
    void removeFragment(Fragment* frag);

    bool isUsingAti() {return mUsingAti;}

    void reportFind(string, int client);
    static void serverCmd(int, string);

    void showFragments(void);

private:
    int mNumDevices;
    vector<NcqDevice*> mDevices;
    list< pair<unsigned int, DeltaLookup*> > mTables;
    typedef list< pair<unsigned int, DeltaLookup*> >::iterator tableListIt;
    map<Fragment*,int> mFragments;
    static Kraken* mInstance;
    sem_t mMutex;
    queue<string> mWorkOrders;
    queue<int> mWorkClients;
    bool mUsingAti;
    bool mBusy;
    ServerCore* mServer;
    unsigned int mJobCounter;
    map<unsigned int, int> mJobMap;
    map<unsigned int, struct timeval> mTimingMap;
    map<unsigned int, int> mActiveMap;
    string mTableInfo;
};


#endif
