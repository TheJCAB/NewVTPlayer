cmake_minimum_required(VERSION 3.20)

add_executable(VTPlayer
    VTPlayer.cpp

    stdafx.h # TODO: Remove
    plat.h
    Song.h
    Stream.h
    StringUtils.h
    targetver.h
    Timeline.h

    Channel.cpp         Channel.h
    Player.cpp          Player.h
    SoundMixer.cpp      SoundMixer.h

    Engine.h
    EnvelopeEngine.cpp
    ToneSlideEngine.cpp
    VibratoEngine.cpp

    Loaders.h
    ModLoader.cpp
    S3MLoader.cpp
    UnknownLoader.cpp
    XmLoader.cpp
)

target_link_options(VTPlayer
    PRIVATE
        -sEXIT_RUNTIME=0
        -sINVOKE_RUN=0
        -sENVIRONMENT=worker
        -sBUILD_AS_WORKER
        -sMODULARIZE
        -sEXPORT_NAME=kk
        #"-sEXPORTED_RUNTIME_METHODS=['ccall']"
        -sEXPORT_ES6
        --preload-file "C:/Users/jcab/Music/MODs/MOD/CHECKNOB.MOD@CHECKNOB.MOD"
)
