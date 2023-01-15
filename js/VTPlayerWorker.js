
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

var sharedCommunicationBuffer;
//var ringBuffer;
//var indices;
//var ringSizeInFloats;

var bufferSizeInFloats;
var bufferPtr         ;
var vtPlayerBuffer    ;

var lastPositionReported = {};

function mainLoop()
{
    console.log("Main loop");
    while (!sharedCommunicationBuffer.areWorkerMessagesPending())
    {
        var ring = sharedCommunicationBuffer.getRingBufferSnapshot();

        const available = ring.ringBufferAvailable;
        if (available <= 0)
        {
            sharedCommunicationBuffer.waitRingBufferTail();
        }
        else
        {
            const sampleCount = VTPlayerGetAudio(bufferPtr, Math.min(bufferSizeInFloats, available), metadataBufferPtr);

            //console.log(
            //    metadataBuffer[0], ' ', // position
            //    metadataBuffer[1], ' ', // line
            //    metadataBuffer[2], ' ', // pattern
            //    metadataBuffer[3], ' ', // tick
            //    metadataBuffer[4], ' ', // totalTicks
            //    metadataBuffer[5], ' ', // millisecond
            //    metadataBuffer[6]       // totalMilliseconds
            //);

            if (sampleCount == 0)
            {
                postMessage(
                    {
                        stop: true,
                    }
                );
                console.log("Song ended");
                break;
            }

            ring.fillFrom(vtPlayerBuffer.subarray(0, sampleCount));
            ring.commitFilled()

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


async function processMessage(event)
{
    console.log('Process message');

    if ('sharedCommunicationBuffer' in event.data)
    {
        const SharedCommunicationModule = await import("./SharedCommunicationBuffer.js");

        sharedCommunicationBuffer = sharedCommunicationBuffer = new SharedCommunicationModule.SharedCommunication(event.data.sharedCommunicationBuffer);
        sampleRate = event.data.sampleRate;

        const { default : createVTPlayer } = await import('/js/VTPlayer.js');
        console.log(createVTPlayer);

        VTPlayerModule = await createVTPlayer();
        // Unfortunately, Emscriptem replaces our onmessage handler, so we need to put it back.
        // We don't want the main thread to be calling C++ functions directly,
        // unless we at some point switch the C++ code itself to use a shared buffer.
        // TODO: Explore Emscriptem's support for pthreads and shared buffers for the heap.
        onmessage = processMessage;

        console.log(VTPlayerModule);
        console.log(Object.keys(VTPlayerModule));
        console.log(VTPlayerModule._AllocateAudioBuffer);

        malloc                      = VTPlayerModule._malloc                    ;
        free                        = VTPlayerModule._free                      ;
        AllocateAudioBuffer         = VTPlayerModule._AllocateAudioBuffer       ;
        FreeAudioBuffer             = VTPlayerModule._FreeAudioBuffer           ;
        VTPlayerLoadSongFromMemory  = VTPlayerModule._VTPlayerLoadSongFromMemory;
        VTPlayerGetAudio            = VTPlayerModule._VTPlayerGetAudio          ;

        bufferSizeInFloats = sharedCommunicationBuffer.RingSizeInFloats / 4;
        bufferPtr          = AllocateAudioBuffer(bufferSizeInFloats);
        vtPlayerBuffer     = new Float32Array(VTPlayerModule.HEAP8.buffer, bufferPtr, bufferSizeInFloats);

        metadataBufferPtr  = malloc(7 * 4);
        metadataBuffer     = new Uint32Array(VTPlayerModule.HEAP8.buffer, metadataBufferPtr, 7);

        postMessage(0);
    }
    if ('modBuffer' in event.data)
    {
        // TODO: Instead of putting the file in a WASM memory buffer,
        // we should look into putting it in the WASM filesystem.
        // That'd save some precious WASM memory.
        const buffer         = event.data.modBuffer;
        const modSizeInBytes = buffer.byteLength;
        const modPtr         = malloc(modSizeInBytes);
        (new Uint8Array(VTPlayerModule.HEAP8.buffer, modPtr, modSizeInBytes)).set(new Uint8Array(buffer));
        VTPlayerLoadSongFromMemory(modPtr, modSizeInBytes, sampleRate);
        free(modPtr);

        const oldCount = sharedCommunicationBuffer.handledWorkerMessage();
        console.log('Worker handlerd message. Old count was ', oldCount);
        mainLoop();
    }
};

var onmessage = processMessage;