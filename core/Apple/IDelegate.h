//
//  IDelegate.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 19/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__IDelegate__
#define __G_Ear_core__IDelegate__

#include "stdplus.h"
#include <functional>

// a slightly obscure naming

namespace Gear
{
	using std::function;

    void icheck(const std::string &identifier, const function<void()> &success , const function<void()> &fail);
}

#endif /* defined(__G_Ear_core__IDelegate__) */
