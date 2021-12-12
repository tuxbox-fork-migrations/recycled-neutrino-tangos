#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <linux/route.h>

#include "libnet.h"

#if 0
//never used
static	void	scanip(char *str, unsigned char *to)
{
	int		val;
	int		c;
	char	*sp;
	int		b = 4;

	for (sp = str; b; b--)
	{
		val = 0;
		for (; (*sp != 0) && (*sp != '.'); sp++)
		{
			c = *sp - '0';
			if ((c < 0) || (c >= 10))
				break;
			val = (val * 10) + c;
		}
		*to = val;
		to++;
		if (!*sp)
			break;
		sp++;
	}
}

int	netSetIP(char *dev, char *ip, char *mask, char *brdcast)
{
	int					fd;
	struct ifreq		req;
	int					rc = -1;
	unsigned char		adr_ip[4];
	unsigned char		adr_mask[4];
	unsigned char		adr_brdcast[4];
	struct sockaddr_in	addr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (!fd)
		return -1;

	scanip(ip, adr_ip);
	scanip(mask, adr_mask);
	scanip(brdcast, adr_brdcast);

	/* init structures */
	memset(&req, 0, sizeof(req));
	strcpy(req.ifr_name, dev);

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;

	addr.sin_addr.s_addr = *((unsigned long *) adr_ip);
	memmove(&req.ifr_addr, &addr, sizeof(addr));
	if (ioctl(fd, SIOCSIFADDR, &req) < 0)
		goto abbruch;

	addr.sin_addr.s_addr = *((unsigned long *) adr_mask);
	memmove(&req.ifr_addr, &addr, sizeof(addr));
	if (ioctl(fd, SIOCSIFNETMASK, &req) < 0)
		goto abbruch;

	addr.sin_addr.s_addr = *((unsigned long *) adr_brdcast);
	memmove(&req.ifr_addr, &addr, sizeof(addr));
	if (ioctl(fd, SIOCSIFBRDADDR, &req) < 0)
		goto abbruch;

	rc = 0;
abbruch:
	close(fd);

	return rc;
}
#endif
void	netGetIP(std::string &dev, std::string &ip, std::string &mask, std::string &brdcast)
{
	int					fd;
	struct ifreq		req;
	struct sockaddr_in	*saddr;
	unsigned char		*addr;

	ip = "";
	mask = "";
	brdcast = "";

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (!fd)
		return;


	memset(&req, 0, sizeof(req));
	strncpy(req.ifr_name, dev.c_str(), sizeof(req.ifr_name) - 1);
	saddr = (struct sockaddr_in *) &req.ifr_addr;
	addr = (unsigned char *) &saddr->sin_addr.s_addr;

	char tmp[80];

	if (ioctl(fd, SIOCGIFADDR, &req) == 0)
		snprintf(tmp, sizeof(tmp), "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
	ip = std::string(tmp);

	if (ioctl(fd, SIOCGIFNETMASK, &req) == 0)
		snprintf(tmp, sizeof(tmp), "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
	mask = std::string(tmp);

	if (ioctl(fd, SIOCGIFBRDADDR, &req) == 0)
		snprintf(tmp, sizeof(tmp), "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
	brdcast = std::string(tmp);

	close(fd);
}
#if 0
//never used
void	netSetDefaultRoute(char *gw)
{
	struct rtentry		re;
	struct sockaddr_in	*in_addr;
	unsigned char		*addr;
	int					fd;
	unsigned char		adr_gw[4] = {0};

	scanip(gw, adr_gw);

	memset(&re, 0, sizeof(struct rtentry));

	in_addr = (struct sockaddr_in *)&re.rt_dst;
	in_addr->sin_family = AF_INET;

	in_addr = (struct sockaddr_in *)&re.rt_gateway;
	in_addr->sin_family = AF_INET;
	addr = (unsigned char *)&in_addr->sin_addr.s_addr;

	in_addr = (struct sockaddr_in *)&re.rt_genmask;
	in_addr->sin_family = AF_INET;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		return;

	re.rt_flags = RTF_GATEWAY | RTF_UP;
	memmove(addr, adr_gw, 4);

	ioctl(fd, SIOCADDRT, &re);

	close(fd);
	return;
}
#endif
void netGetDefaultRoute(std::string &ip)
{
	FILE *fp;
	char interface[9];
	uint32_t destination;
	uint32_t gw;
	uint8_t gateway[4];
	char zeile[256];

	ip = "";
	fp = fopen("/proc/net/route", "r");
	if (fp == NULL)
		return;
	char *fg = fgets(zeile, sizeof(zeile), fp); /* skip header */
	while (fg && (fg = fgets(zeile, sizeof(zeile), fp)))
	{
		destination = 1; /* in case sscanf fails */
		sscanf(zeile, "%8s %x %x", interface, &destination, &gw);
		if (destination)
			continue;
		/* big/little endian kernels have reversed entries, so this is correct */
		memcpy(gateway, &gw, 4);
		char tmp[80];
		snprintf(tmp, sizeof(tmp), "%d.%d.%d.%d", gateway[0], gateway[1], gateway[2], gateway[3]);
		ip = std::string(tmp);
		break;
	}
	fclose(fp);
}

#if 0
static	char	dombuf[256];
static	char	domis = 0;
//never used
char	*netGetDomainname(void)
{
	if (!domis)
		getdomainname(dombuf, 256);
	domis = 1;
	return dombuf;
}

void	netSetDomainname(char *dom)
{
	strcpy(dombuf, dom);
	domis = 1;
	setdomainname(dombuf, strlen(dombuf) + 1);
}
#endif
void netGetHostname(std::string &host)
{
	host = "";
	char hostbuf[256];
	if (!gethostname(hostbuf, sizeof(hostbuf)))
		host = std::string(hostbuf);
}

void	netSetHostname(std::string &host)
{
	if (sethostname(host.c_str(), host.length()) < 0)
	{
		fprintf(stderr, "error set %s\n", host.c_str());
	}
	else
	{
		FILE *fp = fopen("/etc/hostname", "w");
		if (fp != NULL)
		{
			fprintf(fp, "%s\n", host.c_str());
			fclose(fp);
		}
	}
}

void	netSetNameserver(std::string &ip)
{
	FILE	*fp;

	fp = fopen("/etc/resolv.conf", "w");
	if (!fp)
		return;

#if 0
	char	*dom;
	dom = netGetDomainname();
	if (dom && strlen(dom) > 2)
		fprintf(fp, "search %s\n", dom);
#endif
	fprintf(fp, "# generated by neutrino\n");
	if (!ip.empty())
		fprintf(fp, "nameserver %s\n", ip.c_str());
	fclose(fp);
}

void	netGetNameserver(std::string &ip)
{
	FILE *fp;
	char zeile[256];
	char *indexLocal;
	unsigned zaehler;

	ip = "";
	fp = fopen("/etc/resolv.conf", "r");
	if (!fp)
		return;

	while (fgets(zeile, sizeof(zeile), fp))
	{
		if (!strncasecmp(zeile, "nameserver", 10))
		{
			char tmp[20];
			indexLocal = zeile + 10;
			while ((*indexLocal == ' ') || (*indexLocal == '\t'))
				indexLocal++;
			zaehler = 0;
			while ((zaehler < 15) && (((*indexLocal >= '0') && (*indexLocal <= '9')) || (*indexLocal == '.')))
				tmp[zaehler++] = *(indexLocal++);
			tmp[zaehler] = 0;
			ip = std::string(tmp);
			break;
		}
	}
	fclose(fp);
}

void netGetMacAddr(std::string &ifname, unsigned char *mac)
{
	int fd;
	struct ifreq ifr;

	memset(mac, 0, 6);
	memset(&ifr, 0, sizeof(ifr));
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		return;

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, ifname.c_str(), sizeof(ifr.ifr_name) - 1);

	if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0)
		return;

	memmove(mac, ifr.ifr_hwaddr.sa_data, 6);
}
