
var vtPlayerWorker;

var sharedCommunication;

var buttonEl         = document.getElementById('start-button');
var modListEl        = document.querySelector('#modList');
var songTimeline     = document.querySelector('#songTimeline');
var songPositionText = document.querySelector('#songPositionText');
var songTimePosition = document.querySelector('#songTimePosition');
var songLength       = document.querySelector('#songLength');
var infoText         = document.querySelector('#infoText');


var playerPreviousButton    = document.querySelector('#playerPreviousButton');
var playerRewindButton      = document.querySelector('#playerRewindButton');
var playerPlayStopButton    = document.querySelector('#playerPlayStopButton');
var playerFastForwardButton = document.querySelector('#playerFastForwardButton');
var playerNextButton        = document.querySelector('#playerNextButton');

const patternLineList = document.getElementById("patternLineList")

var audioContext;

var isPaused = true;

const MySource         = { whoURL: "https://jcabs-rumblings.com/", modPrefix: "https://jcabs-rumblings.com/MODs/"                  };
const ModArchiveSource = { whoURL: "https://modarchive.org/"     , modPrefix: "https://api.modarchive.org/downloads.php?moduleid=" };

const ModFileList =
[
    { source: MySource, urlPath: "MOD/8CHN.MOD"               , name: "8CHN.MOD"                 },
    { source: MySource, urlPath: "MOD/A King is Born.Mod"     , name: "A King is Born.Mod"       },
    { source: MySource, urlPath: "MOD/AMAROK.MOD"             , name: "AMAROK.MOD"               },
    { source: MySource, urlPath: "MOD/AMBIENTP.MOD"           , name: "AMBIENTP.MOD"             },
    { source: MySource, urlPath: "MOD/ATOMIC2.MOD"            , name: "ATOMIC2.MOD"              },
    { source: MySource, urlPath: "MOD/AUSGEBUR.MOD"           , name: "AUSGEBUR.MOD"             },
    { source: MySource, urlPath: "MOD/BASSBASE.MOD"           , name: "BASSBASE.MOD"             },
    { source: MySource, urlPath: "MOD/Bomb.Mod"               , name: "Bomb.Mod"                 },
    { source: MySource, urlPath: "MOD/CESAR.MOD"              , name: "CESAR.MOD"                },
    { source: MySource, urlPath: "MOD/CHECKNOB.MOD"           , name: "CHECKNOB.MOD"             },
    { source: MySource, urlPath: "MOD/CLOUD.MOD"              , name: "CLOUD.MOD"                },
    { source: MySource, urlPath: "MOD/CONDOM.MOD"             , name: "CONDOM.MOD"               },
    { source: MySource, urlPath: "MOD/CR-ORBIT.MOD"           , name: "CR-ORBIT.MOD"             },
    { source: MySource, urlPath: "MOD/CR-STARL.MOD"           , name: "CR-STARL.MOD"             },
    { source: MySource, urlPath: "MOD/cream_of_the_earth.mod" , name: "cream_of_the_earth.mod"   },
    { source: MySource, urlPath: "MOD/CRYSTALS.MOD"           , name: "CRYSTALS.MOD"             },
    { source: MySource, urlPath: "MOD/Cyberride.Mod"          , name: "Cyberride.Mod"            },
    { source: MySource, urlPath: "MOD/DANDLION.MOD"           , name: "DANDLION.MOD"             },
    { source: MySource, urlPath: "MOD/DANGER.MOD"             , name: "DANGER.MOD"               },
    { source: MySource, urlPath: "MOD/DARKANGL.MOD"           , name: "DARKANGL.MOD"             },
    { source: MySource, urlPath: "MOD/DARKDAYS.MOD"           , name: "DARKDAYS.MOD"             },
    { source: MySource, urlPath: "MOD/DEMODEMO.MOD"           , name: "DEMODEMO.MOD"             },
    { source: MySource, urlPath: "MOD/DEMOMOD1.MOD"           , name: "DEMOMOD1.MOD"             },
    { source: MySource, urlPath: "MOD/DEMOMOD3.MOD"           , name: "DEMOMOD3.MOD"             },
    { source: MySource, urlPath: "MOD/DOPE.MOD"               , name: "DOPE.MOD"                 },
    { source: MySource, urlPath: "MOD/ELYSIUM.MOD"            , name: "ELYSIUM.MOD"              },
    { source: MySource, urlPath: "MOD/EUPHONIK.MOD"           , name: "EUPHONIK.MOD"             },
    { source: MySource, urlPath: "MOD/FEAREXAM.MOD"           , name: "FEAREXAM.MOD"             },
    { source: MySource, urlPath: "MOD/FG93.MOD"               , name: "FG93.MOD"                 },
    { source: MySource, urlPath: "MOD/FREESPA.MOD"            , name: "FREESPA.MOD"              },
    { source: MySource, urlPath: "MOD/FREESTYL.MOD"           , name: "FREESTYL.MOD"             },
    { source: MySource, urlPath: "MOD/GLOBALT.MOD"            , name: "GLOBALT.MOD"              },
    { source: MySource, urlPath: "MOD/GSLINGER.MOD"           , name: "GSLINGER.MOD"             },
    { source: MySource, urlPath: "MOD/hedgehog.mod"           , name: "hedgehog.mod"             },
    { source: MySource, urlPath: "MOD/HYPNOSST.MOD"           , name: "HYPNOSST.MOD"             },
    { source: MySource, urlPath: "MOD/Imaginery Woman.Mod"    , name: "Imaginery Woman.Mod"      },
    { source: MySource, urlPath: "MOD/INCONEXI003.mod"        , name: "INCONEXI003.mod"          },
    { source: MySource, urlPath: "MOD/INCONEXI005.mod"        , name: "INCONEXI005.mod"          },
    { source: MySource, urlPath: "MOD/INCONEXI008.mod"        , name: "INCONEXI008.mod"          },
    { source: MySource, urlPath: "MOD/INCONEXI014.mod"        , name: "INCONEXI014.mod"          },
    { source: MySource, urlPath: "MOD/INTOFACE.MOD"           , name: "INTOFACE.MOD"             },
    { source: MySource, urlPath: "MOD/INVTRO.MOD"             , name: "INVTRO.MOD"               },
    { source: MySource, urlPath: "MOD/ITDARK2.MOD"            , name: "ITDARK2.MOD"              },
    { source: MySource, urlPath: "MOD/Klisje paa Klisje.Mod"  , name: "Klisje paa Klisje.Mod"    },
    { source: MySource, urlPath: "MOD/KLISJE.MOD"             , name: "KLISJE.MOD"               },
    { source: MySource, urlPath: "MOD/Lemon Soda.Mod"         , name: "Lemon Soda.Mod"           },
    { source: MySource, urlPath: "MOD/LONGTECH.MOD"           , name: "LONGTECH.MOD"             },
    { source: MySource, urlPath: "MOD/LUNATIC.MOD"            , name: "LUNATIC.MOD"              },
    { source: MySource, urlPath: "MOD/m16a.mod"               , name: "m16a.mod"                 },
    { source: MySource, urlPath: "MOD/MASSHYST.MOD"           , name: "MASSHYST.MOD"             },
    { source: MySource, urlPath: "MOD/Molecule's Revenge.Mod" , name: "Molecule's Revenge.Mod"   },
    { source: MySource, urlPath: "MOD/MOMETHNG.MOD"           , name: "MOMETHNG.MOD"             },
    { source: MySource, urlPath: "MOD/MYDICKIN.MOD"           , name: "MYDICKIN.MOD"             },
    { source: MySource, urlPath: "MOD/NOCORNER.MOD"           , name: "NOCORNER.MOD"             },
    { source: MySource, urlPath: "MOD/NOWWHAT3.MOD"           , name: "NOWWHAT3.MOD"             },
    { source: MySource, urlPath: "MOD/NTOLVIDO.MOD"           , name: "NTOLVIDO.MOD"             },
    { source: MySource, urlPath: "MOD/Occ-san-geen.Mod"       , name: "Occ-san-geen.Mod"         },
    { source: MySource, urlPath: "MOD/OCC-SAN.MOD"            , name: "OCC-SAN.MOD"              },
    { source: MySource, urlPath: "MOD/ODE2PTK.MOD"            , name: "ODE2PTK.MOD"              },
    { source: MySource, urlPath: "MOD/OLDTIMES.MOD"           , name: "OLDTIMES.MOD"             },
    { source: MySource, urlPath: "MOD/OPTICNRV.MOD"           , name: "OPTICNRV.MOD"             },
    { source: MySource, urlPath: "MOD/Optimum Fuckup.Mod"     , name: "Optimum Fuckup.Mod"       },
    { source: MySource, urlPath: "MOD/overture.mod"           , name: "overture.mod"             },
    { source: MySource, urlPath: "MOD/physical_presence.mod"  , name: "physical_presence.mod"    },
    { source: MySource, urlPath: "MOD/PRODUCT.MOD"            , name: "PRODUCT.MOD"              },
    { source: MySource, urlPath: "MOD/RC_DROPS.MOD"           , name: "RC_DROPS.MOD"             },
    { source: MySource, urlPath: "MOD/RE-CALL.MOD"            , name: "RE-CALL.MOD"              },
    { source: MySource, urlPath: "MOD/REDRAIN.MOD"            , name: "REDRAIN.MOD"              },
    { source: MySource, urlPath: "MOD/ROCK.MOD"               , name: "ROCK.MOD"                 },
    { source: MySource, urlPath: "MOD/SEEDSOF.MOD"            , name: "SEEDSOF.MOD"              },
    { source: MySource, urlPath: "MOD/SHADOW.MOD"             , name: "SHADOW.MOD"               },
    { source: MySource, urlPath: "MOD/SOFTBRIL.MOD"           , name: "SOFTBRIL.MOD"             },
    { source: MySource, urlPath: "MOD/SOMEOK5.MOD"            , name: "SOMEOK5.MOD"              },
    { source: MySource, urlPath: "MOD/SOMEOK6.MOD"            , name: "SOMEOK6.MOD"              },
    { source: MySource, urlPath: "MOD/SOMETHNG.MOD"           , name: "SOMETHNG.MOD"             },
    { source: MySource, urlPath: "MOD/SPA.MOD"                , name: "SPA.MOD"                  },
    { source: MySource, urlPath: "MOD/Stagebox.Mod"           , name: "Stagebox.Mod"             },
    { source: MySource, urlPath: "MOD/Stardust Memories.Mod"  , name: "Stardust Memories.Mod"    },
    { source: MySource, urlPath: "MOD/THEDECAL.MOD"           , name: "THEDECAL.MOD"             },
    { source: MySource, urlPath: "MOD/THEEND.MOD"             , name: "THEEND.MOD"               },
    { source: MySource, urlPath: "MOD/THEJOUR.MOD"            , name: "THEJOUR.MOD"              },
    { source: MySource, urlPath: "MOD/tilbury_fair.mod"       , name: "tilbury_fair.mod"         },
    { source: MySource, urlPath: "MOD/TIME.MOD"               , name: "TIME.MOD"                 },
    { source: MySource, urlPath: "MOD/VANGELIS.MOD"           , name: "VANGELIS.MOD"             },
    { source: MySource, urlPath: "MOD/Whisper my Thoughts.Mod", name: "Whisper my Thoughts.Mod"  },
    { source: MySource, urlPath: "MOD/Wizardry.Mod"           , name: "Wizardry.Mod"             },
    { source: MySource, urlPath: "S3M/(G)FOOLS.S3M"           , name: "(G)FOOLS.S3M"             },
    { source: MySource, urlPath: "S3M/2ndr-sk.s3m"            , name: "2ndr-sk.s3m"              },
    { source: MySource, urlPath: "S3M/2ND_PM.S3M"             , name: "2ND_PM.S3M"               },
    { source: MySource, urlPath: "S3M/2ND_SKAV.S3M"           , name: "2ND_SKAV.S3M"             },
    { source: MySource, urlPath: "S3M/3RD.S3M"                , name: "3RD.S3M"                  },
    { source: MySource, urlPath: "S3M/64mania.s3m"            , name: "64mania.s3m"              },
    { source: MySource, urlPath: "S3M/A94FINAL.S3M"           , name: "A94FINAL.S3M"             },
    { source: MySource, urlPath: "S3M/AMAGO.S3M"              , name: "AMAGO.S3M"                },
    { source: MySource, urlPath: "S3M/amazonas.s3m"           , name: "amazonas.s3m"             },
    { source: MySource, urlPath: "S3M/ANCIENT.S3M"            , name: "ANCIENT.S3M"              },
    { source: MySource, urlPath: "S3M/ANGELH.S3M"             , name: "ANGELH.S3M"               },
    { source: MySource, urlPath: "S3M/ARMANI.S3M"             , name: "ARMANI.S3M"               },
    { source: MySource, urlPath: "S3M/ASY-WILD.S3M"           , name: "ASY-WILD.S3M"             },
    { source: MySource, urlPath: "S3M/autonom.s3m"            , name: "autonom.s3m"              },
    { source: MySource, urlPath: "S3M/BABYLON.S3M"            , name: "BABYLON.S3M"              },
    { source: MySource, urlPath: "S3M/BACKWARD.S3M"           , name: "BACKWARD.S3M"             },
    { source: MySource, urlPath: "S3M/CAPTURED.S3M"           , name: "CAPTURED.S3M"             },
    { source: MySource, urlPath: "S3M/CCITY12.S3M"            , name: "CCITY12.S3M"              },
    { source: MySource, urlPath: "S3M/CHAOS.S3M"              , name: "CHAOS.S3M"                },
    { source: MySource, urlPath: "S3M/CHIPSIM.S3M"            , name: "CHIPSIM.S3M"              },
    { source: MySource, urlPath: "S3M/CONDOM.S3M"             , name: "CONDOM.S3M"               },
    { source: MySource, urlPath: "S3M/crystald.s3m"           , name: "crystald.s3m"             },
    { source: MySource, urlPath: "S3M/ctgoblin.s3m"           , name: "ctgoblin.s3m"             },
    { source: MySource, urlPath: "S3M/datajack.s3m"           , name: "datajack.s3m"             },
    { source: MySource, urlPath: "S3M/DEM.S3M"                , name: "DEM.S3M"                  },
    { source: MySource, urlPath: "S3M/DEMII95.S3M"            , name: "DEMII95.S3M"              },
    { source: MySource, urlPath: "S3M/DESPAI.S3M"             , name: "DESPAI.S3M"               },
    { source: MySource, urlPath: "S3M/DESPAIR.S3M"            , name: "DESPAIR.S3M"              },
    { source: MySource, urlPath: "S3M/DREAM3.S3M"             , name: "DREAM3.S3M"               },
    { source: MySource, urlPath: "S3M/DROLMIX.S3M"            , name: "DROLMIX.S3M"              },
    { source: MySource, urlPath: "S3M/EDEN.S3M"               , name: "EDEN.S3M"                 },
    { source: MySource, urlPath: "S3M/ESP.S3M"                , name: "ESP.S3M"                  },
    { source: MySource, urlPath: "S3M/ETHVISIO.S3M"           , name: "ETHVISIO.S3M"             },
    { source: MySource, urlPath: "S3M/EUPHONIK.S3M"           , name: "EUPHONIK.S3M"             },
    { source: MySource, urlPath: "S3M/EXPLORAT.S3M"           , name: "EXPLORAT.S3M"             },
    { source: MySource, urlPath: "S3M/EXPRESS.S3M"            , name: "EXPRESS.S3M"              },
    { source: MySource, urlPath: "S3M/FLAME.S3M"              , name: "FLAME.S3M"                },
    { source: MySource, urlPath: "S3M/FRIENDS.S3M"            , name: "FRIENDS.S3M"              },
    { source: MySource, urlPath: "S3M/GET2.S3M"               , name: "GET2.S3M"                 },
    { source: MySource, urlPath: "S3M/GMOTION.S3M"            , name: "GMOTION.S3M"              },
    { source: MySource, urlPath: "S3M/GSLINGER.S3M"           , name: "GSLINGER.S3M"             },
    { source: MySource, urlPath: "S3M/HAPPY.S3M"              , name: "HAPPY.S3M"                },
    { source: MySource, urlPath: "S3M/hereyes.s3m"            , name: "hereyes.s3m"              },
    { source: MySource, urlPath: "S3M/hgargoyl.s3m"           , name: "hgargoyl.s3m"             },
    { source: MySource, urlPath: "S3M/HYMNE.S3M"              , name: "HYMNE.S3M"                },
    { source: MySource, urlPath: "S3M/HYPER.S3M"              , name: "HYPER.S3M"                },
    { source: MySource, urlPath: "S3M/ICEFRONT.S3M"           , name: "ICEFRONT.S3M"             },
    { source: MySource, urlPath: "S3M/ID.S3M"                 , name: "ID.S3M"                   },
    { source: MySource, urlPath: "S3M/IDEA.S3M"               , name: "IDEA.S3M"                 },
    { source: MySource, urlPath: "S3M/intro93.s3m"            , name: "intro93.s3m"              },
    { source: MySource, urlPath: "S3M/invtro94.s3m"           , name: "invtro94.s3m"             },
    { source: MySource, urlPath: "S3M/K.S3M"                  , name: "K.S3M"                    },
    { source: MySource, urlPath: "S3M/K_SPIRAL.S3M"           , name: "K_SPIRAL.S3M"             },
    { source: MySource, urlPath: "S3M/lavender.s3m"           , name: "lavender.s3m"             },
    { source: MySource, urlPath: "S3M/LIVIN2.S3M"             , name: "LIVIN2.S3M"               },
    { source: MySource, urlPath: "S3M/LOOPS.S3M"              , name: "LOOPS.S3M"                },
    { source: MySource, urlPath: "S3M/LST_TIME.S3M"           , name: "LST_TIME.S3M"             },
    { source: MySource, urlPath: "S3M/MAELSTRM.S3M"           , name: "MAELSTRM.S3M"             },
    { source: MySource, urlPath: "S3M/mangrove.s3m"           , name: "mangrove.s3m"             },
    { source: MySource, urlPath: "S3M/MASKINA.S3M"            , name: "MASKINA.S3M"              },
    { source: MySource, urlPath: "S3M/MEGAMOD3.S3M"           , name: "MEGAMOD3.S3M"             },
    { source: MySource, urlPath: "S3M/mercrain.s3m"           , name: "mercrain.s3m"             },
    { source: MySource, urlPath: "S3M/MIRROR.S3M"             , name: "MIRROR.S3M"               },
    { source: MySource, urlPath: "S3M/mosquito.s3m"           , name: "mosquito.s3m"             },
    { source: MySource, urlPath: "S3M/network.s3m"            , name: "network.s3m"              },
    { source: MySource, urlPath: "S3M/nighcats.s3m"           , name: "nighcats.s3m"             },
    { source: MySource, urlPath: "S3M/noface.s3m"             , name: "noface.s3m"               },
    { source: MySource, urlPath: "S3M/NOSREAL2.S3M"           , name: "NOSREAL2.S3M"             },
    { source: MySource, urlPath: "S3M/not4kids.s3m"           , name: "not4kids.s3m"             },
    { source: MySource, urlPath: "S3M/NOWWHAT3.S3M"           , name: "NOWWHAT3.S3M"             },
    { source: MySource, urlPath: "S3M/NTOLVIDO.S3M"           , name: "NTOLVIDO.S3M"             },
    { source: MySource, urlPath: "S3M/ODESPAIR.S3M"           , name: "ODESPAIR.S3M"             },
    { source: MySource, urlPath: "S3M/OMNIPHIL.S3M"           , name: "OMNIPHIL.S3M"             },
    { source: MySource, urlPath: "S3M/OPTICNRV.S3M"           , name: "OPTICNRV.S3M"             },
    { source: MySource, urlPath: "S3M/oxg.s3m"                , name: "oxg.s3m"                  },
    { source: MySource, urlPath: "S3M/PANIC.S3M"              , name: "PANIC.S3M"                },
    { source: MySource, urlPath: "S3M/party92.s3m"            , name: "party92.s3m"              },
    { source: MySource, urlPath: "S3M/POSVIBRA.S3M"           , name: "POSVIBRA.S3M"             },
    { source: MySource, urlPath: "S3M/PUBNME.S3M"             , name: "PUBNME.S3M"               },
    { source: MySource, urlPath: "S3M/ramagard.s3m"           , name: "ramagard.s3m"             },
    { source: MySource, urlPath: "S3M/RC_DROPS.S3M"           , name: "RC_DROPS.S3M"             },
    { source: MySource, urlPath: "S3M/REALIZE.S3M"            , name: "REALIZE.S3M"              },
    { source: MySource, urlPath: "S3M/ROCK.S3M"               , name: "ROCK.S3M"                 },
    { source: MySource, urlPath: "S3M/ROCKING.S3M"            , name: "ROCKING.S3M"              },
    { source: MySource, urlPath: "S3M/saint.s3m"              , name: "saint.s3m"                },
    { source: MySource, urlPath: "S3M/SATELL.S3M"             , name: "SATELL.S3M"               },
    { source: MySource, urlPath: "S3M/SEASON.S3M"             , name: "SEASON.S3M"               },
    { source: MySource, urlPath: "S3M/SPASM95.S3M"            , name: "SPASM95.S3M"              },
    { source: MySource, urlPath: "S3M/spltfest.s3m"           , name: "spltfest.s3m"             },
    { source: MySource, urlPath: "S3M/SPRING.S3M"             , name: "SPRING.S3M"               },
    { source: MySource, urlPath: "S3M/STRSHINE.S3M"           , name: "STRSHINE.S3M"             },
    { source: MySource, urlPath: "S3M/suzysgp.s3m"            , name: "suzysgp.s3m"              },
    { source: MySource, urlPath: "S3M/SWAVES2.S3M"            , name: "SWAVES2.S3M"              },
    { source: MySource, urlPath: "S3M/symphony.s3m"           , name: "symphony.s3m"             },
    { source: MySource, urlPath: "S3M/TECHTECH.S3M"           , name: "TECHTECH.S3M"             },
    { source: MySource, urlPath: "S3M/TOUGH.S3M"              , name: "TOUGH.S3M"                },
    { source: MySource, urlPath: "S3M/TURBULEN.S3M"           , name: "TURBULEN.S3M"             },
    { source: MySource, urlPath: "S3M/war.s3m"                , name: "war.s3m"                  },
    { source: MySource, urlPath: "S3M/WOPLSTC.S3M"            , name: "WOPLSTC.S3M"              },
    { source: MySource, urlPath: "XM/ANGEL6.XM"               , name: "ANGEL6.XM"                },
    { source: MySource, urlPath: "XM/DEADLOCK.XM"             , name: "DEADLOCK.XM"              },
    { source: MySource, urlPath: "XM/DRAGON.XM"               , name: "DRAGON.XM"                },
    { source: MySource, urlPath: "XM/ELEKTRON.XM"             , name: "ELEKTRON.XM"              },
    { source: MySource, urlPath: "XM/EMPIRE4.XM"              , name: "EMPIRE4.XM"               },
    { source: MySource, urlPath: "XM/FISHLIME.XM"             , name: "FISHLIME.XM"              },
    { source: MySource, urlPath: "XM/galway.xm"               , name: "galway.xm"                },
    { source: MySource, urlPath: "XM/Goatage.xm"              , name: "Goatage.xm"               },
    { source: MySource, urlPath: "XM/GUILDOFS.XM"             , name: "GUILDOFS.XM"              },
    { source: MySource, urlPath: "XM/INTRO4IT.XM"             , name: "INTRO4IT.XM"              },
    { source: MySource, urlPath: "XM/KLISJE2.XM"              , name: "KLISJE2.XM"               },
    { source: MySource, urlPath: "XM/MSHADOW4.XM"             , name: "MSHADOW4.XM"              },
    { source: MySource, urlPath: "XM/OBLITIRT.XM"             , name: "OBLITIRT.XM"              },
    { source: MySource, urlPath: "XM/PANIC.XM"                , name: "PANIC.XM"                 },
    { source: MySource, urlPath: "XM/PROTECT.XM"              , name: "PROTECT.XM"               },
    { source: MySource, urlPath: "XM/REFLECTR.XM"             , name: "REFLECTR.XM"              },
    { source: MySource, urlPath: "XM/RELAX.XM"                , name: "RELAX.XM"                 },
    { source: MySource, urlPath: "XM/REZON3.XM"               , name: "REZON3.XM"                },
    { source: MySource, urlPath: "XM/SDELICB2.XM"             , name: "SDELICB2.XM"              },
    { source: MySource, urlPath: "XM/SLOWIT.XM"               , name: "SLOWIT.XM"                },
    { source: MySource, urlPath: "XM/songsect.xm"             , name: "songsect.xm"              },
    { source: MySource, urlPath: "XM/synth_c_chaos-suprise.xm", name: "synth_c_chaos-suprise.xm" },
    { source: MySource, urlPath: "XM/templsun.xm"             , name: "templsun.xm"              },
    { source: MySource, urlPath: "XM/THEVOICE.XM"             , name: "THEVOICE.XM"              },
    { source: MySource, urlPath: "XM/TODO5.XM"                , name: "TODO5.XM"                 },

    { source: ModArchiveSource, urlPath: "72098#id_-_space_deliria.s3m", name: "id_-_space_deliria.s3m" },
];

