/*
 * Copyright (C) 2004, 2006-2011, 2013 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_LOGIC_PLAYERCOMMAND_H
#define WL_LOGIC_PLAYERCOMMAND_H

#include <memory>

#include "logic/cmd_queue.h"
#include "economy/flag.h"
#include "logic/message_id.h"
#include "logic/path.h"
#include "logic/trainingsite.h"
#include "logic/warehouse.h"
#include "logic/worker.h"

namespace Widelands {

/**
 * PlayerCommand is for commands issued by players. It has the additional
 * ability to send itself over the network
 *
 * PlayerCommands are given serial numbers once they become authoritative
 * (e.g. after being acked by the server). The serial numbers must then be
 * reasonably unique (to be precise, they must be unique per duetime) and
 * the same across all hosts, to ensure parallel simulation.
 */
class PlayerCommand : public GameLogicCommand {
public:
	PlayerCommand (int32_t time, Player_Number);

	/// For savegame loading
	PlayerCommand() : GameLogicCommand(0), m_sender(0), m_cmdserial(0) {}

	Player_Number sender   () const {return m_sender;}
	uint32_t      cmdserial() const {return m_cmdserial;}
	void set_cmdserial(const uint32_t s) {m_cmdserial = s;}

	virtual void serialize (StreamWrite &) = 0;
	static Widelands::PlayerCommand * deserialize (StreamRead &);

	// Call these from child classes
	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

private:
	Player_Number m_sender;
	uint32_t      m_cmdserial;
};

struct CmdBulldoze:public PlayerCommand {
	CmdBulldoze() : PlayerCommand(), serial(0), recurse(0) {} // For savegame loading
	CmdBulldoze
		(const int32_t t, const int32_t p,
		 PlayerImmovable & pi,
		 const bool _recurse = false)
		: PlayerCommand(t, p), serial(pi.serial()), recurse(_recurse)
	{}

	CmdBulldoze (StreamRead &);

	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_BULLDOZE;}

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
	bool   recurse;
};

struct CmdBuild:public PlayerCommand {
	CmdBuild() : PlayerCommand() {} // For savegame loading
	CmdBuild
		(const int32_t        _duetime,
		 const int32_t        p,
		 const Coords         c,
		 const Building_Index i)
		: PlayerCommand(_duetime, p), coords(c), bi(i)
	{}

	CmdBuild (StreamRead &);

	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_BUILD;}

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Coords         coords;
	Building_Index bi;
};

struct CmdBuildFlag:public PlayerCommand {
	CmdBuildFlag() : PlayerCommand() {} // For savegame loading
	CmdBuildFlag (const int32_t t, const int32_t p, const Coords c) :
		PlayerCommand(t, p), coords(c)
	{}

	CmdBuildFlag (StreamRead &);

	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_FLAG;}

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Coords coords;
};

struct CmdBuildRoad:public PlayerCommand {
	CmdBuildRoad() :
		PlayerCommand(), path(nullptr), start(), nsteps(0), steps(nullptr) {} // For savegame loading
	CmdBuildRoad (int32_t, int32_t, Path &);
	CmdBuildRoad (StreamRead &);

	virtual ~CmdBuildRoad ();

	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_BUILDROAD;}

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Path                       * path;
	Coords                       start;
	Path::Step_Vector::size_type nsteps;
	char                       * steps;
};

