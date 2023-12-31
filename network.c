#include <kernel.h>
#include <netman.h>
#include <ps2ip.h>
#include <util.h>

int ethApplyNetIFConfig(int mode)
{
	int result;
	// By default, auto-negotiation is used.
	static int CurrentMode = NETMAN_NETIF_ETH_LINK_MODE_AUTO;

	if (CurrentMode != mode)
	{ // Change the setting, only if different.
		if ((result = NetManSetLinkMode(mode)) == 0)
			CurrentMode = mode;
	}
	else
		result = 0;

	return result;
}

void EthStatusCheckCb(s32 alarm_id, u16 time, void *common)
{
	iWakeupThread(*(int *)common);
}

int WaitValidNetState(int (*checkingFunction)(void))
{
	int ThreadID, retry_cycles;

	// Wait for a valid network status;
	ThreadID = GetThreadId();
	for (retry_cycles = 0; checkingFunction() == 0; retry_cycles++)
	{ // Sleep for 1000ms.
		SetAlarm(1000 * 16, &EthStatusCheckCb, &ThreadID);
		SleepThread();

		if (retry_cycles >= 10) // 10s = 10*1000ms
			return -1;
	}

	return 0;
}

int ethGetNetIFLinkStatus(void)
{
	return (NetManIoctl(NETMAN_NETIF_IOCTL_GET_LINK_STATUS, NULL, 0, NULL, 0) == NETMAN_NETIF_ETH_LINK_STATE_UP);
}

int ethWaitValidNetIFLinkState(void)
{
	return WaitValidNetState(&ethGetNetIFLinkStatus);
}

int ethGetDHCPStatus(void)
{
	t_ip_info ip_info;
	int result;

	if ((result = ps2ip_getconfig("sm0", &ip_info)) >= 0)
	{ // Check for a successful state if DHCP is enabled.
		if (ip_info.dhcp_enabled)
			result = (ip_info.dhcp_status == DHCP_STATE_BOUND || (ip_info.dhcp_status == DHCP_STATE_OFF));
		else
			result = -1;
	}

	return result;
}

int ethWaitValidDHCPState(void)
{
	return WaitValidNetState(&ethGetDHCPStatus);
}

int ethApplyIPConfig(int use_dhcp, const struct ip4_addr *ip, const struct ip4_addr *netmask, const struct ip4_addr *gateway, const struct ip4_addr *dns)
{
	t_ip_info ip_info;
	int result;

	// SMAP is registered as the "sm0" device to the TCP/IP stack.
	if ((result = ps2ip_getconfig("sm0", &ip_info)) >= 0)
	{
		const ip_addr_t *dns_curr;

		// Obtain the current DNS server settings.
		dns_curr = dns_getserver(0);

		// Check if it's the same. Otherwise, apply the new configuration.
		if ((use_dhcp != ip_info.dhcp_enabled) || (!use_dhcp &&
												   (!ip_addr_cmp(ip, (struct ip4_addr *)&ip_info.ipaddr) ||
													!ip_addr_cmp(netmask, (struct ip4_addr *)&ip_info.netmask) ||
													!ip_addr_cmp(gateway, (struct ip4_addr *)&ip_info.gw) ||
													!ip_addr_cmp(dns, dns_curr))))
		{
			if (use_dhcp)
			{
				ip_info.dhcp_enabled = 1;
			}
			else
			{ // Copy over new settings if DHCP is not used.
				ip_addr_set((struct ip4_addr *)&ip_info.ipaddr, ip);
				ip_addr_set((struct ip4_addr *)&ip_info.netmask, netmask);
				ip_addr_set((struct ip4_addr *)&ip_info.gw, gateway);

				ip_info.dhcp_enabled = 0;
			}

			// Update settings.
			result = ps2ip_setconfig(&ip_info);
			if (!use_dhcp)
				dns_setserver(0, dns);
		}
		else
			result = 0;
	}

	return result;
}

