#ifndef FAKEOAUTHCONNECTION_H
#define FAKEOAUTHCONNECTION_H

#include "OAuthconnection.h"

using namespace Gear;

class FakeOAuthConnection : public OAuthConnection
{
public:
	FakeOAuthConnection() :
		OAuthConnection("", "", "", "", "", "")
	{
	}
};

#endif
