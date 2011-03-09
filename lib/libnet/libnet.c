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

static	void	scanip( char *str, unsigned char *to )
{
	int		val;
	int		c;
	char	*sp;
	int		b=4;

	for( sp=str; b; b-- )
	{
		val=0;
		for(; (*sp != 0) && (*sp != '.'); sp++)
		{
			c = *sp - '0';
			if (( c < 0 ) || ( c>=10))
				break;
			val=(val*10)+c;
		}
		*to=val;
		to++;
		if ( !*sp )
			break;
		sp++;
	}
}

int	netSetIP( char *dev, char *ip, char *mask, char *brdcast )
{
	int					fd;
	struct ifreq		req;
	int					rc=-1;
	unsigned char		adr_ip[4];
	unsigned char		adr_mask[4];
	unsigned char		adr_brdcast[4];
	struct sockaddr_in	addr;

	fd=socket(AF_INET,SOCK_DGRAM,0);
	if ( !fd )
		return -1;

	scanip( ip, adr_ip );
	scanip( mask, adr_mask );
	scanip( brdcast, adr_brdcast );

	/* init structures */
	memset(&req,0,sizeof(req));
	strcpy(req.ifr_name,dev);

	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;

	addr.sin_addr.s_addr = *((unsigned long *) adr_ip);
	memmove(&req.ifr_addr,&addr,sizeof(addr));
	if( ioctl(fd,SIOCSIFADDR,&req) < 0 )
		goto abbruch;

	addr.sin_addr.s_addr = *((unsigned long *) adr_mask);
	memmove(&req.ifr_addr,&addr,sizeof(addr));
	if( ioctl(fd,SIOCSIFNETMASK,&req) < 0 )
		goto abbruch;

	addr.sin_addr.s_addr = *((unsigned long *) adr_brdcast);
	memmove(&req.ifr_addr,&addr,sizeof(addr));
	if( ioctl(fd,SIOCSIFBRDADDR,&req) < 0 )
		goto abbruch;

	rc = 0;
abbruch:
	close(fd);

	return rc;
}

void	netGetIP( char *dev, char *ip, char *mask, char *brdcast )
{
	int					fd;
	struct ifreq		req;
	struct sockaddr_in	*saddr;
	unsigned char		*addr;

	*ip=0;
	*mask=0;
	*brdcast=0;

	fd=socket(AF_INET,SOCK_DGRAM,0);
	if ( !fd )
		return;


	memset(&req,0,sizeof(req));
	strcpy(req.ifr_name,dev);
	saddr = (struct sockaddr_in *) &req.ifr_addr;
	addr= (unsigned char*) &saddr->sin_addr.s_addr;

	if( ioctl(fd,SIOCGIFADDR,&req) == 0 )
		sprintf(ip,"%d.%d.%d.%d",addr[0],addr[1],addr[2],addr[3]);

	if( ioctl(fd,SIOCGIFNETMASK,&req) == 0 )
		sprintf(mask,"%d.%d.%d.%d",addr[0],addr[1],addr[2],addr[3]);

	if( ioctl(fd,SIOCGIFBRDADDR,&req) == 0 )
		sprintf(brdcast,"%d.%d.%d.%d",addr[0],addr[1],addr[2],addr[3]);

	close(fd);
	return;
}

void	netSetDefaultRoute( char *gw )
{
	struct rtentry		re;
	struct sockaddr_in	*in_addr;
	unsigned char		*addr;
	int					fd;
	unsigned char		adr_gw[4];

	scanip( gw, adr_gw );

	memset(&re,0,sizeof(struct rtentry));

	in_addr = (struct sockaddr_in *)&re.rt_dst;
	in_addr->sin_family = AF_INET;

	in_addr = (struct sockaddr_in *)&re.rt_gateway;
	in_addr->sin_family = AF_INET;
	addr=(unsigned char*)&in_addr->sin_addr.s_addr;

	in_addr = (struct sockaddr_in *)&re.rt_genmask;
	in_addr->sin_family = AF_INET;

	fd=socket(AF_INET,SOCK_DGRAM,0);
	if ( fd < 0 )
		return;

	re.rt_flags = RTF_GATEWAY | RTF_UP;
	memmove(addr,adr_gw,4);

	ioctl(fd,SIOCADDRT,&re);

	close(fd);
	return;
}

void netGetDefaultRoute( char *ip )
{
	FILE *fp;
	char interface[9];
	unsigned char destination[4];
	unsigned char gateway[4];
	char zeile[256];

	*ip = 0 ;
	fp = fopen("/proc/net/route","r");
	if (fp == NULL)
		return;
	fgets(zeile,sizeof(zeile),fp);
	while(fgets(zeile,sizeof(zeile),fp))
	{
		sscanf(zeile,"%8s %x %x",interface,(unsigned *) destination,(unsigned *) gateway);
		if (*(unsigned *)destination == 0)
		{
			sprintf(ip,"%d.%d.%d.%d",gateway[0],gateway[1],gateway[2],gateway[3]);
			break;
		}
	}
	fclose(fp);
}

static	char	dombuf[256];
static	char	hostbuf[256];
static	char	domis=0;
static	char	hostis=0;

char	*netGetDomainname( void )
{
	if (!domis)
		getdomainname( dombuf, 256 );
	domis=1;
	return dombuf;
}

void	netSetDomainname( char *dom )
{
	strcpy(dombuf,dom);
	domis=1;
	setdomainname(dombuf,strlen(dombuf)+1);
}

char	*netGetHostname( void )
{
	if (!hostis)
		gethostname( hostbuf, 256 );
	hostis=1;
	return hostbuf;
}

void	netSetHostname( char *host )
{
	FILE * fp;

	strcpy(hostbuf,host);
	hostis=1;
	sethostname(hostbuf,strlen(hostbuf)+1);
	fp = fopen("/etc/hostname", "w");
	if(fp != NULL) {
		fprintf(fp, "%s\n", hostbuf);
		fclose(fp);
	}
}

void	netSetNameserver(const char * const ip)
{
	FILE	*fp;

	fp = fopen("/etc/resolv.conf","w");
	if (!fp)
		return;

#if 0
	char	*dom;
	dom=netGetDomainname();
	if (dom && strlen(dom)>2)
		fprintf(fp,"search %s\n",dom);
#endif
	fprintf(fp, "# generated by neutrino\n");
	if ((ip != NULL) && (strlen(ip) > 0))
		fprintf(fp,"nameserver %s\n",ip);
	fclose(fp);
}

void	netGetNameserver( char *ip )
{
	FILE *fp;
	char zeile[256];
	char *indexLocal;
	unsigned zaehler;

	*ip = 0;
	fp = fopen("/etc/resolv.conf","r");
	if (!fp)
		return;

	while (fgets(zeile,sizeof(zeile),fp))
	{
		if (!strncasecmp(zeile,"nameserver",10))
		{
			indexLocal = zeile + 10;
			while ( (*indexLocal == ' ') || (*indexLocal == '\t') )
				indexLocal++;
			zaehler = 0;
			while ( (zaehler < 15) && ( ((*indexLocal >= '0') && (*indexLocal <= '9')) || (*indexLocal == '.')))
				ip[zaehler++] = *(indexLocal++);
			ip[zaehler] = 0;
			break;
		}
	}
	fclose(fp);
}
