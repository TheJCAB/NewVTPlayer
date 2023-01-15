
var VTPlayerModule;

var malloc;
var free;

var AllocateAudioBuffer;
var FreeAudioBuffer;
var VTPlayerLoadSongFromMemory;
var VTPlayerGetSongData;
var VTPlayerGetAudio;

var metadataBufferPtr;
var metadataBuffer;

var sharedBuffer;
var sampleRate;

var sharedCommunicationBuffer;
var positionQueue = new Array();

var bufferSizeInFloats;
var bufferPtr         ;
var vtPlayerBuffer    ;

var lastPositionReported = {};

var songTotalMilliseconds;

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
            //    metadataBuffer[3], ' ', // millisecond
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

            const head = ring.head;

            const pos =
            {
                head:         head,
                position:     metadataBuffer[0],
                line:         metadataBuffer[1],
                pattern:      metadataBuffer[2],
                seconds:      Math.trunc(metadataBuffer[3] / 1000),
                totalSeconds: Math.trunc(songTotalMilliseconds / 1000),
                percent:      metadataBuffer[3] * 100.0 / songTotalMilliseconds,
            };

            if (positionQueue.length > 0)
            {
                var prev = positionQueue.at(-1);
                if (prev.position == pos.position &&
                    prev.line     == pos.line     &&
                    prev.seconds  == pos.seconds)
                {
                    prev.head = pos.head;
                    // And we're done.
                }
                else
                {
                    positionQueue.push(pos);
                }
            }
            else
            {
                positionQueue.push(pos);
            }

            var i = 0;
            while (i + 1 < positionQueue.length && !ring.isIndexFilled(positionQueue[i].head))
            {
                ++i;
            }
            if (i > 0)
            {
                positionQueue.splice(0, i);
            }

            const positionReported =
            {
                position: positionQueue[0].position,
                line:     positionQueue[0].line,
                seconds:  positionQueue[0].seconds,
            };

            if (positionReported.position != lastPositionReported.position ||
                positionReported.line     != lastPositionReported.line     ||
                positionReported.seconds  != lastPositionReported.seconds)
            {
                postMessage(
                    {
                        position:     positionQueue[0].position,
                        line:         positionQueue[0].line,
                        pattern:      positionQueue[0].pattern,
                        seconds:      positionQueue[0].seconds,
                        totalSeconds: positionQueue[0].totalSeconds,
                        percent:      positionQueue[0].percent,
                    }
                );
                lastPositionReported = positionReported;
            }

            ring.fillFrom(vtPlayerBuffer.subarray(0, sampleCount));
            ring.commitFilled()
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
        VTPlayerGetSongData         = VTPlayerModule._VTPlayerGetSongData       ;
        VTPlayerGetAudio            = VTPlayerModule._VTPlayerGetAudio          ;

        bufferSizeInFloats = Math.min(sampleRate / 50, sharedCommunicationBuffer.RingSizeInFloats);
        bufferPtr          = AllocateAudioBuffer(bufferSizeInFloats);
        vtPlayerBuffer     = new Float32Array(VTPlayerModule.HEAP8.buffer, bufferPtr, bufferSizeInFloats);

        metadataBufferPtr  = malloc(7 * 4);
        metadataBuffer     = new Uint32Array(VTPlayerModule.HEAP8.buffer, metadataBufferPtr, 4);

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

        const pSongData = VTPlayerGetSongData();
        songTotalMilliseconds = VTPlayerModule.HEAPU32[pSongData / 4 + 1];

        const oldCount = sharedCommunicationBuffer.handledWorkerMessage();
        console.log('Worker handlerd message. Old count was ', oldCount);
        mainLoop();
    }
};

var onmessage = processMessage;