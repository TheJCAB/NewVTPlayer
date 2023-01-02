// VTPlayerCmd.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Loaders.h"
#include "Player.h"
#include "WriteWav.h"
#include "AudioOut.h"

#include <string>
#include <string_view>
#include <vector>
#include <random>
#include <span>

using namespace std::string_literals;

constexpr std::string_view ModFiles[] =
{
    R"(MOD\8CHN.MOD)",
    R"(MOD\A King is Born.Mod)",
    R"(MOD\AMAROK.MOD)",
    R"(MOD\AMBIENTP.MOD)",
    R"(MOD\ATOMIC2.MOD)",
    R"(MOD\AUSGEBUR.MOD)",
    R"(MOD\BASSBASE.MOD)",
    R"(MOD\Bomb.Mod)",
    R"(MOD\CESAR.MOD)",
    R"(MOD\CHECKNOB.MOD)",
    R"(MOD\CLOUD.MOD)",
    R"(MOD\CONDOM.MOD)",
    R"(MOD\CR-ORBIT.MOD)",
    R"(MOD\CR-STARL.MOD)",
    R"(MOD\cream_of_the_earth.mod)",
    R"(MOD\CRYSTALS.MOD)",
    R"(MOD\Cyberride.Mod)",
    R"(MOD\DANDLION.MOD)",
    R"(MOD\DANGER.MOD)",
    R"(MOD\DARKANGL.MOD)",
    R"(MOD\DARKDAYS.MOD)",
    R"(MOD\DEMODEMO.MOD)",
    R"(MOD\DEMOMOD1.MOD)",
    R"(MOD\DEMOMOD3.MOD)",
    R"(MOD\DOPE.MOD)",
    R"(MOD\ELYSIUM.MOD)",
    R"(MOD\EUPHONIK.MOD)",
    R"(MOD\FEAREXAM.MOD)",
    R"(MOD\FG93.MOD)",
    R"(MOD\FREESPA.MOD)",
    R"(MOD\FREESTYL.MOD)",
    R"(MOD\GLOBALT.MOD)",
    R"(MOD\GSLINGER.MOD)",
    R"(MOD\hedgehog.mod)",
    R"(MOD\HYPNOSST.MOD)",
    R"(MOD\Imaginery Woman.Mod)",
    R"(MOD\INCONEXI003.mod)",
    R"(MOD\INCONEXI005.mod)",
    R"(MOD\INCONEXI008.mod)",
    R"(MOD\INCONEXI014.mod)",
    R"(MOD\INTOFACE.MOD)",
    R"(MOD\INVTRO.MOD)",
    R"(MOD\ITDARK2.MOD)",
    R"(MOD\Klisje paa Klisje.Mod)",
    R"(MOD\KLISJE.MOD)",
    R"(MOD\Lemon Soda.Mod)",
    R"(MOD\LONGTECH.MOD)",
    R"(MOD\LUNATIC.MOD)",
    R"(MOD\m16a.mod)",
    R"(MOD\MASSHYST.MOD)",
    R"(MOD\Molecule's Revenge.Mod)",
    R"(MOD\MOMETHNG.MOD)",
    R"(MOD\MYDICKIN.MOD)",
    R"(MOD\NOCORNER.MOD)",
    R"(MOD\NOWWHAT3.MOD)",
    R"(MOD\NTOLVIDO.MOD)",
    R"(MOD\Occ-san-geen.Mod)",
    R"(MOD\OCC-SAN.MOD)",
    R"(MOD\ODE2PTK.MOD)",
    R"(MOD\OLDTIMES.MOD)",
    R"(MOD\OPTICNRV.MOD)",
    R"(MOD\Optimum Fuckup.Mod)",
    R"(MOD\overture.mod)",
    R"(MOD\physical_presence.mod)",
    R"(MOD\PRODUCT.MOD)",
    R"(MOD\RC_DROPS.MOD)",
    R"(MOD\RE-CALL.MOD)",
    R"(MOD\REDRAIN.MOD)",
    R"(MOD\ROCK.MOD)",
    R"(MOD\SEEDSOF.MOD)",
    R"(MOD\SHADOW.MOD)",
    R"(MOD\SOFTBRIL.MOD)",
    R"(MOD\SOMEOK5.MOD)",
    R"(MOD\SOMEOK6.MOD)",
    R"(MOD\SOMETHNG.MOD)",
    R"(MOD\SPA.MOD)",
    R"(MOD\Stagebox.Mod)",
    R"(MOD\Stardust Memories.Mod)",
    R"(MOD\THEDECAL.MOD)",
    R"(MOD\THEEND.MOD)",
    R"(MOD\THEJOUR.MOD)",
    R"(MOD\tilbury_fair.mod)",
    R"(MOD\TIME.MOD)",
    R"(MOD\VANGELIS.MOD)",
    R"(MOD\Whisper my Thoughts.Mod)",
    R"(MOD\Wizardry.Mod)",
    R"(S3M\(G)FOOLS.S3M)",
    R"(S3M\2ndr-sk.s3m)",
    R"(S3M\2ND_PM.S3M)",
    R"(S3M\2ND_SKAV.S3M)",
    R"(S3M\3RD.S3M)",
    R"(S3M\64mania.s3m)",
    R"(S3M\A94FINAL.S3M)",
    R"(S3M\AMAGO.S3M)",
    R"(S3M\amazonas.s3m)",
    R"(S3M\ANCIENT.S3M)",
    R"(S3M\ANGELH.S3M)",
    R"(S3M\ARMANI.S3M)",
    R"(S3M\ASY-WILD.S3M)",
    R"(S3M\autonom.s3m)",
    R"(S3M\BABYLON.S3M)",
    R"(S3M\BACKWARD.S3M)",
    R"(S3M\CAPTURED.S3M)",
    R"(S3M\CCITY12.S3M)",
    R"(S3M\CHAOS.S3M)",
    R"(S3M\CHIPSIM.S3M)",
    R"(S3M\CONDOM.S3M)",
    R"(S3M\crystald.s3m)",
    R"(S3M\ctgoblin.s3m)",
    R"(S3M\datajack.s3m)",
    R"(S3M\DEM.S3M)",
    R"(S3M\DEMII95.S3M)",
    R"(S3M\DESPAI.S3M)",
    R"(S3M\DESPAIR.S3M)",
    R"(S3M\DREAM3.S3M)",
    R"(S3M\DROLMIX.S3M)",
    R"(S3M\EDEN.S3M)",
    R"(S3M\ESP.S3M)",
    R"(S3M\ETHVISIO.S3M)",
    R"(S3M\EUPHONIK.S3M)",
    R"(S3M\EXPLORAT.S3M)",
    R"(S3M\EXPRESS.S3M)",
    R"(S3M\FLAME.S3M)",
    R"(S3M\FRIENDS.S3M)",
    R"(S3M\GET2.S3M)",
    R"(S3M\GMOTION.S3M)",
    R"(S3M\GSLINGER.S3M)",
    R"(S3M\HAPPY.S3M)",
    R"(S3M\hereyes.s3m)",
    R"(S3M\hgargoyl.s3m)",
    R"(S3M\HYMNE.S3M)",
    R"(S3M\HYPER.S3M)",
    R"(S3M\ICEFRONT.S3M)",
    R"(S3M\ID.S3M)",
    R"(S3M\IDEA.S3M)",
    R"(S3M\intro93.s3m)",
    R"(S3M\invtro94.s3m)",
    R"(S3M\K.S3M)",
    R"(S3M\K_SPIRAL.S3M)",
    R"(S3M\lavender.s3m)",
    R"(S3M\LIVIN2.S3M)",
    R"(S3M\LOOPS.S3M)",
    R"(S3M\LST_TIME.S3M)",
    R"(S3M\MAELSTRM.S3M)",
    R"(S3M\mangrove.s3m)",
    R"(S3M\MASKINA.S3M)",
    R"(S3M\MEGAMOD3.S3M)",
    R"(S3M\mercrain.s3m)",
    R"(S3M\MIRROR.S3M)",
    R"(S3M\mosquito.s3m)",
    R"(S3M\network.s3m)",
    R"(S3M\nighcats.s3m)",
    R"(S3M\noface.s3m)",
    R"(S3M\NOSREAL2.S3M)",
    R"(S3M\not4kids.s3m)",
    R"(S3M\NOWWHAT3.S3M)",
    R"(S3M\NTOLVIDO.S3M)",
    R"(S3M\ODESPAIR.S3M)",
    R"(S3M\OMNIPHIL.S3M)",
    R"(S3M\OPTICNRV.S3M)",
    R"(S3M\oxg.s3m)",
    R"(S3M\PANIC.S3M)",
    R"(S3M\party92.s3m)",
    R"(S3M\POSVIBRA.S3M)",
    R"(S3M\PUBNME.S3M)",
    R"(S3M\ramagard.s3m)",
    R"(S3M\RC_DROPS.S3M)",
    R"(S3M\REALIZE.S3M)",
    R"(S3M\ROCK.S3M)",
    R"(S3M\ROCKING.S3M)",
    R"(S3M\saint.s3m)",
    R"(S3M\SATELL.S3M)",
    R"(S3M\SEASON.S3M)",
    R"(S3M\SPASM95.S3M)",
    R"(S3M\spltfest.s3m)",
    R"(S3M\SPRING.S3M)",
    R"(S3M\STRSHINE.S3M)",
    R"(S3M\suzysgp.s3m)",
    R"(S3M\SWAVES2.S3M)",
    R"(S3M\symphony.s3m)",
    R"(S3M\TECHTECH.S3M)",
    R"(S3M\TOUGH.S3M)",
    R"(S3M\TURBULEN.S3M)",
    R"(S3M\war.s3m)",
    R"(S3M\WOPLSTC.S3M)",
    R"(XM\ANGEL6.XM)",
    R"(XM\DEADLOCK.XM)",
    R"(XM\DRAGON.XM)",
    R"(XM\ELEKTRON.XM)",
    R"(XM\EMPIRE4.XM)",
    R"(XM\FISHLIME.XM)",
    R"(XM\galway.xm)",
    R"(XM\Goatage.xm)",
    R"(XM\GUILDOFS.XM)",
    R"(XM\INTRO4IT.XM)",
    R"(XM\KLISJE2.XM)",
    R"(XM\MSHADOW4.XM)",
    R"(XM\OBLITIRT.XM)",
    R"(XM\PANIC.XM)",
    R"(XM\PROTECT.XM)",
    R"(XM\REFLECTR.XM)",
    R"(XM\RELAX.XM)",
    R"(XM\REZON3.XM)",
    R"(XM\SDELICB2.XM)",
    R"(XM\SLOWIT.XM)",
    R"(XM\songsect.xm)",
    R"(XM\synth_c_chaos-suprise.xm)",
    R"(XM\templsun.xm)",
    R"(XM\THEVOICE.XM)",
    R"(XM\TODO5.XM)",
};

