#include <4dm.h>
#include "Config.h"

using namespace fdm;

$hook(void, WorldClient, updateLocal, StateManager& s, Player* player, double dt)
{
	if (!Config::netOptimization()) return original(self, s, player, dt);

	using namespace Connection;

	if (self->client->status == Client::CONNECTED)
	{
		constexpr int COUNT = 64;
		SteamNetworkingMessage_t* incomingMsgs[COUNT];
		int numMsgs = self->client->Interface->ReceiveMessagesOnConnection(self->client->connectionHandle, incomingMsgs, COUNT);
		while (numMsgs > 0)
		{
			for (int i = 0; i < numMsgs; i++)
			{
				self->handleMessage({ incomingMsgs[i] }, player);
			}
			numMsgs = self->client->Interface->ReceiveMessagesOnConnection(self->client->connectionHandle, incomingMsgs, COUNT);
		}
	}

	original(self, s, player, dt);
}

$hook(void, WorldServer, updateBackend, double dt)
{
	if (!Config::netOptimization()) return original(self, dt);

	using namespace Connection;

	if (self->restartCountdownStarted
		|| glfwGetTime() - self->restartTime <= self->restartInterval - self->restartCountdownTime)
	{
		if (self->server.status == Server::ONLINE)
		{
			constexpr int COUNT = 64;
			SteamNetworkingMessage_t* incomingMsgs[COUNT];
			int numMsgs = self->server.Interface->ReceiveMessagesOnPollGroup(self->server.pollGroupHandle, incomingMsgs, COUNT);
			while (numMsgs > 0)
			{
				for (int i = 0; i < numMsgs; i++)
				{
					self->handleMessage({ incomingMsgs[i] }, dt);
				}
				numMsgs = self->server.Interface->ReceiveMessagesOnPollGroup(self->server.pollGroupHandle, incomingMsgs, COUNT);
			}
		}
	}

	original(self, dt);
}

$hook(SteamNetworkingMessage_t*, Connection::OutMessage, createMessage, uint32_t recipient, int sendFlags)
{
	if (!Config::netOptimization()) return original(self, recipient, sendFlags);
	
	return original(self, recipient, sendFlags != k_nSteamNetworkingSend_Reliable ? k_nSteamNetworkingSend_UnreliableNoNagle : k_nSteamNetworkingSend_ReliableNoNagle);
}
