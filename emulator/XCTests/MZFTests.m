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

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testLoadingAndExecuting {
    XCTestExpectation *expectation = [[XCTestExpectation alloc] initWithDescription:@"Wait for HALT"];
    
    XCTAssert([MZFile load:@"TestHalt"]);
    
    mzfCallbackBlock = ^(z80_t *cpu) {
        [expectation fulfill];
    };
    mz800.halt_cb = mzfCallback;
    
    [self waitForExpectations:@[expectation] timeout:30];
}

@end