struct CmdFlagAction : public PlayerCommand {
	CmdFlagAction() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdFlagAction (const int32_t t, const int32_t p, const Flag & f) :
		PlayerCommand(t, p), serial(f.serial())
	{}

	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_FLAGACTION;}


	CmdFlagAction (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct CmdStartStopBuilding : public PlayerCommand {
	CmdStartStopBuilding() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdStartStopBuilding (const int32_t t, const Player_Number p, Building & b)
		: PlayerCommand(t, p), serial(b.serial())
	{}

	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_STOPBUILDING;}

	CmdStartStopBuilding (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct CmdMilitarySiteSetSoldierPreference : public PlayerCommand {
	CmdMilitarySiteSetSoldierPreference() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdMilitarySiteSetSoldierPreference (const int32_t t, const Player_Number p, Building & b, uint8_t prefs)
		: PlayerCommand(t, p), serial(b.serial()), preference(prefs)
	{}

	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_MILITARYSITESETSOLDIERPREFERENCE;}

	CmdMilitarySiteSetSoldierPreference (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
	uint8_t preference;
};
struct CmdStartOrCancelExpedition : public PlayerCommand {
	CmdStartOrCancelExpedition() : PlayerCommand() {} // For savegame loading
	CmdStartOrCancelExpedition (int32_t const t, Player_Number const p, Building & b)
		: PlayerCommand(t, p), serial(b.serial())
	{}

	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_PORT_START_EXPEDITION;}

	CmdStartOrCancelExpedition (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct CmdEnhanceBuilding:public PlayerCommand {
	CmdEnhanceBuilding() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdEnhanceBuilding
		(const int32_t        _duetime,
		 const int32_t        p,
		 Building           & b,
		 const Building_Index i)
		: PlayerCommand(_duetime, p), serial(b.serial()), bi(i)
	{}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_ENHANCEBUILDING;}

	CmdEnhanceBuilding (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
	Building_Index bi;
};

struct CmdDismantleBuilding:public PlayerCommand {
	CmdDismantleBuilding() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdDismantleBuilding
		(const int32_t t, const int32_t p,
		 PlayerImmovable & pi)
		: PlayerCommand(t, p), serial(pi.serial())
	{}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_DISMANTLEBUILDING;}

	CmdDismantleBuilding (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct CmdEvictWorker : public PlayerCommand {
	CmdEvictWorker() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdEvictWorker
		(const int32_t t, const int32_t p,
		 Worker & w)
		: PlayerCommand(t, p), serial(w.serial())
	{}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_EVICTWORKER;}

	CmdEvictWorker (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct CmdShipScoutDirection : public PlayerCommand {
	CmdShipScoutDirection() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdShipScoutDirection
		(int32_t const t, Player_Number const p, Serial s, uint8_t direction)
		: PlayerCommand(t, p), serial(s), dir(direction)
	{}

	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_SHIP_SCOUT;}

	CmdShipScoutDirection (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
	uint8_t dir;
};

struct CmdShipConstructPort : public PlayerCommand {
	CmdShipConstructPort() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdShipConstructPort
		(int32_t const t, Player_Number const p, Serial s, Coords c)
		: PlayerCommand(t, p), serial(s), coords(c)
	{}

	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_SHIP_CONSTRUCT_PORT;}

	CmdShipConstructPort (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
	Coords coords;
};

struct CmdShipExploreIsland : public PlayerCommand {
	CmdShipExploreIsland() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdShipExploreIsland
		(int32_t const t, Player_Number const p, Serial s, bool cw)
		: PlayerCommand(t, p), serial(s), clockwise(cw)
	{}

	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_SHIP_EXPLORE;}

	CmdShipExploreIsland (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
	bool clockwise;
};

struct CmdShipSink : public PlayerCommand {
	CmdShipSink() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdShipSink
		(int32_t const t, Player_Number const p, Serial s)
		: PlayerCommand(t, p), serial(s)
	{}

	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_SHIP_SINK;}

	CmdShipSink(StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct CmdShipCancelExpedition : public PlayerCommand {
	CmdShipCancelExpedition() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdShipCancelExpedition
		(int32_t const t, Player_Number const p, Serial s)
		: PlayerCommand(t, p), serial(s)
	{}

	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_SHIP_CANCELEXPEDITION;}

	CmdShipCancelExpedition(StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct CmdSetWarePriority : public PlayerCommand {
	// For savegame loading
	CmdSetWarePriority() :
		PlayerCommand(),
		m_serial(0),
		m_type(0),
		m_index(),
		m_priority(0)
	{}
	CmdSetWarePriority
		(int32_t duetime, Player_Number sender,
		 PlayerImmovable &,
		 int32_t type, Ware_Index index, int32_t priority);

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_SETWAREPRIORITY;}

	CmdSetWarePriority(StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial m_serial;
	int32_t m_type; ///< this is always WARE right now
	Ware_Index m_index;
	int32_t m_priority;
};

struct CmdSetWareMaxFill : public PlayerCommand {
	CmdSetWareMaxFill() : PlayerCommand(), m_serial(0), m_index(), m_max_fill(0) {} // For savegame loading
	CmdSetWareMaxFill
		(int32_t duetime, Player_Number,
		 PlayerImmovable &,
		 Ware_Index, uint32_t maxfill);

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_SETWAREMAXFILL;}

	CmdSetWareMaxFill(StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial m_serial;
	Ware_Index m_index;
	uint32_t m_max_fill;
};

struct CmdChangeTargetQuantity : public PlayerCommand {
	CmdChangeTargetQuantity() : PlayerCommand(), m_economy(0), m_ware_type() {} //  For savegame loading.
	CmdChangeTargetQuantity
		(int32_t duetime, Player_Number sender,
		 uint32_t economy, Ware_Index index);

	//  Write/Read these commands to/from a file (for savegames).
	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	CmdChangeTargetQuantity(StreamRead &);

	void serialize (StreamWrite &) override;

protected:
	uint32_t   economy  () const {return m_economy;}
	Ware_Index ware_type() const {return m_ware_type;}

private:
	uint32_t   m_economy;
	Ware_Index m_ware_type;
};


struct CmdSetWareTargetQuantity : public CmdChangeTargetQuantity {
	CmdSetWareTargetQuantity() : CmdChangeTargetQuantity(), m_permanent(0) {}
	CmdSetWareTargetQuantity
		(int32_t duetime, Player_Number sender,
		 uint32_t economy, Ware_Index index,
		 uint32_t permanent);

	//  Write/Read these commands to/from a file (for savegames).
	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_SETWARETARGETQUANTITY;}

	CmdSetWareTargetQuantity(StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	uint32_t m_permanent;
};

struct CmdResetWareTargetQuantity : public CmdChangeTargetQuantity {
	CmdResetWareTargetQuantity() : CmdChangeTargetQuantity() {}
	CmdResetWareTargetQuantity
		(int32_t duetime, Player_Number sender,
		 uint32_t economy, Ware_Index index);

	//  Write/Read these commands to/from a file (for savegames).
	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_RESETWARETARGETQUANTITY;}

	CmdResetWareTargetQuantity(StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;
};

struct CmdSetWorkerTargetQuantity : public CmdChangeTargetQuantity {
	CmdSetWorkerTargetQuantity() : CmdChangeTargetQuantity(), m_permanent(0) {}
	CmdSetWorkerTargetQuantity
		(int32_t duetime, Player_Number sender,
		 uint32_t economy, Ware_Index index,
		 uint32_t permanent);

	//  Write/Read these commands to/from a file (for savegames).
	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_SETWORKERTARGETQUANTITY;}

	CmdSetWorkerTargetQuantity(StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	uint32_t m_permanent;
};

struct CmdResetWorkerTargetQuantity : public CmdChangeTargetQuantity {
	CmdResetWorkerTargetQuantity() : CmdChangeTargetQuantity() {}
	CmdResetWorkerTargetQuantity
		(int32_t duetime, Player_Number sender,
		 uint32_t economy, Ware_Index index);

	//  Write/Read these commands to/from a file (for savegames).
	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_RESETWORKERTARGETQUANTITY;}

	CmdResetWorkerTargetQuantity(StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;
};

struct CmdChangeTrainingOptions : public PlayerCommand {
	CmdChangeTrainingOptions() : PlayerCommand(), serial(0), attribute(0), value(0) {} // For savegame loading
	CmdChangeTrainingOptions
		(const int32_t    t,
		 const Player_Number p,
		 TrainingSite &   ts,
		 const int32_t    at,
		 const int32_t    val)
		: PlayerCommand(t, p), serial(ts.serial()), attribute(at), value(val)
	{}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_CHANGETRAININGOPTIONS;}

	CmdChangeTrainingOptions (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
	int32_t attribute;
	int32_t value;
};

struct CmdDropSoldier : public PlayerCommand {
	CmdDropSoldier () : PlayerCommand(), serial(0), soldier(0) {} //  for savegames
	CmdDropSoldier
		(const int32_t    t,
		 const int32_t    p,
		 Building &       b,
		 const int32_t    _soldier)
		: PlayerCommand(t, p), serial(b.serial()), soldier(_soldier)
	{}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_DROPSOLDIER;}

	CmdDropSoldier(StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
	Serial soldier;
};

struct CmdChangeSoldierCapacity : public PlayerCommand {
	CmdChangeSoldierCapacity () : PlayerCommand(), serial(0), val(0) {} //  for savegames
	CmdChangeSoldierCapacity
		(const int32_t t, const int32_t p, Building & b, const int32_t i)
		: PlayerCommand(t, p), serial(b.serial()), val(i)
	{}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_CHANGESOLDIERCAPACITY;}

	CmdChangeSoldierCapacity (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
	int32_t val;
};

/////////////TESTING STUFF
struct CmdEnemyFlagAction : public PlayerCommand {
	CmdEnemyFlagAction() : PlayerCommand(), serial(0), number(0) {} // For savegame loading
	CmdEnemyFlagAction(int32_t t, int32_t p, const Flag& f, uint32_t num)
	   : PlayerCommand(t, p), serial(f.serial()), number(num) {
	}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_ENEMYFLAGACTION;}

	CmdEnemyFlagAction (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial        serial;
	uint8_t       number;
};

/// Abstract base for commands about a message.
struct PlayerMessageCommand : public PlayerCommand {
	PlayerMessageCommand () : PlayerCommand() {} //  for savegames
	PlayerMessageCommand
		(const uint32_t t, const Player_Number p, const Message_Id i)
		: PlayerCommand(t, p), m_message_id(i)
	{}

	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

	PlayerMessageCommand(StreamRead &);

	Message_Id message_id() const {return m_message_id;}

private:
	Message_Id m_message_id;
};

struct CmdMessageSetStatusRead : public PlayerMessageCommand {
	CmdMessageSetStatusRead () : PlayerMessageCommand() {}
	CmdMessageSetStatusRead
		(const uint32_t t, const Player_Number p, const Message_Id i)
		: PlayerMessageCommand(t, p, i)
	{}

	uint8_t id() const override {return QUEUE_CMD_MESSAGESETSTATUSREAD;}

	CmdMessageSetStatusRead(StreamRead & des) : PlayerMessageCommand(des) {}

	void execute (Game &) override;
	void serialize (StreamWrite &) override;
};

struct CmdMessageSetStatusArchived : public PlayerMessageCommand {
	CmdMessageSetStatusArchived () : PlayerMessageCommand() {}
	CmdMessageSetStatusArchived
		(const uint32_t t, const Player_Number p, const Message_Id i)
		: PlayerMessageCommand(t, p, i)
	{}

	uint8_t id() const override {return QUEUE_CMD_MESSAGESETSTATUSARCHIVED;}

	CmdMessageSetStatusArchived(StreamRead & des) : PlayerMessageCommand(des) {
	}

	void execute (Game &) override;
	void serialize (StreamWrite &) override;
};

/**
 * Command to change the stock policy for a ware or worker in a warehouse.
 */
struct CmdSetStockPolicy : PlayerCommand {
	CmdSetStockPolicy
		(int32_t time, Player_Number p,
		 Warehouse & wh, bool isworker, Ware_Index ware,
		 Warehouse::StockPolicy policy);

	uint8_t id() const override;

	void execute(Game & game) override;

	// Network (de-)serialization
	CmdSetStockPolicy(StreamRead & des);
	void serialize(StreamWrite & ser) override;

	// Savegame functions
	CmdSetStockPolicy();
	void Write(FileWrite &, Editor_Game_Base &, MapObjectSaver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, MapObjectLoader &) override;

private:
	Serial m_warehouse;
	bool m_isworker;
	Ware_Index m_ware;
	Warehouse::StockPolicy m_policy;
};

}

#endif  // end of include guard: WL_LOGIC_PLAYERCOMMAND_H
