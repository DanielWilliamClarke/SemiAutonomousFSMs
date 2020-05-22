//////////////////////////////////////////////////////
// author Daniel Clarke 2013/14

#include "OPCClient.h"

OPCClient::OPCClient(std::string serverName, std::map<std::string, double> channels)
{
	//convert the map into a bstr[]
	opcSvr = new SSOPC_Server();
	channelSet = channels;
	_bstr_t* itemSet = new _bstr_t[channelSet.size()];
	int idx = 0;

	for(channelIter it = channelSet.begin(); it != channelSet.end(); ++it, idx++)
	{
		itemSet[idx] = ("Channel1.Device1." + it->first).c_str();
	}

	_bstr_t server = CComBSTR(serverName.c_str()).Detach();
	channelGrp = opcSvr->InitServer(server);
	if(channelSet.size() > 0)
	{
		channelGrp->AddItems(itemSet, channelSet.size());
	}
}

SSOPC_Group* OPCClient::getChannelGrp()
{
	return channelGrp;
}

std::map<std::string, double> OPCClient::getChannelSet()
{
	return channelSet;
}

OPCClientInput::OPCClientInput(std::string serverName, std::map<std::string, double> channelSet) : OPCClient(serverName,  channelSet)
{
}

void OPCClientInput::readChannels()
{
	_variant_t *itemValues = getChannelGrp()->ReadAllItems();

	for(channelIter it = channelSet.begin(); it != channelSet.end(); ++it, ++itemValues)
	{
		it->second = itemValues->dblVal;//BstrToStdString(itemValues->bstrVal);
	}
}

// conversion with temp.
std::string OPCClientInput::BstrToStdString(BSTR bstr, int cp)
{
    std::string str;
    BstrToStdString(bstr, str, cp);
    return str;
}

// convert a BSTR to a std::string. 
std::string& OPCClientInput::BstrToStdString(const BSTR bstr, std::string& dst, int cp)
{
    if (!bstr)
    {
        // define NULL functionality. I just clear the target.
        dst.clear();
        return dst;
    }

    // request content length in single-chars through a terminating
    // nullchar in the BSTR. note: BSTR's support imbedded nullchars,
    // so this will only convert through the first nullchar.
    int res = WideCharToMultiByte(cp, 0, bstr, -1, NULL, 0, NULL, NULL);
    if (res > 0)
    {
        dst.resize(res);
        WideCharToMultiByte(cp, 0, bstr, -1, &dst[0], res, NULL, NULL);
    }
    else
    {    // no content. clear target
        dst.clear();
    }
    return dst;
}

OPCClientOutput::OPCClientOutput(std::string serverName, std::map<std::string, double> channelSet) : OPCClient(serverName,  channelSet)
{
}

void OPCClientOutput::writeChannels()
{
	//write
	int idx = 0;
	_variant_t* outputValue = new _variant_t[channelSet.size()];
	int* index = new int[channelSet.size()];

	for(channelIter it = channelSet.begin(); it != channelSet.end(); ++it)
	{
		outputValue[idx] = it->second;
		index[idx] = idx++;
	}

	if(channelSet.size() > 0)
	{
		getChannelGrp()->WriteItems(index, outputValue, channelSet.size());
	}
}

void OPCClientOutput::spliceStateOutput(State_Output* outputs)
{
	channelSet[outputs->channel] = outputs->value;

	if(outputs->nextOutput != nullptr)
	{
		spliceStateOutput(outputs->nextOutput);
	}
}
