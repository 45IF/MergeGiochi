#include "plugin.h"
#include "CHud.h"

// Solo verifica del caricamento: nessun asset o comportamento di gioco modificato.
class ProvaASI {
public:
    ProvaASI() {
        const auto version = plugin::GetGameVersion();
        if (version != GAME_10US_COMPACT && version != GAME_10US_HOODLUM)
            return;
        plugin::Events::initGameEvent += [] {
            CHud::SetHelpMessage("MergeGiochi: plugin ASI caricato", true, false, false);
        };
    }
} provaASI;
