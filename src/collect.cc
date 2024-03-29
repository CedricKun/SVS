#include "common.h"

#include "params.h"
#include "VideoCap.h"

using namespace std;
using namespace cv;

unsigned num_threads = sysconf(_SC_NPROCESSORS_CONF);;

int main(int argc, char *argv[])
{
    if(argc < 3 || argc > 4){
        std::cerr << std::endl
             << "Usage: ./bin/run_avm path_to_yaml path_to_save_path"
             << std::endl;
        return 1;
    }
    size_t num_view = 2;
    std::vector<VideoCap*> caps;
    for (size_t i = 0; i < num_view; i++)
    {
        auto v = new VideoCap(vConfigs[i], vNames[i], argv[1], argv[2]);
        caps.emplace_back(v);
    }

    std::vector<thread> threads;
    for (size_t i = 0; i < num_view; i++)
    {
        threads.emplace_back(thread(&VideoCap::Run, caps[i]));
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(i+4, &cpuset);
        int rc = pthread_setaffinity_np(threads[i].native_handle(),
                                        sizeof(cpu_set_t), &cpuset);
        if (rc != 0) {
        std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
        }
    }
    // while(1){}
    for(size_t i = 0; i < num_view; i++){
        threads[i].join();
    }
    return 0;
}