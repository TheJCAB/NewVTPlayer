
var memory;

var AllocateAudioBuffer;
var FreeAudioBuffer;
var VTPlayerGetAudio;

var sharedBuffer;
var ringBuffer;
var indices;

//calc.Module.onRuntimeInitialized = _ => {
//    console.log(calc.Module);
//}

onmessage = async (event) => {

    const audioConstants = await import("./AudioConstants.js");

    const RingBufferSize = audioConstants.RingBufferSize;

    sharedBuffer = event.data;
    ringBuffer = new Float32Array(sharedBuffer, 0, RingBufferSize);
    indices = new Int32Array(sharedBuffer, RingBufferSize * 4, 2);

    const { default : createVTPlayer } = await import('/js/VTPlayer.js');

    console.log(createVTPlayer);

    const VTPlayerModule = await createVTPlayer();

    console.log(VTPlayerModule);
    console.log(Object.keys(VTPlayerModule));
    console.log(VTPlayerModule._AllocateAudioBuffer);

    memory = VTPlayerModule.asm.memory;

    malloc                      = VTPlayerModule._malloc                    ;
    free                        = VTPlayerModule._free                      ;
    AllocateAudioBuffer         = VTPlayerModule._AllocateAudioBuffer       ;
    FreeAudioBuffer             = VTPlayerModule._FreeAudioBuffer           ;
    VTPlayerLoadSongFromMemory  = VTPlayerModule._VTPlayerLoadSongFromMemory;
    VTPlayerGetAudio            = VTPlayerModule._VTPlayerGetAudio          ;

    const bufferSizeInFloats = 128; //RingBufferSize / 4;
    const bufferPtr          = AllocateAudioBuffer(bufferSizeInFloats);
    const vtPlayerBuffer     = new Float32Array(memory.buffer, bufferPtr, bufferSizeInFloats);

    {
        var response = await fetch("/MODs/MOD/CONDOM.MOD");
        var buffer = await response.arrayBuffer();
        const modSizeInBytes = buffer.byteLength;
        const modPtr         = malloc(modSizeInBytes);
        (new Uint8Array(memory.buffer, modPtr, modSizeInBytes)).set(new Uint8Array(buffer));
        VTPlayerLoadSongFromMemory(modPtr, modSizeInBytes);
        free(modPtr);
    }

    postMessage(0);

    while (true)
    {
        var   head = indices[0];
        const tail = indices[1];

        const available = (tail + RingBufferSize - 1 - head) % (RingBufferSize);
        if (available < bufferSizeInFloats)
        {
            Atomics.wait(indices, 1, tail);
        }
        else
        {
            const filled = VTPlayerGetAudio(bufferPtr, bufferSizeInFloats);

            if (head + filled <= RingBufferSize)
            {
                ringBuffer.set(vtPlayerBuffer.subarray(0, filled), head);
                Atomics.store(indices, 0, head + filled);
            }
            else
            {
                const firstSize = RingBufferSize - head;
                ringBuffer.set(vtPlayerBuffer.subarray(0, firstSize), head);
                ringBuffer.set(vtPlayerBuffer.subarray(firstSize, filled));
                Atomics.store(indices, 0, filled - firstSize);
            }
        }
    }
};
