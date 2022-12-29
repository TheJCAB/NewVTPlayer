
window.startAudio = async (audioContext) => {

    console.log(crossOriginIsolated);

    const buffer = new SharedArrayBuffer(128 * 8 * 4 + 2 * 4);

    const indices = new Int32Array(buffer, 128 * 8 * 4, 2);
    indices[0] = 0;    // Worker-controlled head
    indices[1] = 0;    // Processor-controlled tail

    await audioContext.audioWorklet.addModule('/js/AudioWorklet.js');

    let node = new AudioWorkletNode(audioContext, 'VTPlayerAudioWorklet');
    node.port.postMessage(buffer);
    node.connect(audioContext.destination);

    const worker = new Worker('/js/VTPlayerWorker.js');
    worker.postMessage(buffer);
};