template < typename T, size_t N, typename Engine >
T& PickRandom(T (&list)[N], Engine& engine)
{
    std::uniform_int_distribution<size_t> distribution{0, N-1};
    return list[distribution(engine)];
}

int VTPlayerCmd(std::span<std::string_view const> args)
{
    if (args[1] == "loadlist")
    {
        // A sort of stress test. Loads all files in the list.

        std::shared_ptr<VTPlayerLib::ModSong const> song;
        Generator<VTPlayerLib::Fragment> player;

        for (auto&& songFileName : ModFiles)
        {
            printf("%s\n", songFileName.data());
            VTPlayerLib::Stream s{ fopen((R"(C:\Users\jcab\Music\MODs\)"s + std::string(songFileName)).c_str(), "rb") };
            song = VTPlayerLib::LoadUnknown(s);
            player = MixBufferEngine(song, 48000u);
            for (auto fragment : player)
            {
                // Do nothing, just get them all.
                fragment = fragment;
            }
        }
    }
    else if (args[1] == "randomList")
    {
        auto randomEngine{std::default_random_engine(std::random_device{}())};
        for (;;)
        {
            auto const songFileName = PickRandom(ModFiles, randomEngine);
            printf("%s\n", songFileName.data());
            VTPlayerLib::Stream s{ fopen((R"(C:\Users\jcab\Music\MODs\)"s + std::string(songFileName)).c_str(), "rb") };
            auto song = VTPlayerLib::LoadUnknown(s);

            VTPlayerLib::PlayAudioSound(MixBufferEngine(song, 48000u));
        }
    }
    else if (args[1] == "play")
    {
        VTPlayerLib::Stream s{ fopen(args[2].data(), "rb") };
        auto song = VTPlayerLib::LoadUnknown(s);

        VTPlayerLib::PlayAudioSound(MixBufferEngine(song, 48000u));
    }
    else if (args[1] == "save")
    {
        //auto song = VTPlayerLib::LoadUnknown(VTPlayerLib::Stream{ fopen(R"(C:\Users\thejc\source\MODs\XM\thevoice.xm)", "rb") });
        VTPlayerLib::Stream s{ fopen(R"(C:\Users\thejc\source\Songs\rock.mod)", "rb") };
        auto song = VTPlayerLib::LoadUnknown(s);

        std::vector<float> buffer;

        for (auto&& fragment : MixBufferEngine(song, 44100u))
        {
            auto const offset = buffer.size();
            auto const size = static_cast<size_t>(fragment->GetCount());
            buffer.resize(offset + size);
            fragment->MixMono(VTPlayerLib::MakeSpan(buffer, offset, size), 0, true);
        }

        // Output the sound in the buffer.
        VTPlayerLib::WriteWavMono(LR"(C:\TEMP\File.wav)", std::span<float>{buffer.data(), buffer.size()});
    }
    else if (args[1] == "positions")
    {
        VTPlayerLib::Stream s{ fopen(args[2].data(), "rb") };
        auto song = VTPlayerLib::LoadUnknown(s);

        for (auto const modPosition : VTPlayerLib::ModPositionEnumerator(*song, 44100u))
        {
            auto const line = VTPlayerLib::RenderPosition(*song, modPosition);
            wprintf(L"%ls\n", line.c_str());
        }
    }

    return 0;
}

int main(int argc, char const* const* argv)
{
    std::vector<std::string_view> const args{ argv, argv + argc };
    return VTPlayerCmd(VTPlayerLib::MakeSpan(args));
}
