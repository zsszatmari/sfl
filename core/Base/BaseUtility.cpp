//
//  BaseUtility.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/12/13.
//
//

#include "stdplus.h"
#include "BaseUtility.h"

namespace Base
{
#ifdef USE_BOOST_LOCALES
    std::locale utfLocale;
#endif

    void localeInit()
    {
#ifdef USE_BOOST_LOCALES
        boost::locale::generator gen;
        utfLocale = gen("en_US.UTF-8");
#endif
    }
}

#ifdef BOOST_NO_EXCEPTIONS
namespace boost
{	
	void throw_exception( std::exception const & e )
	{
		throw e;
	}
}
#endif