void ethPrintIPConfig(void)
{
	t_ip_info ip_info;
	u8 ip_address[4], netmask[4], gateway[4], dns[4];

	// SMAP is registered as the "sm0" device to the TCP/IP stack.
	if (ps2ip_getconfig("sm0", &ip_info) >= 0)
	{
		const ip_addr_t *dns_curr;

		// Obtain the current DNS server settings.
		dns_curr = dns_getserver(0);

		ip_address[0] = ip4_addr1((struct ip4_addr *)&ip_info.ipaddr);
		ip_address[1] = ip4_addr2((struct ip4_addr *)&ip_info.ipaddr);
		ip_address[2] = ip4_addr3((struct ip4_addr *)&ip_info.ipaddr);
		ip_address[3] = ip4_addr4((struct ip4_addr *)&ip_info.ipaddr);

		netmask[0] = ip4_addr1((struct ip4_addr *)&ip_info.netmask);
		netmask[1] = ip4_addr2((struct ip4_addr *)&ip_info.netmask);
		netmask[2] = ip4_addr3((struct ip4_addr *)&ip_info.netmask);
		netmask[3] = ip4_addr4((struct ip4_addr *)&ip_info.netmask);

		gateway[0] = ip4_addr1((struct ip4_addr *)&ip_info.gw);
		gateway[1] = ip4_addr2((struct ip4_addr *)&ip_info.gw);
		gateway[2] = ip4_addr3((struct ip4_addr *)&ip_info.gw);
		gateway[3] = ip4_addr4((struct ip4_addr *)&ip_info.gw);

		dns[0] = ip4_addr1(dns_curr);
		dns[1] = ip4_addr2(dns_curr);
		dns[2] = ip4_addr3(dns_curr);
		dns[3] = ip4_addr4(dns_curr);

		sio_printf("IP:\t%d.%d.%d.%d\n"
				   "NM:\t%d.%d.%d.%d\n"
				   "GW:\t%d.%d.%d.%d\n"
				   "DNS:\t%d.%d.%d.%d\n",
				   ip_address[0], ip_address[1], ip_address[2], ip_address[3],
				   netmask[0], netmask[1], netmask[2], netmask[3],
				   gateway[0], gateway[1], gateway[2], gateway[3],
				   dns[0], dns[1], dns[2], dns[3]);
	}
	else
	{
		sio_printf("Unable to read IP address.\n");
	}
}

void ethPrintLinkStatus(void)
{
	int mode, baseMode;

	// SMAP is registered as the "sm0" device to the TCP/IP stack.
	sio_printf("Link:\t");
	if (NetManIoctl(NETMAN_NETIF_IOCTL_GET_LINK_STATUS, NULL, 0, NULL, 0) == NETMAN_NETIF_ETH_LINK_STATE_UP)
		sio_printf("Up\n");
	else
		sio_printf("Down\n");

	sio_printf("Mode:\t");
	mode = NetManIoctl(NETMAN_NETIF_IOCTL_ETH_GET_LINK_MODE, NULL, 0, NULL, 0);

	// NETMAN_NETIF_ETH_LINK_MODE_PAUSE is a flag, so file it off first.
	baseMode = mode & (~NETMAN_NETIF_ETH_LINK_DISABLE_PAUSE);
	switch (baseMode)
	{
	case NETMAN_NETIF_ETH_LINK_MODE_10M_HDX:
		sio_printf("10M HDX");
		break;
	case NETMAN_NETIF_ETH_LINK_MODE_10M_FDX:
		sio_printf("10M FDX");
		break;
	case NETMAN_NETIF_ETH_LINK_MODE_100M_HDX:
		sio_printf("100M HDX");
		break;
	case NETMAN_NETIF_ETH_LINK_MODE_100M_FDX:
		sio_printf("100M FDX");
		break;
	default:
		sio_printf("Unknown");
	}
	if (!(mode & NETMAN_NETIF_ETH_LINK_DISABLE_PAUSE))
		sio_printf(" with ");
	else
		sio_printf(" without ");
	sio_printf("Flow Control\n");
}

int initNetworking()
{
	NetManInit();

	// Attempt to apply the new link setting.
	if (ethApplyNetIFConfig(NETMAN_NETIF_ETH_LINK_MODE_AUTO) != 0)
	{
		sio_printf("Error: failed to set link mode.\n");
		return 0;
	}

	struct ip4_addr IP, NM, GW, DNS;
	// Initialize IP address.
	// In this example, DHCP is enabled, hence the IP, NM, GW and DNS fields are cleared to 0..
	ip4_addr_set_zero(&IP);
	ip4_addr_set_zero(&NM);
	ip4_addr_set_zero(&GW);
	ip4_addr_set_zero(&DNS);

	// Initialize the TCP/IP protocol stack.
	ps2ipInit(&IP, &NM, &GW);

	// Enable DHCP
	ethApplyIPConfig(1, &IP, &NM, &GW, &DNS);

	// Wait for the link to become ready.
	sio_printf("Waiting for connection...\n");
	if (ethWaitValidNetIFLinkState() != 0)
	{
		sio_printf("Error: failed to get valid link status.\n");
		return 0;
	}

	sio_printf("Waiting for DHCP lease...");
	// Wait for DHCP to initialize, if DHCP is enabled.
	if (ethWaitValidDHCPState() != 0)
	{
		sio_printf("DHCP failed\n.");
		return 0;
	}
	sio_printf("done!\n");

	sio_printf("Initialized:\n");
	ethPrintLinkStatus();
	ethPrintIPConfig();
	return 1;
}
