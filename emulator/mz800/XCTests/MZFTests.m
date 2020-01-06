//
//  MZFTests.m
//  MZ-800-Emulator-Tests
//
//  Created by Gunter Hager on 22.07.18.
//

#import <XCTest/XCTest.h>
#import "chips/z80.h"
#import "../mz800.h"
#import "MZFile.h"

@interface MZFTests : XCTestCase

@end

extern mz800_t mz800;

@implementation MZFTests

void (^mzfCallbackBlock)(z80_t *cpu);

void mzfCallback(z80_t *cpu) {
    mzfCallbackBlock(cpu);
}

- (void)testLoadingAndExecuting {
    XCTestExpectation *expectation = [[XCTestExpectation alloc] initWithDescription:@"Wait for HALT"];
    
    mzfCallbackBlock = ^(z80_t *cpu) {
        [expectation fulfill];
    };
    mz800.halt_cb = mzfCallback;
    
    XCTAssert([MZFile load:@"TestHalt"]);
    
    [self waitForExpectations:@[expectation] timeout:30];
}

@end
