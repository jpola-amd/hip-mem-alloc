#include <hip/hip_runtime_api.h>

#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr std::size_t kAllocationBytes = std::size_t{1} << 30;  // 1 GiB
constexpr std::uint64_t kGiB = std::uint64_t{1} << 30;

void print_hip_error(const char* operation, hipError_t error) {
    std::cerr << operation << " failed: " << hipGetErrorName(error) << " ("
              << static_cast<int>(error) << "): " << hipGetErrorString(error) << '\n';
}

bool parse_device_id(const char* text, int& device_id) {
    errno = 0;
    char* end = nullptr;
    const long value = std::strtol(text, &end, 10);

    if(errno == ERANGE || end == text || *end != '\0' || value < 0 || value > INT_MAX) {
        return false;
    }

    device_id = static_cast<int>(value);
    return true;
}

bool release_allocations(std::vector<void*>& allocations) {
    bool all_freed = true;

    for(auto allocation = allocations.rbegin(); allocation != allocations.rend(); ++allocation) {
        const hipError_t status = hipFree(*allocation);
        if(status != hipSuccess) {
            print_hip_error("hipFree", status);
            all_freed = false;
        }
    }

    allocations.clear();
    return all_freed;
}

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [device-id]\n"
              << "Allocate 1 GiB HIP host-runtime allocations until hipMalloc returns an error.\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    if(argc > 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    int device_id = 0;
    if(argc == 2 && !parse_device_id(argv[1], device_id)) {
        std::cerr << "Invalid device ID: " << argv[1] << '\n';
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    hipError_t status = hipInit(0);
    if(status != hipSuccess) {
        print_hip_error("hipInit", status);
        return EXIT_FAILURE;
    }

    int device_count = 0;
    status = hipGetDeviceCount(&device_count);
    if(status != hipSuccess) {
        print_hip_error("hipGetDeviceCount", status);
        return EXIT_FAILURE;
    }

    if(device_id >= device_count) {
        std::cerr << "Requested device " << device_id << " is unavailable; this system has "
                  << device_count << " HIP device(s).\n";
        return EXIT_FAILURE;
    }

    status = hipSetDevice(device_id);
    if(status != hipSuccess) {
        print_hip_error("hipSetDevice", status);
        return EXIT_FAILURE;
    }

    hipDeviceProp_t device_properties{};
    status = hipGetDeviceProperties(&device_properties, device_id);
    if(status != hipSuccess) {
        print_hip_error("hipGetDeviceProperties", status);
        return EXIT_FAILURE;
    }

    std::cout << "Using HIP device " << device_id << ": " << device_properties.name << '\n'
              << "Allocating " << (kAllocationBytes / kGiB) << " GiB blocks until hipMalloc fails.\n";

    std::vector<void*> allocations;
    hipError_t allocation_status = hipSuccess;

    while(allocation_status == hipSuccess) {
        void* allocation = nullptr;
        allocation_status = hipMalloc(&allocation, kAllocationBytes);
        if(allocation_status == hipSuccess) {
            allocations.push_back(allocation);
        }
    }

    const std::uint64_t successful_allocations = allocations.size();
    const std::uint64_t total_allocated_bytes = successful_allocations * kAllocationBytes;

    std::cout << "hipMalloc stopped after " << successful_allocations
              << " successful allocation(s).\n"
              << "Total memory allocated: " << total_allocated_bytes << " bytes ("
              << (total_allocated_bytes / kGiB) << " GiB).\n";
    print_hip_error("hipMalloc", allocation_status);

    const bool freed_all_allocations = release_allocations(allocations);
    std::cout << "Released " << successful_allocations << " allocation(s).\n";

    // Exhausting memory is the normal termination condition for this tool.
    // Any other allocation or cleanup failure produces a non-zero exit status.
    if(allocation_status != hipErrorOutOfMemory || !freed_all_allocations) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}