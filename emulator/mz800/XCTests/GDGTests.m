//
//  GDGTests.m
//  MZ-800-Emulator-Tests
//
//  Created by Gunter Hager on 21.07.18.
//

#import <XCTest/XCTest.h>
#import "chips/z80.h"
#import "../mz800.h"
#import "MZFile.h"

@interface GDGTests : XCTestCase
@end

extern mz800_t mz800;

@implementation GDGTests

void (^gdgCallbackBlock)(z80_t *cpu);

void gdgCallback(z80_t *cpu) {
    gdgCallbackBlock(cpu);
}

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testCharacters {
    
    XCTestExpectation *expectation = [[XCTestExpectation alloc] initWithDescription:@"Wait for HALT"];
    
    XCTAssert([MZFile load:@"TestCharacters"]);
    
    gdgCallbackBlock = ^(z80_t *cpu) {
        [expectation fulfill];
    };
    mz800.halt_cb = gdgCallback;
    
    [self waitForExpectations:@[expectation] timeout:10];
}

- (void)testMZ700VRAM {
    
    XCTestExpectation *expectation = [[XCTestExpectation alloc] initWithDescription:@"Wait for HALT"];
    
    XCTAssert([MZFile load:@"TestMZ700VRAM"]);
    
    gdgCallbackBlock = ^(z80_t *cpu) {
        [expectation fulfill];
    };
    mz800.halt_cb = gdgCallback;
    
    [self waitForExpectations:@[expectation] timeout:10];
}

- (void)testMZ700ColorVRAM {
    
    XCTestExpectation *expectation = [[XCTestExpectation alloc] initWithDescription:@"Wait for HALT"];
    
    // Fill VRAM with character B
    memset(mz800.gdg.vram, 0x02, 0x3e8);
    
    XCTAssert([MZFile load:@"TestMZ700Color"]);
    
    gdgCallbackBlock = ^(z80_t *cpu) {
        [expectation fulfill];
    };
    mz800.halt_cb = gdgCallback;
    
    [self waitForExpectations:@[expectation] timeout:10];
}

- (void)testBorder {
    
    XCTestExpectation *expectation = [[XCTestExpectation alloc] initWithDescription:@"Wait for HALT"];
    
    XCTAssert([MZFile load:@"TestBorder"]);
    
    gdgCallbackBlock = ^(z80_t *cpu) {
        if(mz800.gdg.bcol == 0b1001) {
            [expectation fulfill];
        } else {
            XCTFail(@"TestBorder found wrong color");
        }
    };
    mz800.halt_cb = gdgCallback;
    
    [self waitForExpectations:@[expectation] timeout:10];
}



@end

