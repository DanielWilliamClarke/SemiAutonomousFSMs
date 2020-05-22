#include <iostream>
#include <sstream>
#include <fstream>
#include <comdef.h>
#include "ssopc.h"
#include <math.h>

using namespace std;

static SSOPC_COMLib g_cl;

// ---------------------------
_bstr_t items[] = {
	"Channel1.Device1.MatlabOutputsA",
	"Channel1.Device1.MatlabOutputsB"
};
// ---------------------------

// convert a BSTR to a std::string. 
std::string& BstrToStdString(const BSTR bstr, std::string& dst, int cp = CP_UTF8)
{
    if (!bstr)
    {
        // define NULL functionality. I just clear the target.
        dst.clear();
        return dst;
    }

    // request content length in single-chars through a terminating
    //  nullchar in the BSTR. note: BSTR's support imbedded nullchars,
    //  so this will only convert through the first nullchar.
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

// conversion with temp.
std::string BstrToStdString(BSTR bstr, int cp = CP_UTF8)
{
    std::string str;
    BstrToStdString(bstr, str, cp);
    return str;
}

int main()
{		
	cout << "hello opc!" << endl;

	try
	{
		SSOPC_Server opcSvr;
		SSOPC_Group *outputGrp;
		SSOPC_Group *inputGrp;
			
		/*outputGrp = opcSvr.InitServer("Kepware.KEPServerEX.V5");
		outputGrp->AddItems(items, 2);*/

		inputGrp = opcSvr.InitServer("Kepware.KEPServerEX.V5");
		inputGrp->AddItems(items, 2);

		int i = 0;

		while(true)
		{	
			Sleep(1000);

			////write
			//_variant_t v[2];
			//v[0] = cos(i) + (i * 10);
			//v[1] = 10 * i;

			//int index[2] = {0, 1};
			//outputGrp->WriteItems(index, v, 2);
			//i++;
			//std::cout << "writing value " << i << std::endl;
			
			// read
			_variant_t *itemValues = inputGrp->ReadAllItems();
			//returns an array as pointer

			for(int i = 0; i < inputGrp->getNumOfItems(); i++, ++itemValues)
			{
				std::cout << "value " << i << " returned from server = " << BstrToStdString(itemValues->bstrVal) << std::endl;
			}

			//std::cout << "value 2 returned from server = " << itemValues->dblVal << std::endl;
		}
	}
	catch(SSOPC_Exception &ex)
	{
		cout << ex.ErrDesc() << endl;
	}

	return 0;
}