var modListElement;

var playingRandom = false;
var currentIndex = -1;
var currentPercent = 0;
var currentSeconds = 0;
var totalSeconds = 0;

async function startMod(index, dontDrain = false)
{
    if (index < 0)
    {
        await pauseAudio();
        return;
    }

    const mod = ModFileList[index];
    currentIndex = index;

    if (vtPlayerWorker == undefined)
    {
        await startAudio();
    }

    currentPercent = 0;
    currentSeconds = 0;
    totalSeconds   = 1;

    vtPlayerWorker.postMessage({
        modUrl: mod.source.modPrefix + mod.urlPath,
        songId: index,
        dontDrain: dontDrain,
    });

    await resumeAudio();
};

window.populateModList = (modListEl) =>
{
    modListElement = modListEl;
    for (var modFile of ModFileList)
    {
        var option = document.createElement('option');
        option.text = modFile.name;
        modListElement.add(option, null);
    }

    modListElement.addEventListener('change', async () =>
        {
            console.log('List selected: ', modListElement.selectedIndex, " ", modListElement.value);
            await startMod(modListElement.selectedIndex);
            playingRandom = false;
        },
        false
    );
};

let patternSelectedLine = -1

// Replaces the displayed lines and clears the selection.
function setPatternLines(lines) {
    patternLineList.replaceChildren(...lines.map((text, i) => {
        const div = document.createElement("div")
        div.className = "lineItem"
        div.textContent = text
        div.addEventListener("click", () => selectPatternLine(i))
        return div
    }))
    patternSelectedLine = -1
}

