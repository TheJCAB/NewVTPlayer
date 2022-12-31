
import { RingBufferSize } from "./AudioConstants.js"

class VTPlayerAudioWorkletProcessor extends AudioWorkletProcessor {
    constructor(nodeOptions) {
        super();

        this._isInitialized = false;
        this.port.onmessage = this._initialize.bind(this);
    }

    // Initializes upon the event from the worker backend.
    _initialize(initData) {
        const sharedBuffer = initData.data;

        this._indices = new Int32Array(sharedBuffer, RingBufferSize * 4, 2);

        // Worker's output buffer, mono. TODO: Stereo.
        this._ringBuffer = new Float32Array(sharedBuffer, 0, RingBufferSize);

        this._isInitialized = true;
        //this.port.postMessage({});
    }

    process(inputs, outputs) {
        if (!this._isInitialized) {
            return true;
        }

        // For now this is only mono channel.
        const outputChannelData = outputs[0][0];

        const head = this._indices[0];
        var   tail = this._indices[1];

        const available = (head + RingBufferSize - tail) % (RingBufferSize);
        if (available < 128)
        {
            // We don't have enough data, so silence it is.
            // TODO: to avoid jitter, we should now wait until the buffer is full before we start draining it again.
            // One longer pause is (arguably) better than many short ones.
            return true;
        }

        if (tail + 128 <= RingBufferSize)
        {
            outputChannelData.set(this._ringBuffer.subarray(tail, tail + 128));
        }
        else
        {
            const firstSize = RingBufferSize - tail;
            outputChannelData.set(this._ringBuffer.subarray(tail, tail + firstSize));
            outputChannelData.set(this._ringBuffer.subarray(0, 128 - firstSize), firstSize);
        }

        Atomics.store(this._indices, 1, (tail + 128) % (RingBufferSize));
        Atomics.notify(this._indices, 1);

        return true;
    }
}

registerProcessor('VTPlayerAudioWorklet', VTPlayerAudioWorkletProcessor);