#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "ssopc.h"
#include "opcerror.h"

using namespace std;

//----------------------------------------------------------

SSOPC_Exception::SSOPC_Exception(string errTxt)
{
	m_errTxt = errTxt;
}

string SSOPC_Exception::ErrDesc()
{
	return m_errTxt;
}

//----------------------------------------------------------

SSOPC_COMLib::SSOPC_COMLib()
{
	//	Initialize the COM library
	HRESULT hr = CoInitialize(NULL);
	if(FAILED(hr))
	{	
		stringstream ss;
		ss << "Initialisation of COM Library failed, hr = " << hr;
		cout << ss.str() << endl;
		exit(1);
	}
}

SSOPC_COMLib::~SSOPC_COMLib()
{
	//	Uninit COM
	CoUninitialize();
}

//----------------------------------------------------------

SSOPC_Group::SSOPC_Group(SSOPC_Server *ownedOpcSvr)
{
	m_ownedServer = ownedOpcSvr;
	m_IOPCItemMgt = NULL;
	m_hServerGroup = NULL;
	m_IOPCSyncIO = NULL;
	m_hServerItems = NULL;
	m_numOfItems = 0;
	m_itemValues = NULL;
}

SSOPC_Group::~SSOPC_Group()
{
	if(m_IOPCSyncIO != NULL) m_IOPCSyncIO->Release();
	if(m_IOPCItemMgt != NULL) m_IOPCItemMgt->Release();

	if(m_hServerGroup != NULL)
	{
		//	Remove Group
		IOPCServer *pIOPCServer = m_ownedServer->GetIOPCServer();
		
		HRESULT hr = pIOPCServer->RemoveGroup(m_hServerGroup, FALSE);
		if (hr != S_OK)
		{	
			stringstream ss;
			ss << "RemoveGroup failed, hr = " << hr;
			throw SSOPC_Exception(ss.str());
		}
	}

	if(m_hServerItems != NULL) delete [] m_hServerItems;
	if(m_itemValues != NULL) delete [] m_itemValues; 
}

void SSOPC_Group::InitGroup()
{
	//	Add a group object with a unique name
	HRESULT hr;

	OPCHANDLE 
		clientHandle = 1;
	
	DWORD 
		reqUptRate = 5000, 
		revisedUptRate;
	
	DWORD 
		lcid = 0x409; // Code 0x409 = ENGLISH

	IOPCItemMgt *itemMgt = NULL;
	OPCHANDLE grpHdl = NULL;
	IOPCServer *pIOPCServer = m_ownedServer->GetIOPCServer();

	hr = pIOPCServer->AddGroup(L"",						
						FALSE,							
						reqUptRate,						
						clientHandle,					
						NULL,							
						NULL,							
						lcid,							
						&m_hServerGroup,				
						&revisedUptRate,				
					    IID_IOPCItemMgt,				
						(LPUNKNOWN*)&m_IOPCItemMgt);	
	if(FAILED(hr))
	{
		stringstream ss;
		ss << "Can't add Group to Server, hr = " << hr;
		throw SSOPC_Exception(ss.str());
	}

	//	query interface for sync calls on group object
	hr = m_IOPCItemMgt->QueryInterface(IID_IOPCSyncIO, (void**)&m_IOPCSyncIO);
	if(FAILED(hr))
	{	
		string errTxt = "No IOPCSyncIO found!";
		throw SSOPC_Exception(errTxt);
	}
}

void SSOPC_Group::AddItems(_bstr_t itemNames[], int num)
{

	OPCITEMDEF *itemDefs = NULL;
	OPCITEMRESULT *pAddResult = NULL;
	HRESULT *pErrors = NULL;
	HRESULT hr;
	stringstream ss;
	
	m_numOfItems = num;

	itemDefs = new OPCITEMDEF[m_numOfItems];
	m_itemValues = new _variant_t[m_numOfItems];

	for(int i=0; i<m_numOfItems; i++)
	{
		//	define an item table with one item as in-paramter for AddItem 
		itemDefs[i].szAccessPath		  = _bstr_t("");			//	Accesspath not needed
		itemDefs[i].szItemID			  = itemNames[i];		//	ItemID
		itemDefs[i].bActive			  = FALSE;			
		itemDefs[i].hClient			  = 1;
		itemDefs[i].dwBlobSize		  = 0;
		itemDefs[i].pBlob			  = NULL;
		itemDefs[i].vtRequestedDataType = 0;				//	return values in native (cannonical) datatype 
														//	defined by the item itself
	}

	hr = m_IOPCItemMgt->AddItems(m_numOfItems, itemDefs, &pAddResult, &pErrors);	
	if(hr != S_OK)
	{
		
		ss << "AddItems failed, hr = " << hr;
		
		if( hr == S_FALSE)
		{
			for(int i=0; i<m_numOfItems; i++)
			{
				if(pErrors[i] != S_OK)
					ss << "sub hr[" << i << "] = " << pErrors[i];
			}
		}
	}
	else
	{
		m_hServerItems = new OPCHANDLE[m_numOfItems];

		for(int i=0; i<num; i++)
			m_hServerItems[i] = pAddResult[i].hServer;
	}

	if(pAddResult != NULL)
	{
		for(int i=0; i<m_numOfItems; i++)
		{
			if(pAddResult[i].pBlob != NULL) 
			{
				CoTaskMemFree(pAddResult->pBlob);
			}
		}

		CoTaskMemFree(pAddResult);
	}

	if(pErrors != NULL) CoTaskMemFree(pErrors);
	if(itemDefs != NULL) delete [] itemDefs;
	if(hr != S_OK) throw SSOPC_Exception(ss.str());
}

