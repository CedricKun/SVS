#include "common.h"

#include "params.h"
#include "VideoCap.h"
#include "System.h"

using namespace std;
using namespace cv;


unsigned num_threads = sysconf(_SC_NPROCESSORS_CONF);;


int main(int argc, const char *argv[])
{
    if(argc < 3 || argc > 4){
        std::cerr << std::endl
             << "Usage: ./bin/run_avm path_to_yaml path_to_save_path"
             << std::endl;
        return 1;
    }
    size_t num_view = 4;
    System *AVM = new System(argv, num_view);

    std::vector<std::thread> threads;
    for (size_t i = 0; i < num_view; i++)
    {
        threads.emplace_back(std::thread(&VideoCap::Run, AVM->mCaps[i]));
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(i, &cpuset);
        int rc = pthread_setaffinity_np(threads[i].native_handle(),
                                        sizeof(cpu_set_t), &cpuset);
        if (rc != 0) {
            std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
        }
    }

    AVM->Start();

    return 0;
}