// Selects a line (-1 clears) and scrolls it into view.
function selectPatternLine(index) {
    const items = patternLineList.children
    if (items[patternSelectedLine]) items[patternSelectedLine].classList.remove("selected")
    patternSelectedLine = items[index] ? index : -1
    if (patternSelectedLine >= 0) {
        items[patternSelectedLine].classList.add("selected")
        items[patternSelectedLine].scrollIntoView({ block: "nearest" })
    }
}


window.startAudio = async () =>
{
    if (audioContext == undefined)
    {
        audioContext = new AudioContext();
        audioContext.resume();
    }

    const SharedCommunicationModule = await import("./SharedCommunicationBuffer.js");

    console.log(crossOriginIsolated);

    // 10 second ring buffer should cover any conceivable glitch?
    const buffer = SharedCommunicationModule.initializeSharedBuffer(audioContext.sampleRate * 10);
    sharedCommunication = new SharedCommunicationModule.SharedCommunication(buffer);

    await audioContext.audioWorklet.addModule('/js/AudioWorklet.js');

    let node = new AudioWorkletNode(audioContext, 'VTPlayerAudioWorklet');
    node.port.postMessage({
        sharedCommunicationBuffer: buffer,
    });
    node.connect(audioContext.destination);

    vtPlayerWorker = new Worker('/js/VTPlayerWorker.js');
    vtPlayerWorker.postMessage({
        sharedCommunicationBuffer: buffer,
        sampleRate:                audioContext.sampleRate,
    });
    var resolveIsInitialized;
    const isInitialized = new Promise(resolved => resolveIsInitialized = resolved);
    vtPlayerWorker.onmessage = (event) =>
    {
        resolveIsInitialized(event);
        vtPlayerWorker.onmessage = (event) =>
        {
            // Must run before the position update, which selects a line in the new pattern.
            if ('newPattern' in event.data)
            {
                setPatternLines(event.data.newPatternLines)
                selectPatternLine(0)
            }
            if ('position' in event.data)
            {
                currentPercent = event.data.percent;
                currentSeconds = event.data.seconds;
                totalSeconds   = event.data.totalSeconds;
                
                //console.log(event.data.position, ' ', event.data.line, ' ', event.data.percent);
                songTimeline.value = event.data.percent;
                {
                    const minutes = Math.floor(event.data.seconds / 60);
                    const secondsHi = Math.floor(event.data.seconds / 10) - minutes * 6;
                    const secondsLo = Math.floor(event.data.seconds) - minutes * 60 - secondsHi * 10;
                    songTimePosition.textContent = `${minutes}:${secondsHi}${secondsLo}`;
                }
                songPositionText.textContent = `${event.data.position}(${event.data.pattern}).${event.data.line}/${event.data.patternLength}`;
                {
                    const minutes = Math.floor(event.data.totalSeconds / 60);
                    const secondsHi = Math.floor(event.data.totalSeconds / 10) - minutes * 6;
                    const secondsLo = Math.floor(event.data.totalSeconds) - minutes * 60 - secondsHi * 10;
                    songLength.textContent = `${minutes}:${secondsHi}${secondsLo}`;
                }
                selectPatternLine(event.data.line)
            }
            if ('startNewSongId' in event.data)
            {
                // Delayed selection of the module in the list.
                // This happens when we started the song without draining the ring buffer.
                modListElement.value = ModFileList[event.data.startNewSongId].name;
            }
            if ('songInfo' in event.data)
            {
                infoText.innerHTML = event.data.songInfo.replaceAll("\n", "<br>");
            }
            if ('songEnded' in event.data)
            {
                var index;
                if (playingRandom)
                {
                    index = Math.floor(Math.random() * ModFileList.length);
                }
                else
                {
                    index = currentIndex + 1;
                }

                // Note: We don't drain the ring buffer in this case. We're just setting up the next song.
                startMod(index, true);
            }
        }
    }

    //await new Promise(resolve => { node.port.onmessage = (event) => { resolve(event); } });
    await isInitialized;

    isPaused = false;
    playerPreviousButton   .addEventListener('click', onPrevious   , false);
    playerRewindButton     .addEventListener('click', onRewind     , false);
    playerPlayStopButton   .addEventListener('click', onPauseResume, false);
    playerFastForwardButton.addEventListener('click', onFastForward, false);
    playerNextButton       .addEventListener('click', onNext       , false);
    songTimeline.addEventListener('click', onSetPercent , false);
    sharedCommunication.requestStartOutput();
};

