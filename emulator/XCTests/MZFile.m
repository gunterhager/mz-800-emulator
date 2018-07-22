//
//  MZFile.m
//  MZ-800-Emulator-Tests
//
//  Created by Gunter Hager on 22.07.18.
//

#import <Foundation/Foundation.h>
#import "MZFile.h"
#import "../common/fs.h"
#import "../mz800.h"
#import "../mzf.h"

extern mz800_t mz800;

@implementation MZFile

+ (BOOL)load:(NSString *)fileName {
    NSURL *url = [[NSBundle bundleForClass:[self class]] URLForResource:fileName withExtension:@"mzf"];
    NSData *data = [NSData dataWithContentsOfURL:url];
    
    const uint8_t *ptr = [data bytes];
    boolean_t result = mzf_load(ptr, [data length], &mz800.cpu, mz800.dram0);
    return result;
}

@end
