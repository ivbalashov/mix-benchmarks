#include <random>
#include <cstdint>
#include <algorithm>
#include <fstream>

#include "../Compiler.h"
#include "convolution.h"

#include "benchmark/benchmark.h"

//const long long StartSize = 419430400;        // 400mb
//const long long DataSourceSize = 10737418240; // 10gb
//const long long MaxDataSize = 3355443200;     // 3200mb 
//const long long DataSizeStep = 419430400;     // 400mb
//const long long PatternLength = 200;          // 200 bytes

const unsigned MaxMatrixHeight = 60000;     // 3433mb of full matrix
const unsigned MaxMatrixWidth = 60000;
const unsigned DataSizeStep = 10000;     // 95mb
const unsigned KernelDim = 3;


double *generate_data(long long data_size)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-100.0, 100.0);

   auto mem = new double[data_size];
   std::generate_n(mem, data_size, [dis, gen]() mutable { return (double) (dis(gen)); });

   return mem;
}

void BM_convolution(benchmark::State &state)
{
    auto pattern = (double *) generate_data(KernelDim * KernelDim);
    auto data_source = (Double *) generate_data(MaxMatrixHeight * MaxMatrixWidth);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(
            0, 
            MaxMatrixHeight * MaxMatrixWidth - state.range(0) - 1);

    auto result = new double[MaxMatrixHeight * MaxMatrixWidth];

    for (auto _: state) 
    {
        apply_convolution(
                    data_source + (unsigned) (dis(gen)), 
                    state.range(0), 
                    state.range(0), 
                    pattern, 
                    KernelDim,
                    result);
    }

    delete[] pattern;
    delete[] data_source;
    delete[] result;
}                       
BENCHMARK(BM_convolution)->DenseRange(DataSizeStep, MaxMatrixHeight, DataSizeStep);

void BM_convolution_mix(benchmark::State &state)
{
    auto pattern = (double *) generate_data(KernelDim * KernelDim);
    auto data_source = (Double *) generate_data(MaxMatrixHeight * MaxMatrixWidth);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(
            0, 
            MaxMatrixHeight * MaxMatrixWidth - state.range(0) - 1);

    auto result = new double[MaxMatrixHeight * MaxMatrixWidth];
    
    // Generate specialized function
    Compiler compiler("Naive");
    compiler.setFunction(
            apply_convolution_mix(
                &compiler.getContext(), 
                pattern, 
                KernelDim));
    auto *spec = reinterpret_cast<void (*)(const Double *, unsigned, unsigned, double *)>(compiler.compile());

    for (auto _: state)
    {
	    auto current_data = data_source + (unsigned) (dis(gen));
        auto result = spec(
                (Double *) (current_data), 
                state.range(0), 
                state.range(0),
                result);
    }

    delete[] pattern;
    delete[] data_source;
    delete[] result;
}
BENCHMARK(BM_convolution_mix)->DenseRange(DataSizeStep, MaxMatrixHeight, DataSizeStep);

