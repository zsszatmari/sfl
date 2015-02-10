//
//  JavascriptEngineTiny.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 07/03/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "JavascriptEngineTiny.h"
#include "duktape.h"

namespace Gear
{
#define method JavascriptEngineTiny::
    
	method JavascriptEngineTiny() :
		_ctx(duk_create_heap_default())
	{
	}

	method ~JavascriptEngineTiny()
	{
		duk_destroy_heap(_ctx);
	}

    void method load(const std::string &js)
    {
    	duk_eval_string_noresult(_ctx, js.c_str());
    }

    std::string method execute(const std::string &statement)
    {
    	const char *result = NULL;
    	int err = duk_peval_string(_ctx, statement.c_str());
		if (err == 0) {
			result = duk_get_string(_ctx, -1);
		} else {
//#ifdef DEBUG
			std::cout << "error " << err << " while evaluating (" << statement.length() << "): " << statement.substr(0,320) << std::endl;
//#endif
		}

		//printf("result is: %s\n", result);
		if (result) {
			duk_pop(_ctx);
			return result;
		}
		duk_pop(_ctx);
		return "";
    }
}