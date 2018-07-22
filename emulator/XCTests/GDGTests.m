#import <XCTest/XCTest.h>
#import "chips/z80.h"
#import "../common/fs.h"
#import "../mz800.h"
#import "../mzf.h"

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
    
    NSURL *url = [[NSBundle bundleForClass:[self class]] URLForResource:@"WriteCharacter" withExtension:@"mzf"];
    XCTAssertNotNil(url);
    NSData *data = [NSData dataWithContentsOfURL:url];
    XCTAssertNotNil(data);
    
    const uint8_t *ptr = [data bytes];
    XCTAssert(ptr != NULL);
    boolean_t result = mzf_load(ptr, [data length], &mz800.cpu, mz800.dram0);
    XCTAssert(result);
    
    callbackBlock = ^(z80_t *cpu) {
        [expectation fulfill];
    };
    mz800.halt_cb = callback;
    
    [self waitForExpectations:@[expectation] timeout:30];
}

@end

