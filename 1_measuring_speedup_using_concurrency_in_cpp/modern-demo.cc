#include <thread>
#include <stdexcept>
#include <iostream>
#include <string>
#include <cmath>
#include <chrono>

constexpr float size_factor = 10.0f;
constexpr int dataset_size = 1'00'000 * size_factor;

struct args_s {
    int start_index;
    int end_index;
    const int* data1;
    const int* data2;
    int* result;
};

class timer {
    private:
        std::chrono::_V2::system_clock::time_point start;
    public:
        timer() : start(
            std::chrono::system_clock::now()
        ) {}
        ~timer() {
            std::cout
                << "[TIME]: "
                << std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now() - start
                ).count() << "ms\n";
        }
};


class compute_unit {

    public:
        static void sequential_processor(
            int* _arr1,
            int* _arr2,
            int* _result
        ) {
            for(int index = 0; index < dataset_size; index++) {
                auto val1 = std::pow(_arr1[index],index % (dataset_size/10));
                auto val2 = std::pow(_arr2[index],index % (dataset_size/10));
                for(int j = 0; j < 1000; j++) {
                    val1 += std::pow(val1, 2) / j;
                    val2 += std::pow(val2, 2) / j;
                }
                _result[index] = val1 + val2;
            }
        }

        static void* work(
            void* _args
        ) {
            args_s *args = static_cast<args_s*>(_args);
            for(int index = args->start_index; index < args->end_index; index++) {
                auto val1 = std::pow(args->data1[index],index % (dataset_size/10));
                auto val2 = std::pow(args->data2[index],index % (dataset_size/10));
                for(int j = 0; j < 1000; j++) {
                    val1 += std::pow(val1, 2) / j;
                    val2 += std::pow(val2, 2) / j;
                }
                args->result[index] = val1 + val2;
            } return nullptr;
        }

        static void parallel_processor(
            int* _arr1,
            int* _arr2,
            int* _result
        ) {
            args_s arg1{.start_index = 0, .end_index = dataset_size / 2, .data1 = _arr1, .data2 = _arr2, .result = _result};

            cpu_set_t set1;
            pthread_attr_t attr1;
            pthread_t thread1;

            int core_1 = 0;
            
            CPU_ZERO(&set1);
            CPU_SET(core_1, &set1);

            pthread_attr_init(&attr1);
            int ret = pthread_attr_setaffinity_np(&attr1, sizeof(set1), (const cpu_set_t*)&set1);
            if (ret != 0) throw std::runtime_error("pthread_setaffinity_np");

            pthread_create(&thread1, &attr1, (void*(*)(void*))work, &arg1);
            
            std::cout << "-------- worker 1 started --------\n"; 

            args_s arg2{.start_index = dataset_size / 2, .end_index = dataset_size, .data1 = _arr1, .data2 = _arr2, .result = _result};

            cpu_set_t set2;
            pthread_attr_t attr2;
            pthread_t thread2;

            int core_2 = 1;
            
            CPU_ZERO(&set2);
            CPU_SET(core_2, &set2);

            pthread_attr_init(&attr2);
            ret = pthread_attr_setaffinity_np(&attr2, sizeof(set2), (const cpu_set_t*)&set2);
            if (ret != 0) throw std::runtime_error("pthread_setaffinity_np");

            pthread_create(&thread2, &attr2, (void*(*)(void*))work, &arg2);

            std::cout << "-------- worker 2 started --------\n"; 

            // ---- all join ---- //
            pthread_join(thread1, nullptr);
            pthread_join(thread2, nullptr);
            // ------------------ //

        }

    public:
        static void initialize(
            int* _arr
        ) {
            for(int index = 0; index < dataset_size; index++) {
                _arr[index] = index * 2;
            }
        }

};

int main(int argc, char const *argv[]) {
    
    int* arr1 = new int[dataset_size]{1};
    int* arr2 = new int[dataset_size]{1};
    int* res = new int[dataset_size]{0};

    compute_unit::initialize(arr1);
    compute_unit::initialize(arr2);

    switch(std::stoi(argv[1])) {
        case 0: {
            {
                timer t;
                std::cout << "running compute_unit::sequential_processor\n";
                compute_unit::sequential_processor(arr1, arr2, res);
            }
        } break;
        case 1: {
            {
                timer t;
                std::cout << "running compute_unit::parallel_processor\n";
                compute_unit::parallel_processor(arr1, arr2, res);
            }
        } break;
        default: {}
    }

    return 0;
}
