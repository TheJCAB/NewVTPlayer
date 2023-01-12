
var VTPlayerModule;

var malloc;
var free;

var AllocateAudioBuffer;
var FreeAudioBuffer;
var VTPlayerLoadSongFromMemory;
var VTPlayerGetAudio;

var metadataBufferPtr;
var metadataBuffer;

var sharedBuffer;
var sampleRate;
var ringBuffer;
var indices;

var RingBufferSize;

var bufferSizeInFloats;
var bufferPtr         ;
var vtPlayerBuffer    ;

var lastPositionReported = {};

function mainLoop()
{
    console.log("Main loop");
    while (Atomics.load(indices, 2) == 0)
    {
        const head = indices[0];
        const tail = indices[1];

        const available = (tail + RingBufferSize - 1 - head) % RingBufferSize;
        if (available <= 0)
        {
            Atomics.wait(indices, 1, tail);
        }
        else
        {
            const filled = VTPlayerGetAudio(bufferPtr, Math.min(bufferSizeInFloats, available), metadataBufferPtr);

            //console.log(
            //    metadataBuffer[0], ' ', // position
            //    metadataBuffer[1], ' ', // line
            //    metadataBuffer[2], ' ', // pattern
            //    metadataBuffer[3], ' ', // tick
            //    metadataBuffer[4], ' ', // totalTicks
            //    metadataBuffer[5], ' ', // millisecond
            //    metadataBuffer[6]       // totalMilliseconds
            //);

            if (filled == 0)
            {
                postMessage(
                    {
                        stop: true,
                    }
                );
                console.log("Song ended");
                break;
            }

            if (head + filled <= RingBufferSize)
            {
                ringBuffer.set(vtPlayerBuffer.subarray(0, filled), head);
            }
            else
            {
                const firstSize = RingBufferSize - head;
                ringBuffer.set(vtPlayerBuffer.subarray(0, firstSize), head);
                ringBuffer.set(vtPlayerBuffer.subarray(firstSize, filled));
            }

            Atomics.store(indices, 0, (head + filled) % RingBufferSize);

            const positionReported =
            {
                position: metadataBuffer[0],
                line:     metadataBuffer[1],
            };

            if (positionReported.position != lastPositionReported.position ||
                positionReported.line     != lastPositionReported.line)
            {
                postMessage(
                    {
                        position:     metadataBuffer[0],
                        line:         metadataBuffer[1],
                        pattern:      metadataBuffer[2],
                        seconds:      metadataBuffer[5] / 1000.0,
                        totalSeconds: metadataBuffer[6] / 1000.0,
                        percent:      metadataBuffer[5] * 100.0 / metadataBuffer[6],
                    }
                );
                lastPositionReported = positionReported;
            }
        }
    }
    console.log("Main loop exit");
}


function processMessage(event)
{
    console.log('Process message');

    // TODO: Instead of putting the file in a WASM memory buffer,
    // we should look into putting it in the WASM filesystem.
    // That'd save some precious WASM memory.
    const buffer         = event.data;
    const modSizeInBytes = buffer.byteLength;
    const modPtr         = malloc(modSizeInBytes);
    (new Uint8Array(VTPlayerModule.HEAP8.buffer, modPtr, modSizeInBytes)).set(new Uint8Array(buffer));
    VTPlayerLoadSongFromMemory(modPtr, modSizeInBytes, sampleRate);
    free(modPtr);

    const oldCount = Atomics.sub(indices, 2, 1);
    console.log('count was ', oldCount);
    mainLoop();
}

var onmessage = async (event) => {

    const audioConstants = await import("./AudioConstants.js");

    RingBufferSize = audioConstants.RingBufferSize;

    sharedBuffer = event.data.buffer;
    sampleRate = event.data.sampleRate;
    ringBuffer = new Float32Array(sharedBuffer, 0, RingBufferSize);
    indices = new Int32Array(sharedBuffer, RingBufferSize * 4, 3);

    const { default : createVTPlayer } = await import('/js/VTPlayer.js');

    console.log(createVTPlayer);

    VTPlayerModule = await createVTPlayer();

    console.log(VTPlayerModule);
    console.log(Object.keys(VTPlayerModule));
    console.log(VTPlayerModule._AllocateAudioBuffer);

    malloc                      = VTPlayerModule._malloc                    ;
    free                        = VTPlayerModule._free                      ;
    AllocateAudioBuffer         = VTPlayerModule._AllocateAudioBuffer       ;
    FreeAudioBuffer             = VTPlayerModule._FreeAudioBuffer           ;
    VTPlayerLoadSongFromMemory  = VTPlayerModule._VTPlayerLoadSongFromMemory;
    VTPlayerGetAudio            = VTPlayerModule._VTPlayerGetAudio          ;

    bufferSizeInFloats = RingBufferSize / 4;
    bufferPtr          = AllocateAudioBuffer(bufferSizeInFloats);
    vtPlayerBuffer     = new Float32Array(VTPlayerModule.HEAP8.buffer, bufferPtr, bufferSizeInFloats);

    metadataBufferPtr  = malloc(7 * 4);
    metadataBuffer     = new Uint32Array(VTPlayerModule.HEAP8.buffer, metadataBufferPtr, 7);

    //{
    //    var response = await fetch("/MODs/S3M/ctgoblin.s3m");
    //    var buffer = await response.arrayBuffer();
    //    const modSizeInBytes = buffer.byteLength;
    //    const modPtr         = malloc(modSizeInBytes);
    //    (new Uint8Array(VTPlayerModule.HEAP8.buffer, modPtr, modSizeInBytes)).set(new Uint8Array(buffer));
    //    VTPlayerLoadSongFromMemory(modPtr, modSizeInBytes, sampleRate);
    //    free(modPtr);
    //}

    onmessage = processMessage;

    postMessage(0);

    //mainLoop();
};
