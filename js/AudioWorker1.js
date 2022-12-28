var memory;
var add;
var sub;

var AllocateAudioBuffer;
var FreeAudioBuffer;
var VTPlayerGetAudio;

//calc.Module.onRuntimeInitialized = _ => {
//    console.log(calc.Module);
//}

onmessage = async (event) => {

    const { default : createVTPlayer } = await import('/js/bin/VTPlayer.js');

    console.log(createVTPlayer);

    const VTPlayerModule = await createVTPlayer();

    console.log(VTPlayerModule);
    console.log(Object.keys(VTPlayerModule));
    console.log(VTPlayerModule._AllocateAudioBuffer);

    add    = VTPlayerModule._add;
    sub    = VTPlayerModule._sub;
    memory = VTPlayerModule.asm.memory;

    AllocateAudioBuffer = VTPlayerModule._AllocateAudioBuffer;
    FreeAudioBuffer     = VTPlayerModule._FreeAudioBuffer    ;
    VTPlayerGetAudio    = VTPlayerModule._VTPlayerGetAudio   ;

    const bufferSizeInFloats = 128;
    const bufferPtr          = AllocateAudioBuffer(bufferSizeInFloats);
    const buffer             = new Float32Array(memory.buffer, bufferPtr, bufferSizeInFloats);

    console.log(buffer);

    const filled = VTPlayerGetAudio(bufferPtr, bufferSizeInFloats);

    console.log(buffer);

    postMessage(0);
};
