#include <sycl/sycl.hpp>
#include <iostream>
#include <cstdlib>

int main() {
    std::cout << "Testing SYCL Device Selection..." << std::endl;
    std::cout << std::endl;

    try {
        // Test 1: Device availability
        std::cout << "=== Device Availability Test ===" << std::endl;
        
        // Try to get GPU device
        sycl::device gpu_device;
        bool gpu_available = false;
        try {
            gpu_device = sycl::device(sycl::gpu_selector_v);
            std::cout << "✓ GPU available: " << gpu_device.get_info<sycl::info::device::name>() << std::endl;
            gpu_available = true;
        } catch (const sycl::exception& e) {
            std::cout << "✗ CPU not available: " << e.what() << std::endl;
        }

        // Try to get CPU device
        sycl::device cpu_device;
        bool cpu_available = false;
        try {
            cpu_device = sycl::device(sycl::cpu_selector_v);
            std::cout << "✓ CPU available: " << cpu_device.get_info<sycl::info::device::name>() << std::endl;
            cpu_available = true;
        } catch (const sycl::exception& e) {
            std::cout << "✗ CPU not available: " << e.what() << std::endl;
        }

        // Get default device
        sycl::device default_device;
        try {
            default_device = sycl::device(sycl::default_selector_v);
            std::cout << "✓ Default available: " << default_device.get_info<sycl::info::device::name>() << std::endl;
        } catch (const sycl::exception& e) {
            std::cout << "✗ Default device not available: " << e.what() << std::endl;
        }

        // Test 2: Fallback selection
        std::cout << std::endl << "=== Fallback Selection Test ===" << std::endl;
        
        sycl::device selected_device;
        if (gpu_available) {
            selected_device = gpu_device;
            std::cout << "✓ Selected GPU: " << selected_device.get_info<sycl::info::device::name>() << std::endl;
        } else if (cpu_available) {
            selected_device = cpu_device;
            std::cout << "✓ Selected CPU: " << selected_device.get_info<sycl::info::device::name>() << std::endl;
        } else {
            std::cout << "✗ No suitable device found" << std::endl;
            return 1;
        }

        // Test 3: Queue creation
        std::cout << std::endl << "=== Queue Creation Test ===" << std::endl;
        
        sycl::queue queue(selected_device);
        std::cout << "✓ Queue created successfully" << std::endl;

        // Test 4: Simple kernel execution
        std::cout << std::endl << "=== Simple Kernel Test ===" << std::endl;
        
        int result = 0;
        {
            sycl::buffer<int, 1> buf(&result, 1);
            queue.submit([&](sycl::handler& h) {
                auto acc = buf.get_access<sycl::access::mode::write>(h);
                h.single_task([=]() {
                    acc[0] = 42;
                });
            });
        }
        
        std::cout << "✓ Kernel executed successfully, result: " << result << std::endl;

        // Test 5: Device information
        std::cout << std::endl << "=== Device Information ===" << std::endl;
        std::cout << "Device: " << selected_device.get_info<sycl::info::device::name>() << std::endl;
        std::cout << "Vendor: " << selected_device.get_info<sycl::info::device::vendor>() << std::endl;
        std::cout << "Driver Version: " << selected_device.get_info<sycl::info::device::driver_version>() << std::endl;
        std::cout << "Compute Units: " << selected_device.get_info<sycl::info::device::max_compute_units>() << std::endl;
        std::cout << "Global Memory: " << selected_device.get_info<sycl::info::device::global_mem_size>() / (1024*1024) << " MB" << std::endl;

        std::cout << std::endl << "=== All Tests Passed ===" << std::endl;
        return 0;

    } catch (const sycl::exception& e) {
        std::cerr << "SYCL Exception: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
} 