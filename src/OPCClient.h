//////////////////////////////////////////////////////
// author Daniel Clarke 2013/14

#ifndef OPCCLIENT_H
#define OPCCLIENT_H

#include <map>
#include <comdef.h>
#include <atlbase.h>
#include "OPC\ssopc.h"
#include "Trans_Input.h"

static SSOPC_COMLib g_cl;

typedef std::map<std::string, double>::iterator channelIter;

class OPCClient
{
public:

	OPCClient(std::string, std::map<std::string, double>);
	SSOPC_Group* getChannelGrp();
	std::map<std::string, double> getChannelSet();
protected:

	SSOPC_Server* opcSvr;
	SSOPC_Group *channelGrp;
	std::map<std::string, double> channelSet;
};

class OPCClientInput : public OPCClient
{
public:

	//this class will read new data 
	OPCClientInput(std::string, std::map<std::string, double>);
	void readChannels();
protected:

private:

	std::string BstrToStdString(BSTR, int cp = CP_UTF8);
	std::string& BstrToStdString(const BSTR , std::string&, int cp = CP_UTF8);
};

class OPCClientOutput : public OPCClient
{
public:

	//this class will write output data 
	OPCClientOutput(std::string, std::map<std::string, double>);
	void writeChannels();
	void spliceStateOutput(State_Output* outputs);
protected:
	
};

#endif //OPCCLIENT_H