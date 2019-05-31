#include <iostream>
#include <string>
#include <vector>
#include <queue>

#include <cstdlib>
#include <cmath>
#include <cstdarg>

#include <pthread.h>

#define N_NODES 4
#define N_FRAMES 3

using namespace std;

// Data packet encapsulated in PhyGram
struct Packet
{
    bool isFwd;
    long addrHash;
    string data;
};

// Datagram on physical layer.
struct PhyGram
{
    int src, dst;
    Packet pkt;
};

// Node configuration.
struct NodeCfg
{
    int nodeId;
    const long *frmCount, *frmTtl;  // Current frame count and time-to-live
    pthread_barrier_t *frmSync, *frmRel;    // Sync barriers
    pthread_mutex_t *logMutex;  // Logging mutex
};

// Helper function for printing log.
inline void prnLog(pthread_mutex_t *mutex, const char *fmt, ...)
{
    va_list args;
    pthread_mutex_lock(mutex);
    va_start (args, fmt);
    vprintf(fmt, args);
    va_end (args);
    pthread_mutex_unlock(mutex);
}

// Message exchange matrix.
// Node i will push its messages to node j in
// comChs[i][j], and receive all its message
// from comChs[i][i].
queue<PhyGram> comChs[N_NODES][N_NODES];

void msgPosting()
{
    for (int i = 0; i < N_NODES; i++)
        for (int j = 0; j < N_NODES; j++)
            if (i != j) {
                while (!comChs[i][j].empty()) {
                    comChs[j][j].push(comChs[i][j].front());
                    comChs[i][j].pop();
                }
            }
    return;
}

// Normal node process routine.
void *nodeProcess(void *nodeConfig)
{
    NodeCfg *nc = (NodeCfg *)nodeConfig;
    int id = nc->nodeId;
    long frm;
    const long ttl = *(nc->frmTtl);
    auto &chnl = comChs[id];
    auto &mut = nc->logMutex;

    // Node initialization
    prnLog(mut, "Node %d initialized.\n", id);

    do {
        pthread_barrier_wait(nc->frmRel);
        frm = *(nc->frmCount);

        // Node loop
        prnLog(mut, "Node %d reports on frame %ld.\n", 
                id, frm);
        while (!chnl[id].empty()) {
            prnLog(mut, "Node %d received '%s' from %d.\n", id,
                    chnl[id].front().pkt.data.c_str(), chnl[id].front().src);
            chnl[id].pop();
        }

        pthread_barrier_wait(nc->frmSync);
    } while (frm + 1 < ttl);

    prnLog(mut, "Node %d quit.\n", id);

    return NULL;
}

int main()
{
    void *thrdRetVal;

    const long frmTtl = N_FRAMES;
    long frmCount;
    pthread_attr_t thrdAttr;
    pthread_barrier_t frmSync, frmRel, finish;
    pthread_mutex_t logMutex;
    std::vector<pthread_t *> thrds;
    std::vector<NodeCfg *> nodeCfgs;

    pthread_mutex_init(&logMutex, NULL);
    pthread_barrier_init(&frmSync, NULL, N_NODES + 1);
    pthread_barrier_init(&frmRel, NULL, N_NODES + 1);

    prnLog(&logMutex, "Starting...\n");

    comChs[0][1].push(PhyGram{
        .src = 0,
        .dst = 1,
        .pkt = Packet{
            .isFwd = false,
            .addrHash = 0,
            .data = "hello"
        }
    });

    for (int i = 0; i < N_NODES; i++) {
        
        auto t = new pthread_t;
        auto nc = new NodeCfg {
            .nodeId = i,
            .frmCount = &frmCount,
            .frmTtl = &frmTtl,
            .frmSync = &frmSync,
            .frmRel = &frmRel,
            .logMutex = &logMutex
        };

        thrds.push_back(t);
        nodeCfgs.push_back(nc);

        pthread_attr_init(&thrdAttr);
        pthread_create(t, &thrdAttr, &nodeProcess, (void *)nc);
    }

    for (frmCount = 0; frmCount < frmTtl; frmCount++) {
        prnLog(&logMutex, "Posting messages...");
        msgPosting();
        prnLog(&logMutex, "Done\n");
        pthread_barrier_wait(&frmRel);
        pthread_barrier_wait(&frmSync);
    }

    prnLog(&logMutex, "Main finished.\n");
    
    for (auto t : thrds) {
        pthread_join(*t, &thrdRetVal);
        delete t;
    }

    for (auto ncfg : nodeCfgs) {
        delete ncfg;
    }

    pthread_mutex_destroy(&logMutex);
    pthread_barrier_destroy(&frmSync);
    pthread_barrier_destroy(&frmRel);

    return 0;
}