_variant_t *SSOPC_Group::ReadAllItems()
{
	HRESULT hr; 
	OPCITEMSTATE *pItemValues = NULL;
	HRESULT *pErrors = NULL;
	stringstream ss;

	hr = m_IOPCSyncIO->Read(OPC_DS_DEVICE, m_numOfItems, m_hServerItems, &pItemValues,&pErrors);
	if(hr != S_OK) 
	{
		ss << "ReadItems failed, hr = " << hr;
		
		if( hr == S_FALSE)
		{
			for(int i=0; i<m_numOfItems; i++)
			{
				if(pErrors[i] != S_OK)
					ss << "sub hr[" << i << "] = " << pErrors[i];
			}
		}			
	}
	else
	{
		for(int i=0; i<m_numOfItems; i++)
		{
			m_itemValues[i] = pItemValues[i].vDataValue;
		}
	}
	
	for(int i=0; i<m_numOfItems; i++)
	{
		VariantClear(&pItemValues[i].vDataValue);
	}

	if(pItemValues != NULL) CoTaskMemFree(pItemValues);
	if(pErrors != NULL) CoTaskMemFree(pErrors);
	if(hr != S_OK) throw SSOPC_Exception(ss.str());

	return m_itemValues;
}

void SSOPC_Group::WriteItems(int itemIndex[], _variant_t values[], int num)
{

	HRESULT *pErrors = NULL; 
	HRESULT hr; 
	OPCHANDLE *phItems = NULL;
	stringstream ss;

	phItems = new OPCHANDLE[num];

	for(int i=0; i<num; i++)
	{
		phItems[i] = m_hServerItems[ itemIndex[i] ];
	}

	hr = m_IOPCSyncIO->Write(num, phItems, values, &pErrors);

	if(hr != S_OK) 
	{	
		ss << "WriteItems failed, hr = " << hr;
		
		if( hr == S_FALSE)
		{
			for(int i=0; i<m_numOfItems; i++)
			{
				if(pErrors[i] != S_OK)
					ss << "sub hr[" << i << "] = " << pErrors[i];
			}
		}			
	}

	if(pErrors != NULL) CoTaskMemFree(pErrors);
	if(phItems != NULL) delete [] phItems;
	if(hr != S_OK) throw SSOPC_Exception(ss.str());
}

int SSOPC_Group::getNumOfItems()
{
	return m_numOfItems;
}

//----------------------------------------------------------

SSOPC_Server::SSOPC_Server()
{
	m_IOPCServer = NULL;
	m_pOPCGroup = NULL;
}

SSOPC_Server::~SSOPC_Server()
{
	if(m_pOPCGroup != NULL) delete m_pOPCGroup;
	if(m_IOPCServer != NULL) m_IOPCServer->Release();
}

SSOPC_Group * SSOPC_Server::InitServer(_bstr_t opcSvrName)
{
	HRESULT hr;
	CLSID clsid;

	//	Given a ProgID, this looks up the associated CLSID in the registry
	hr = CLSIDFromProgID(opcSvrName, &clsid);
	if (hr != S_OK)
	{	
		stringstream ss;
		ss << "Retrival of CLSID failed, hr = " << hr;
		throw SSOPC_Exception(ss.str());
	}

	//	Create the OPC server object and query for the IOPCServer interface of the object
	hr = CoCreateInstance (clsid, NULL, CLSCTX_LOCAL_SERVER ,IID_IOPCServer, (void**)&m_IOPCServer);
	if (hr != S_OK)
	{
		m_IOPCServer = NULL;
		stringstream ss;
		ss << "Creation of IOPCServer-Object failed, hr = " << hr;
		throw SSOPC_Exception(ss.str());
	}

	// Create a OPC Group object
	m_pOPCGroup = new SSOPC_Group(this);
	m_pOPCGroup->InitGroup();

	return m_pOPCGroup;
}

IOPCServer *SSOPC_Server::GetIOPCServer()
{
	return m_IOPCServer;
}

//----------------------------------------------------------