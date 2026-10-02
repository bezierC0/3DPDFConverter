#ifndef CONVERSIONCALLBACKS_H
#define CONVERSIONCALLBACKS_H

#include <cstdint>

struct ConversionCallbacks
{
    void* userData = nullptr;
    void (*reportProgress)(void* userData, std::int32_t stage, double progress, const char* message) = nullptr;
    void (*reportLog)(void* userData, std::int32_t level, const char* message) = nullptr;
    bool (*isCancellationRequested)(void* userData) = nullptr;
};

#endif // CONVERSIONCALLBACKS_H
