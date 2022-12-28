
window.startAudio = async (audioContext) => {
    // await audioContext.audioWorklet.addModule('AudioWorklet0.js');
    await audioContext.audioWorklet.addModule('/js/AudioWorklet1.js');

    const worker = new Worker('/js/AudioWorker1.js');

    worker.onmessage = (buffers) => { console.log('Worker is live'); };

    worker.postMessage(0);

    //let node = new AudioWorkletNode(audioContext, 'AudioWorklet1');
    //node.connect(audioContext.destination);
};
