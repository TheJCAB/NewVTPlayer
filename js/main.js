
var vtPlayerWorker;

var indices;

window.startAudio = async (audioContext) => {

    const audioConstants = await import("./AudioConstants.js");

    const RingBufferSize = audioConstants.RingBufferSize;

    console.log(crossOriginIsolated);

    const buffer = new SharedArrayBuffer(RingBufferSize * 4 + 3 * 4);

    indices = new Int32Array(buffer, RingBufferSize * 4, 3);
    indices[0] = 0;    // Worker-controlled head
    indices[1] = 0;    // Processor-controlled tail
    indices[2] = 0;    // Posted message count

    await audioContext.audioWorklet.addModule('/js/AudioWorklet.js');

    let node = new AudioWorkletNode(audioContext, 'VTPlayerAudioWorklet');
    node.port.postMessage(buffer);
    node.connect(audioContext.destination);

    vtPlayerWorker = new Worker('/js/VTPlayerWorker.js');
    vtPlayerWorker.postMessage(buffer);
};

window.startNextAudio = async () => {

    var response = await fetch("/MODs/MOD/8CHN.MOD");
    var buffer   = await response.arrayBuffer();

    vtPlayerWorker.postMessage(buffer);
    const oldCount = Atomics.add(indices, 2, 1);
    //console.log('Count increased to ', oldCount + 1);
};
