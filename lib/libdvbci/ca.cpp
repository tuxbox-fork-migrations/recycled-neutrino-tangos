#include <stdio.h>

#include <libdvbci/common_access.h>
#include <libdvbapi/debug.h>
#define dvbapi_debug(args...) _dvbapi_debug(DVBAPI_DEBUG_CA, this, args)


static cCA *inst = NULL;

/* those are all dummies for now.. */
cCA::cCA(void)
{
	dvbapi_debug("%s\n", __FUNCTION__);
}

cCA::~cCA()
{
	dvbapi_debug("%s\n", __FUNCTION__);
}

cCA *cCA::GetInstance()
{
	_dvbapi_debug(DVBAPI_DEBUG_CA, NULL, "%s\n", __FUNCTION__);
	if (inst == NULL)
		inst = new cCA();

	return inst;
}

void cCA::MenuEnter(enum CA_SLOT_TYPE, uint32_t p)
{
	dvbapi_debug("%s param:%d\n", __FUNCTION__, (int)p);
}

void cCA::MenuAnswer(enum CA_SLOT_TYPE, uint32_t p, uint32_t /*choice*/)
{
	dvbapi_debug("%s param:%d\n", __FUNCTION__, (int)p);
}

void cCA::InputAnswer(enum CA_SLOT_TYPE, uint32_t p, uint8_t * /*Data*/, int /*Len*/)
{
	dvbapi_debug("%s param:%d\n", __FUNCTION__, (int)p);
}

void cCA::MenuClose(enum CA_SLOT_TYPE, uint32_t p)
{
	dvbapi_debug("%s param:%d\n", __FUNCTION__, (int)p);
}

uint32_t cCA::GetNumberCISlots(void)
{
	dvbapi_debug("%s\n", __FUNCTION__);
	return 0;
}

uint32_t cCA::GetNumberSmartCardSlots(void)
{
	dvbapi_debug("%s\n", __FUNCTION__);
	return 0;
}

void cCA::ModuleName(enum CA_SLOT_TYPE, uint32_t p, char * /*Name*/)
{
	/* TODO: waht to do with *Name? */
	dvbapi_debug("%s param:%d\n", __FUNCTION__, (int)p);
}

bool cCA::ModulePresent(enum CA_SLOT_TYPE, uint32_t p)
{
	dvbapi_debug("%s param:%d\n", __FUNCTION__, (int)p);
	return false;
}

void cCA::ModuleReset(enum CA_SLOT_TYPE, uint32_t p)
{
	dvbapi_debug("%s param:%d\n", __FUNCTION__, (int)p);
}

bool cCA::SendPMT(int, unsigned char *, int, CA_SLOT_TYPE)
{
	dvbapi_debug("%s\n", __FUNCTION__);
	return true;
}

bool cCA::SendMessage(const CA_MESSAGE *)
{
	dvbapi_debug("%s\n", __FUNCTION__);
	return true;
}

bool cCA::Start(void)
{
	dvbapi_debug("%s\n", __FUNCTION__);
	return true;
}

void cCA::Stop(void)
{
	dvbapi_debug("%s\n", __FUNCTION__);
}

void cCA::Ready(bool p)
{
	dvbapi_debug("%s param:%d\n", __FUNCTION__, (int)p);
}

void cCA::SetInitMask(enum CA_INIT_MASK p)
{
	dvbapi_debug("%s param:%d\n", __FUNCTION__, (int)p);
}
