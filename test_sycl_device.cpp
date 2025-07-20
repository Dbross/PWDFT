#include <sycl/sycl.hpp>
#include <iostream>

int main() {
    std::cout << "Testing SYCL Device Selection..." << std::endl;
    
    // Test device availability
    std::cout << "\n=== Device Availability Test ===" << std::endl;
    
    // Check GPU
    try {
        sycl::device gpu_device(sycl::gpu_selector_v);
        std::cout << "✓ GPU available: " << gpu_device.get_info<sycl::info::device::name>() << std::endl;
    } catch (const sycl::exception& e) {
        std::cout << "✗ GPU not available: " << e.what() << std::endl;
    }
    
    // Check CPU
    try {
        sycl::device cpu_device(sycl::cpu_selector_v);
        std::cout << "✓ CPU available: " << cpu_device.get_info<sycl::info::device::name>() << std::endl;
    } catch (const sycl::exception& e) {
        std::cout << "✗ CPU not available: " << e.what() << std::endl;
    }
    
    // Check Default
    try {
        sycl::device default_device(sycl::default_selector_v);
        std::cout << "✓ Default available: " << default_device.get_info<sycl::info::device::name>() << std::endl;
    } catch (const sycl::exception& e) {
        std::cout << "✗ Default not available: " << e.what() << std::endl;
    }
    
    // Test fallback selection
    std::cout << "\n=== Fallback Selection Test ===" << std::endl;
    
    sycl::device selected_device;
    bool has_gpu = false;
    
    try {
        selected_device = sycl::device(sycl::gpu_selector_v);
        std::cout << "✓ Selected GPU: " << selected_device.get_info<sycl::info::device::name>() << std::endl;
        has_gpu = true;
    } catch (const sycl::exception& e) {
        std::cout << "✗ GPU selection failed, trying CPU..." << std::endl;
        try {
            selected_device = sycl::device(sycl::cpu_selector_v);
            std::cout << "✓ Selected CPU: " << selected_device.get_info<sycl::info::device::name>() << std::endl;
        } catch (const sycl::exception& e2) {
            std::cout << "✗ CPU selection failed, trying default..." << std::endl;
            try {
                selected_device = sycl::device(sycl::default_selector_v);
                std::cout << "✓ Selected Default: " << selected_device.get_info<sycl::info::device::name>() << std::endl;
            } catch (const sycl::exception& e3) {
                std::cout << "✗ All device selection failed!" << std::endl;
                std::cout << "  GPU error: " << e.what() << std::endl;
                std::cout << "  CPU error: " << e2.what() << std::endl;
                std::cout << "  Default error: " << e3.what() << std::endl;
                return 1;
            }
        }
    }
    
    // Test queue creation
    std::cout << "\n=== Queue Creation Test ===" << std::endl;
    
    try {
        auto asyncHandler = [&](sycl::exception_list eL) {
            for (auto &e : eL) {
                try {
                    std::rethrow_exception(e);
                } catch (sycl::exception &e) {
                    std::cout << "Queue exception: " << e.what() << std::endl;
                }
            }
        };
        
        sycl::queue test_queue(selected_device, asyncHandler);
        std::cout << "✓ Queue created successfully" << std::endl;
        
        // Test simple kernel
        std::cout << "\n=== Simple Kernel Test ===" << std::endl;
        int result = 0;
        {
            sycl::buffer<int, 1> buf(&result, 1);
            test_queue.submit([&](sycl::handler& h) {
                auto acc = buf.get_access<sycl::access::mode::write>(h);
                h.single_task([=]() {
                    acc[0] = 42;
                });
            });
        }
        std::cout << "✓ Kernel executed successfully, result: " << result << std::endl;
        
    } catch (const sycl::exception& e) {
        std::cout << "✗ Queue creation failed: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== All Tests Passed ===" << std::endl;
    return 0;
} 