#import <XCTest/XCTest.h>
#import "chips/z80.h"
#import "../mz800.h"

@interface GDGTests : XCTestCase
extern mz800_t mz800;
@end

static void (^callbackBlock)(z80_t *cpu);

void callback(z80_t *cpu) {
    callbackBlock(cpu);
}

@implementation GDGTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testHalt {
    XCTestExpectation *expectation = [[XCTestExpectation alloc] initWithDescription:@"Wait for HALT"];
    
    callbackBlock = ^(z80_t *cpu) {
        [expectation fulfill];
    };
    mz800.halt_cb = callback;
    
    [self waitForExpectations:@[expectation] timeout:30];
}

- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

@end

