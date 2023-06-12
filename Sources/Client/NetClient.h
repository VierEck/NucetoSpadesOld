/*
 Copyright (c) 2013 yvt
 based on code of pysnip (c) Mathias Kaerlev 2011-2012.

 This file is part of OpenSpades.

 OpenSpades is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 OpenSpades is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with OpenSpades.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <set>
#include <cstdint>

#include "PhysicsConstants.h"
#include "Player.h"
#include <Core/Debug.h>
#include <Core/Math.h>
#include <Core/ServerAddress.h>
#include <Core/Stopwatch.h>
#include <Core/VersionInfo.h>
#include <OpenSpades.h>

#include <enet/enet.h>

struct _ENetHost;
struct _ENetPeer;
typedef _ENetHost ENetHost;
typedef _ENetPeer ENetPeer;

namespace spades {
	namespace client {
		class Client;
		class Player;
		enum NetClientStatus {
			NetClientStatusNotConnected = 0,
			NetClientStatusConnecting,
			NetClientStatusReceivingMap,
			NetClientStatusConnected
		};

		class World;
		class NetPacketReader;
		struct PlayerInput;
		struct WeaponInput;
		class Grenade;
		struct GameProperties;
		class NetClient {
			Client *client;
			NetClientStatus status;
			ENetHost *host;
			ENetPeer *peer;
			std::string statusString;
			unsigned int mapSize;
			std::vector<char> mapData;
			std::shared_ptr<GameProperties> properties;

			int protocolVersion;

			class BandwidthMonitor {
				ENetHost *host;
				Stopwatch sw;
				double lastDown;
				double lastUp;

			public:
				BandwidthMonitor(ENetHost *);
				double GetDownlinkBps() { return lastDown * 8.; }
				double GetUplinkBps() { return lastUp * 8.; }
				void Update();
			};

			std::unique_ptr<BandwidthMonitor> bandwidthMonitor;

			struct {
				std::unique_ptr<IStream> stream;
				std::vector<char> data;

				float start_time;
				float delta_time;
			} demo;

			std::vector<Vector3> savedPlayerPos;
			std::vector<Vector3> savedPlayerFront;
			std::vector<int> savedPlayerTeam;

			std::vector<std::vector<char>> savedPackets;

			int timeToTryMapLoad;
			bool tryMapLoadOnPacketType;

			unsigned int lastPlayerInput;
			unsigned int lastWeaponInput;

			// used for some scripts including Arena by Yourself
			IntVector3 temporaryPlayerBlockColor;

			bool HandleHandshakePacket(NetPacketReader &);
			void HandleGamePacket(NetPacketReader &);
			World *GetWorld();
			Player *GetPlayer(int);
			Player *GetPlayerOrNull(int);
			Player *GetLocalPlayer();
			Player *GetLocalPlayerOrNull();

			std::string DisconnectReasonString(uint32_t);

			void MapLoaded();

			void SendVersion();
			void SendVersionEnhanced(const std::set<std::uint8_t> &propertyIds);

		public:
			NetClient(Client *, bool replay);
			~NetClient();

			NetClientStatus GetStatus() { return status; }

			std::string GetStatusString() { return statusString; }

			/**
			 * Return a non-null reference to `GameProperties` for this connection.
			 * Must be the connected state.
			 */
			std::shared_ptr<GameProperties> &GetGameProperties() {
				SPAssert(properties);
				return properties;
			}

			void Connect(const ServerAddress &hostname);
			void Disconnect();

			int GetPing();

			void DoEvents(int timeout = 0);
			void DoPackets(NetPacketReader &);

			void SendJoin(int team, WeaponType, std::string name, int kills);
			void SendPosition();
			void SendOrientation(Vector3);
			void SendPlayerInput(PlayerInput);
			void SendWeaponInput(WeaponInput);
			void SendBlockAction(IntVector3, BlockActionType);
			void SendBlockLine(IntVector3 v1, IntVector3 v2);
			void SendReload();
			void SendTool();
			void SendGrenade(Grenade *);
			void SendHeldBlockColor();
			void SendHit(int targetPlayerId, HitType type);
			void SendChat(std::string, bool global);
			void SendWeaponChange(WeaponType);
			void SendTeamChange(int team);
			void SendHandShakeValid(int challenge);

			double GetDownlinkBps() { return bandwidthMonitor->GetDownlinkBps(); }
			double GetUplinkBps() { return bandwidthMonitor->GetUplinkBps(); }

			IStream* HandleDemoStream(std::string, bool replay);
			void RegisterDemoPacket(ENetPacket *packet);
			void DemoStart(std::string, bool replay);
			void DemoStop();
			bool DemoStarted = false;

			void joinReplay();
			void ReadNextDemoPacket();
			void DoDemo();
			bool DemoSkippingMap;
			int GetDemoTimer();
			std::string demo_end_time;
			int demo_skip_time;
			void DemoSkipMap();
			bool DemoFirstJoined;
			void DemoSetFollow();

			void DemoCommands(std::string command);
			void DemoCommandPause();
			bool DemoPaused;
			bool PauseDemoAfterSkip;
			void DemoCommandUnpause(bool skipped);

			void DemoCommandFF(int seconds);
			void DemoCommandBB(int seconds);
			void DemoCommandGT(std::string delta);
			float demo_skip_end_time;

			void DemoCommandSP(float speed);
			int DemoStringToInt(std::string integer);
			std::pair<int, bool> DemoFollowState;
			void DemoCommandNextUps(int ups);
			int demo_next_ups;
			int demo_count_ups;
			bool PrevUps;
			void DemoCommandPrevUps(int ups);
			void DemoCountUps();
		};
	}
}
