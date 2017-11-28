#include "network/nethostproxy.h"

#include <memory>

#include "base/log.h"
#include "network/relay_protocol.h"

std::unique_ptr<NetHostProxy> NetHostProxy::connect(const std::pair<NetAddress, NetAddress>& addresses, const std::string& name, const std::string& password) {
	std::unique_ptr<NetHostProxy> ptr(new NetHostProxy(addresses, name, password));
	if (ptr->conn_ == nullptr || !ptr->conn_->is_connected()) {
		ptr.reset();
	}
	return ptr;
}

NetHostProxy::~NetHostProxy() {
	if (conn_ && conn_->is_connected()) {
		while (!clients_.empty()) {
			close(clients_.begin()->first);
			clients_.erase(clients_.begin());
		}
		conn_->close();
	}
}

bool NetHostProxy::is_connected(const ConnectionId id) const {
	return clients_.count(id) > 0 && clients_.at(id).state_ == Client::State::kConnected;
}

void NetHostProxy::close(const ConnectionId id) {
	auto iter_client = clients_.find(id);
	if (iter_client == clients_.end()) {
		// Not connected anyway
		return;
	}
    conn_->send(RelayCommand::kDisconnectClient);
    conn_->send(id);
    if (iter_client->second.received_.empty()) {
		// No pending messages, remove the client
		clients_.erase(iter_client);
    } else {
    	// Still messages pending. Keep the structure so the host can receive them
		iter_client->second.state_ = Client::State::kDisconnected;
    }
}

bool NetHostProxy::try_accept(ConnectionId* new_id) {
	// Always read all available data into buffers
	receive_commands();

	for (auto& entry : clients_) {
		if (entry.second.state_ == Client::State::kConnecting) {
			*new_id = entry.first;
			entry.second.state_ = Client::State::kConnected;
			return true;
		}
	}
	return false;
}

bool NetHostProxy::try_receive(const ConnectionId id, RecvPacket* packet) {
	receive_commands();

	// Check whether client is not (yet) connected
	if (clients_.count(id) == 0 || clients_.at(id).state_ == Client::State::kConnecting)
		return false;

	std::queue<RecvPacket>& packet_list = clients_.at(id).received_;

	// Now check whether there is data for the requested client
	if (packet_list.empty()) {
		// If the client is already disconnected it should not be in the map anymore
		assert(clients_.at(id).state_ == Client::State::kConnected);
		return false;
	}

	*packet = std::move(packet_list.front());
	packet_list.pop();
	if (packet_list.empty() && clients_.at(id).state_ == Client::State::kDisconnected) {
		// If the receive buffer is empty now, remove client
		clients_.erase(id);
	}
	return true;
}

void NetHostProxy::send(const ConnectionId id, const SendPacket& packet) {
	std::vector<ConnectionId> vec;
	vec.push_back(id);
	send(vec, packet);
}

void NetHostProxy::send(const std::vector<ConnectionId>& ids, const SendPacket& packet) {
	if (ids.empty()) {
		return;
	}

	receive_commands();

	conn_->send(RelayCommand::kToClients);
	for (ConnectionId id : ids) {
		if (is_connected(id)) {
			// This should be but is not always the case. It can happen that we receive a client disconnect
			// on receive_commands() above and the GameHost did not have the chance to react to it yet.
			conn_->send(id);
		}
	}
	conn_->send(0);
	conn_->send(packet);
}

NetHostProxy::NetHostProxy(const std::pair<NetAddress, NetAddress>& addresses, const std::string& name, const std::string& password)
	: conn_(NetRelayConnection::connect(addresses.first)) {

	if ((conn_ == nullptr || !conn_->is_connected()) && addresses.second.is_valid()) {
		conn_ = NetRelayConnection::connect(addresses.second);
	}

   	if (conn_ == nullptr || !conn_->is_connected()) {
		return;
   	}

   	conn_->send(RelayCommand::kHello);
   	conn_->send(kRelayProtocolVersion);
   	conn_->send(name);
   	conn_->send(password);
   	conn_->send(password);

   	// Wait 10 seconds for an answer
	uint32_t endtime = time(nullptr) + 10;
	while (!NetRelayConnection::Peeker(conn_).cmd()) {
		if (time(nullptr) > endtime) {
			// No message received in time
			conn_->close();
			conn_.reset();
			return;
		}
	}

	RelayCommand cmd;
	conn_->receive(&cmd);

   	if (cmd != RelayCommand::kWelcome) {
		conn_->close();
		conn_.reset();
		return;
   	}

   	// Check version
	endtime = time(nullptr) + 10;
	while (!NetRelayConnection::Peeker(conn_).uint8_t()) {
		if (time(nullptr) > endtime) {
			// No message received in time
			conn_->close();
			conn_.reset();
			return;
		}
	}
	uint8_t relay_proto_version;
	conn_->receive(&relay_proto_version);
   	if (relay_proto_version != kRelayProtocolVersion) {
		conn_->close();
		conn_.reset();
		return;
   	}

   	// Check game name
	endtime = time(nullptr) + 10;
	while (!NetRelayConnection::Peeker(conn_).string()) {
		if (time(nullptr) > endtime) {
			// No message received in time
			conn_->close();
			conn_.reset();
			return;
		}
	}
	std::string game_name;
	conn_->receive(&game_name);
   	if (game_name != name) {
		conn_->close();
		conn_.reset();
		return;
   	}
}

void NetHostProxy::receive_commands() {
	if (!conn_->is_connected()) {
		return;
	}

	// Receive all available commands
	RelayCommand cmd;
	NetRelayConnection::Peeker peek(conn_);
	if (!peek.cmd(&cmd)) {
		// No command to receive
		return;
	}
	switch (cmd) {
		case RelayCommand::kDisconnect:
			if (peek.string()) {
				// Command is completely in the buffer, handle it
				conn_->receive(&cmd);
				std::string reason;
				conn_->receive(&reason);
				conn_->close();
				// Set all clients to offline
				for (auto& entry : clients_) {
					entry.second.state_ = Client::State::kDisconnected;
				}
			}
			break;
		case RelayCommand::kConnectClient:
			if (peek.uint8_t()) {
				conn_->receive(&cmd);
				uint8_t id;
				conn_->receive(&id);
#ifndef NDEBUG
			   auto result = clients_.emplace(
			      std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple());
				assert(result.second);
#else
			   clients_.emplace(
			      std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple());
#endif
			}
			break;
		case RelayCommand::kDisconnectClient:
			if (peek.uint8_t()) {
				conn_->receive(&cmd);
				uint8_t id;
				conn_->receive(&id);
				assert(clients_.count(id));
				clients_.at(id).state_ = Client::State::kDisconnected;
			}
			break;
		case RelayCommand::kFromClient:
			if (peek.uint8_t() && peek.recvpacket()) {
				conn_->receive(&cmd);
				uint8_t id;
				conn_->receive(&id);
				RecvPacket packet;
				conn_->receive(&packet);
				assert(clients_.count(id));
				clients_.at(id).received_.push(std::move(packet));
			}
			break;
		case RelayCommand::kPing:
			conn_->receive(&cmd);
			// Reply with a pong
			conn_->send(RelayCommand::kPong);
			break;
		default:
			// Other commands should not be possible.
			// Then is either something wrong with the protocol or there is an implementation mistake
			log("Received command code %i from relay server, do not know what to do with it\n",
					static_cast<uint8_t>(cmd));
			NEVER_HERE();
	}
}
