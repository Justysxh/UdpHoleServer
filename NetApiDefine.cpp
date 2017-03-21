//
//  NetApiDefine.cpp
//  FProject
//
//  Created by wdkjapp on 2016/12/29.
//  Copyright © 2016年 UNICOMCQ. All rights reserved.
//

#include <stdio.h>
#include <cstring>
#include "NetApiDefine.h"
#include <sys/time.h>
#include <stdarg.h>



void fnetPrintf(const char* pstrFormat, ...)
{
    va_list args;
    va_start(args, pstrFormat);
    timeval tv = {0};
    gettimeofday(&tv, NULL);
    printf("%li.%li ", tv.tv_sec,tv.tv_usec);
    vprintf( pstrFormat, args);
    va_end(args);
}