async function onStartRandomSong()
{
    buttonEl.disabled = true;

    const index = Math.floor(Math.random() * ModFileList.length);

    const mod = ModFileList[index];

    modListElement.value = mod.name;
    await startMod(index);
    playingRandom = true;

    buttonEl.disabled = false;
};

async function resumeAudio()
{
    await sharedCommunication.requestStartOutput();
    playerPlayStopButton.innerHTML = '&#9208;&#65039;';
    isPaused = false;
}

async function pauseAudio()
{
        await sharedCommunication.requestStopOutput();
        playerPlayStopButton.innerHTML = '&#9654;&#65039;';
        isPaused = true;
}


async function onPrevious()
{
    // If we're "enough" into the song, rewind back to the beginning.
    if (currentSeconds > 10 || currentPercent >= 30)
    {
        vtPlayerWorker.postMessage({ setSeconds: 0 });
        return;
    }

    // Otherwise, go to the previous song.

    const index = currentIndex - 1;
    if (index < 0)
    {
        index = ModFileList.length;
    }

    const mod = ModFileList[index];

    modListElement.value = mod.name;
    await startMod(index);
};

async function onRewind()
{
    var newSeconds = currentSeconds - 10;
    if (newSeconds < 0)
    {
        newSeconds = 0;
    }

    vtPlayerWorker.postMessage({ setSeconds: newSeconds });
};

async function onPauseResume()
{
    await isPaused ? resumeAudio() : pauseAudio();
};

async function onFastForward()
{
    var newSeconds = currentSeconds + 10;
    if (newSeconds > totalSeconds)
    {
        newSeconds = totalSeconds;
    }

    vtPlayerWorker.postMessage({ setSeconds: newSeconds });
};

async function onNext()
{
    const index = currentIndex + 1;
    if (index >= ModFileList.length)
    {
        index = 0;
    }

    const mod = ModFileList[index];

    modListElement.value = mod.name;
    await startMod(index);
};

function onSetPercent(event)
{
    vtPlayerWorker.postMessage({ setPercent: event.offsetX / songTimeline.offsetWidth });
};

window.initialize = async () =>
{
    populateModList(modListEl);

    // A simple onLoad handler. It also handles user gesture to unlock the audio
    // playback.
    window.addEventListener('load', async () => {
        buttonEl.addEventListener('click', onStartRandomSong, false);
    });
}
