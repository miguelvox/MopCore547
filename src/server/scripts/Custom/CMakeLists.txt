# Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

set(scripts_STAT_SRCS
  ${scripts_STAT_SRCS}
    Custom/event_april.cpp
	Custom/script_store.cpp
    Custom/PVP/duel_reset.cpp
    Custom/PVP/arena_fast_start.cpp
    Custom/Tools/npc_promo.cpp
	Custom/Tools/teleguy.cpp
	Custom/Tools/petguy.cpp
)

message("  -> Prepared: Custom")
