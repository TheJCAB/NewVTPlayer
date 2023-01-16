
// Emscriptem module.
var VTPlayerModule;

// C++ functions that we call.
var malloc;
var free;
var AllocateAudioBuffer;
var FreeAudioBuffer;
var VTPlayerLoadSongFromMemory;
var VTPlayerGetSongData;
var VTPlayerGetAudio;

// C++ utility buffer to get metadata about the played song.
var metadataBufferPtr;
var metadataBuffer;

// Sampling rate (samples per second, typically 48000) specified by the audio output.
var sampleRate;

// All threads are sharing a buffer to communicate with each other.
// sharedCommunicationBuffer is the object we use to access this communication.
var sharedCommunicationBuffer;

// Simple queue of position data for samples in the ring buffer.
// This allows us to have a long buffer, while still showing data to the user
// that corresponds to the actual audio output.
var positionQueue = new Array();

// Position data kept around to decide when to add a new entry to the position queue.
var lastPositionReported = {};

// C++ buffer used to request samples from the player.
var bufferSizeInFloats;
var bufferPtr         ;
var vtPlayerBuffer    ;

// Total lengh of the song.
var songTotalMilliseconds;

async function mainLoop()
{
    console.log("Main loop");
    var songEnded = true;
    while (true)
    {
        var ring = sharedCommunicationBuffer.getRingBufferSnapshot();

        var available = ring.ringBufferAvailable;
        while (available <= sampleRate / 100)
        {
            await sharedCommunicationBuffer.waitRingBufferTail(ring.tail);
            ring = sharedCommunicationBuffer.getRingBufferSnapshot();
            available = ring.ringBufferAvailable;
        }

        const sampleCount = VTPlayerGetAudio(bufferPtr, Math.min(bufferSizeInFloats, available), metadataBufferPtr);

        //console.log(
        //    metadataBuffer[0], ' ', // position
        //    metadataBuffer[1], ' ', // line
        //    metadataBuffer[2], ' ', // pattern
        //    metadataBuffer[3], ' ', // millisecond
        //);

        if (sampleCount == 0)
        {
            if (!songEnded)
            {
                postMessage(
                    {
                        songEnded: true,
                    }
                );
                console.log("Song ended");
                songEnded = true;
            }
            await new Promise(resolve => { setTimeout(resolve, 10); });
            continue;
            //break;
        }

        songEnded = false;

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
        //await new Promise(resolve => { setTimeout(resolve, 0); });
    }
    //console.log("Main loop exit");
}

var onmessage = async (event) =>
{
    const SharedCommunicationModule = await import("./SharedCommunicationBuffer.js");

    sharedCommunicationBuffer = sharedCommunicationBuffer = new SharedCommunicationModule.SharedCommunication(event.data.sharedCommunicationBuffer);
    sampleRate = event.data.sampleRate;

    const { default : createVTPlayer } = await import('/js/VTPlayer.js');
    console.log(createVTPlayer);

    VTPlayerModule = await createVTPlayer();
    // Unfortunately, Emscriptem replaces our onmessage handler with a new one
    // that allows the main thread to call C++ functions directly.
    // Fortunately we don't care. This handler is single-shot initialization, so we just need
    // to make sure we install our handler after the Emscriptem initialization above.
    // We don't want the main thread to be calling C++ functions directly,
    // unless we at some point switch the C++ code itself to use a shared buffer,
    // which  might eliminate the need for any of this goop.
    // TODO: Explore Emscriptem's support for pthreads and shared buffer heap.
    onmessage = processMessage;

    console.log(VTPlayerModule);
    console.log(Object.keys(VTPlayerModule));
    console.log(VTPlayerModule._AllocateAudioBuffer);

    // C++ functions that we call.
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

    await mainLoop();
}

async function processMessage(event)
{
    console.log('Process message');

    if ('modUrl' in event.data)
    {
        const response = await fetch(event.data.modUrl);
        console.log('file fetched');
        const buffer   = await response.arrayBuffer();
        console.log('buffer obtained');

        // TODO: Instead of putting the file in a WASM memory buffer,
        // we should look into putting it in the WASM filesystem.
        // That'd save some precious WASM memory.
        const modSizeInBytes = buffer.byteLength;
        const modPtr         = malloc(modSizeInBytes);
        (new Uint8Array(VTPlayerModule.HEAP8.buffer, modPtr, modSizeInBytes)).set(new Uint8Array(buffer));
        console.log('buffer copied');
        VTPlayerLoadSongFromMemory(modPtr, modSizeInBytes, sampleRate);
        console.log('song loaded');
        free(modPtr);

        const pSongData = VTPlayerGetSongData();
        songTotalMilliseconds = VTPlayerModule.HEAPU32[pSongData / 4 + 1];
    }
};
