/**
 * @file test_coretypes.cpp
 * @brief Simple verification test for CoreTypes library
 * 
 * This file verifies that all CoreTypes definitions compile correctly
 * and can be used as intended.
 */

#include "CoreTypes.h"
#include <iostream>
#include <cassert>

using namespace CoreTypes;

void testResultCode() {
    // Test ResultCode enumeration
    ResultCode success = ResultCode::Success;
    ResultCode fileNotFound = ResultCode::FileNotFound;
    ResultCode invalidFormat = ResultCode::InvalidFormat;
    ResultCode meshFailed = ResultCode::MeshGenerationFailed;
    ResultCode prcFailed = ResultCode::PrcExportFailed;
    ResultCode pdfFailed = ResultCode::PdfExportFailed;
    ResultCode unknown = ResultCode::UnknownError;
    
    assert(static_cast<int>(success) == 0);
    assert(static_cast<int>(fileNotFound) == 1);
    assert(static_cast<int>(invalidFormat) == 2);
    assert(static_cast<int>(meshFailed) == 3);
    assert(static_cast<int>(prcFailed) == 4);
    assert(static_cast<int>(pdfFailed) == 5);
    assert(static_cast<int>(unknown) == 99);
    
    std::cout << "✓ ResultCode enumeration test passed" << std::endl;
}

void testPageSize() {
    // Test PageSize enumeration
    PageSize a4 = PageSize::A4;
    PageSize a3 = PageSize::A3;
    PageSize letter = PageSize::Letter;
    PageSize legal = PageSize::Legal;
    
    // Just verify they compile and can be assigned
    assert(a4 == PageSize::A4);
    assert(a3 == PageSize::A3);
    assert(letter == PageSize::Letter);
    assert(legal == PageSize::Legal);
    
    std::cout << "✓ PageSize enumeration test passed" << std::endl;
}

void testLightingMode() {
    // Test LightingMode enumeration
    LightingMode artwork = LightingMode::Artwork;
    LightingMode cad = LightingMode::CAD;
    LightingMode headlamp = LightingMode::Headlamp;
    LightingMode none = LightingMode::None;
    
    assert(artwork == LightingMode::Artwork);
    assert(cad == LightingMode::CAD);
    assert(headlamp == LightingMode::Headlamp);
    assert(none == LightingMode::None);
    
    std::cout << "✓ LightingMode enumeration test passed" << std::endl;
}

void testViewAngle() {
    // Test ViewAngle enumeration
    ViewAngle front = ViewAngle::Front;
    ViewAngle back = ViewAngle::Back;
    ViewAngle top = ViewAngle::Top;
    ViewAngle bottom = ViewAngle::Bottom;
    ViewAngle left = ViewAngle::Left;
    ViewAngle right = ViewAngle::Right;
    ViewAngle isometric = ViewAngle::Isometric;
    
    assert(front == ViewAngle::Front);
    assert(back == ViewAngle::Back);
    assert(top == ViewAngle::Top);
    assert(bottom == ViewAngle::Bottom);
    assert(left == ViewAngle::Left);
    assert(right == ViewAngle::Right);
    assert(isometric == ViewAngle::Isometric);
    
    std::cout << "✓ ViewAngle enumeration test passed" << std::endl;
}

void testMeshSettings() {
    // Test MeshSettings POD structure with defaults
    MeshSettings settings1;
    assert(settings1.deflection == 0.001);
    assert(settings1.angle == 0.5);
    assert(settings1.relative == false);
    assert(settings1.inParallel == true);
    
    // Test custom values
    MeshSettings settings2;
    settings2.deflection = 0.01;
    settings2.angle = 0.3;
    settings2.relative = true;
    settings2.inParallel = false;
    
    assert(settings2.deflection == 0.01);
    assert(settings2.angle == 0.3);
    assert(settings2.relative == true);
    assert(settings2.inParallel == false);
    
    std::cout << "✓ MeshSettings structure test passed" << std::endl;
}

void testPdfSettings() {
    // Test PdfSettings POD structure with defaults
    PdfSettings settings1;
    assert(settings1.pageSize == PageSize::A4);
    assert(settings1.lighting == LightingMode::CAD);
    assert(settings1.defaultView == ViewAngle::Isometric);
    assert(settings1.backgroundColor == 0xFFFFFF);
    
    // Test custom values
    PdfSettings settings2;
    settings2.pageSize = PageSize::A3;
    settings2.lighting = LightingMode::Headlamp;
    settings2.defaultView = ViewAngle::Front;
    settings2.backgroundColor = 0x000000;
    
    assert(settings2.pageSize == PageSize::A3);
    assert(settings2.lighting == LightingMode::Headlamp);
    assert(settings2.defaultView == ViewAngle::Front);
    assert(settings2.backgroundColor == 0x000000);
    
    std::cout << "✓ PdfSettings structure test passed" << std::endl;
}

void testExportSettings() {
    // Test ExportSettings POD structure
    ExportSettings settings;
    
    // Verify default values from nested structures
    assert(settings.meshSettings.deflection == 0.001);
    assert(settings.pdfSettings.pageSize == PageSize::A4);
    
    // Test custom values
    settings.meshSettings.deflection = 0.005;
    settings.meshSettings.angle = 0.7;
    settings.pdfSettings.pageSize = PageSize::Letter;
    settings.pdfSettings.lighting = LightingMode::Artwork;
    
    assert(settings.meshSettings.deflection == 0.005);
    assert(settings.meshSettings.angle == 0.7);
    assert(settings.pdfSettings.pageSize == PageSize::Letter);
    assert(settings.pdfSettings.lighting == LightingMode::Artwork);
    
    std::cout << "✓ ExportSettings structure test passed" << std::endl;
}

void testOperationResult() {
    // Test OperationResult with success
    OperationResult result1;
    result1.code = ResultCode::Success;
    result1.message = "";
    assert(result1.IsSuccess() == true);
    
    // Test OperationResult with failure
    OperationResult result2;
    result2.code = ResultCode::FileNotFound;
    result2.message = "File not found: test.step";
    assert(result2.IsSuccess() == false);
    assert(result2.message == "File not found: test.step");
    
    // Test all error codes
    OperationResult result3;
    result3.code = ResultCode::InvalidFormat;
    assert(result3.IsSuccess() == false);
    
    result3.code = ResultCode::MeshGenerationFailed;
    assert(result3.IsSuccess() == false);
    
    result3.code = ResultCode::PrcExportFailed;
    assert(result3.IsSuccess() == false);
    
    result3.code = ResultCode::PdfExportFailed;
    assert(result3.IsSuccess() == false);
    
    result3.code = ResultCode::UnknownError;
    assert(result3.IsSuccess() == false);
    
    std::cout << "✓ OperationResult structure test passed" << std::endl;
}

int main() {
    std::cout << "Running CoreTypes verification tests..." << std::endl;
    std::cout << std::endl;
    
    try {
        testResultCode();
        testPageSize();
        testLightingMode();
        testViewAngle();
        testMeshSettings();
        testPdfSettings();
        testExportSettings();
        testOperationResult();
        
        std::cout << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "All CoreTypes tests passed successfully!" << std::endl;
        std::cout << "========================================" << std::endl;
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
