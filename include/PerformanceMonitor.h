// PerformanceMonitor.h
// Optional performance monitoring utilities for debugging and optimization

#ifndef PERFORMANCE_MONITOR_H
#define PERFORMANCE_MONITOR_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Enable/disable performance monitoring
// #define ENABLE_PERFORMANCE_MONITORING

#ifdef ENABLE_PERFORMANCE_MONITORING

/**
 * @brief Simple performance timer for measuring code execution time
 */
class PerformanceTimer {
private:
    unsigned long startTime;
    const char *label;
    
public:
    PerformanceTimer(const char *name) : label(name) {
        startTime = micros();
    }
    
    ~PerformanceTimer() {
        unsigned long elapsed = micros() - startTime;
        if (elapsed > 1000) {
            Serial.printf("[PERF] %s: %lu ms\n", label, elapsed / 1000);
        } else {
            Serial.printf("[PERF] %s: %lu µs\n", label, elapsed);
        }
    }
};

/**
 * @brief Stack usage monitor for FreeRTOS tasks
 */
class StackMonitor {
public:
    static void printTaskStack(const char *taskName, TaskHandle_t taskHandle = nullptr) {
        if (taskHandle == nullptr) {
            taskHandle = xTaskGetCurrentTaskHandle();
        }
        
        UBaseType_t highWaterMark = uxTaskGetStackHighWaterMark(taskHandle);
        Serial.printf("[STACK] %s: %u bytes free (high water mark)\n", 
                     taskName, highWaterMark * sizeof(StackType_t));
    }
    
    static void printAllStacks() {
        Serial.println("\n=== Stack Usage Report ===");
        // Note: This requires task handles to be stored globally
        Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
        Serial.printf("Min free heap: %u bytes\n", ESP.getMinFreeHeap());
        Serial.printf("Heap size: %u bytes\n", ESP.getHeapSize());
        Serial.println("========================\n");
    }
};

/**
 * @brief Memory leak detector
 */
class HeapMonitor {
private:
    uint32_t lastHeap;
    uint32_t minHeap;
    const char *label;
    
public:
    HeapMonitor(const char *name) : label(name) {
        lastHeap = ESP.getFreeHeap();
        minHeap = ESP.getMinFreeHeap();
    }
    
    void checkpoint() {
        uint32_t currentHeap = ESP.getFreeHeap();
        int32_t delta = (int32_t)currentHeap - (int32_t)lastHeap;
        
        if (delta < -1024) {  // More than 1KB lost
            Serial.printf("[HEAP] %s: Lost %d bytes (now: %u, min: %u)\n",
                         label, -delta, currentHeap, ESP.getMinFreeHeap());
        }
        
        lastHeap = currentHeap;
    }
    
    static void printHeapStats() {
        Serial.printf("[HEAP] Free: %u, Min: %u, Size: %u, Frag: %d%%\n",
                     ESP.getFreeHeap(),
                     ESP.getMinFreeHeap(),
                     ESP.getHeapSize(),
                     100 - (ESP.getFreeHeap() * 100 / ESP.getHeapSize()));
    }
};

// Convenience macros
#define PERF_TIMER(name) PerformanceTimer __perf_##name(#name)
#define STACK_CHECK(name) StackMonitor::printTaskStack(name)
#define HEAP_CHECK() HeapMonitor::printHeapStats()

#else

// No-op macros when disabled
#define PERF_TIMER(name)
#define STACK_CHECK(name)
#define HEAP_CHECK()

class PerformanceTimer {
public:
    PerformanceTimer(const char *) {}
};

class StackMonitor {
public:
    static void printTaskStack(const char *, TaskHandle_t = nullptr) {}
    static void printAllStacks() {}
};

class HeapMonitor {
public:
    HeapMonitor(const char *) {}
    void checkpoint() {}
    static void printHeapStats() {}
};

#endif // ENABLE_PERFORMANCE_MONITORING

#endif // PERFORMANCE_MONITOR_H
