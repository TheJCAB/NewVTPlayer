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

    sharedBuffer = event.data;
    ringBuffer = new Float32Array(sharedBuffer, 0, 128 * 8);
    indices = new Int32Array(sharedBuffer, 128 * 8 * 4, 2);

    const { default : createVTPlayer } = await import('/js/VTPlayer.js');

    console.log(createVTPlayer);

    const VTPlayerModule = await createVTPlayer();

    console.log(VTPlayerModule);
    console.log(Object.keys(VTPlayerModule));
    console.log(VTPlayerModule._AllocateAudioBuffer);

    memory = VTPlayerModule.asm.memory;

    AllocateAudioBuffer = VTPlayerModule._AllocateAudioBuffer;
    FreeAudioBuffer     = VTPlayerModule._FreeAudioBuffer    ;
    VTPlayerGetAudio    = VTPlayerModule._VTPlayerGetAudio   ;

    const bufferSizeInFloats = 128;
    const bufferPtr          = AllocateAudioBuffer(bufferSizeInFloats);
    const vtPlayerBuffer     = new Float32Array(memory.buffer, bufferPtr, bufferSizeInFloats);

    postMessage(0);

    while (true)
    {
        var   head = indices[0];
        const tail = indices[1];

        const available = (tail + 128 * 8 - 1 - head) % (128 * 8);
        if (available < bufferSizeInFloats)
        {
            Atomics.wait(indices, 1, tail);
        }
        else
        {
            const filled = VTPlayerGetAudio(bufferPtr, bufferSizeInFloats);

            if (head + filled <= 128 * 8)
            {
                ringBuffer.set(vtPlayerBuffer.subarray(0, filled), head);
                Atomics.store(indices, 0, head + filled);
            }
            else
            {
                const firstSize = 128 * 8 - head;
                ringBuffer.set(vtPlayerBuffer.subarray(0, firstSize), head);
                ringBuffer.set(vtPlayerBuffer.subarray(firstSize, filled));
                Atomics.store(indices, 0, filled - firstSize);
            }
        }
    }
};
