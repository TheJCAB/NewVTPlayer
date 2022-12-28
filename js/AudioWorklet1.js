
// Description of shared states. See shared-buffer-worker.js for the
// description.
const STATE = {
    'REQUEST_RENDER': 0,
    'IB_FRAMES_AVAILABLE': 1,
    'IB_READ_INDEX': 2,
    'IB_WRITE_INDEX': 3,
    'OB_FRAMES_AVAILABLE': 4,
    'OB_READ_INDEX': 5,
    'OB_WRITE_INDEX': 6,
    'RING_BUFFER_LENGTH': 7,
    'KERNEL_LENGTH': 8,
};

class AudioWorklet1Processor extends AudioWorkletProcessor {
    constructor(nodeOptions) {
        super();

        this._isInitialized = false;
        this.port.onmessage = this._initialize.bind(this);
    }

    // Initializes upon the event from the worker backend.
    _initialize(eventFromWorker) {
        const sharedBuffers = eventFromWorker.data;

        // Get the states buffer.
        this._states = new Int32Array(sharedBuffers.states);

        // Worker's output buffer, mono. TODO: Stereo.
        this._outputRingBuffer = [new Float32Array(sharedBuffers.outputRingBuffer)];

        this._ringBufferLength = this._states[STATE.RING_BUFFER_LENGTH];
        this._kernelLength     = this._states[STATE.KERNEL_LENGTH];

        this._isInitialized = true;
        this.port.postMessage({message: 'PROCESSOR_READY'});
    }

    // Pull the data out of the shared input buffer to fill |outputChannelData| (128-frames).
    _pullOutputChannelData(outputChannelData) {
        const outputReadIndex = this._states[STATE.OB_READ_INDEX];
        const nextReadIndex = outputReadIndex + outputChannelData.length;

        if (nextReadIndex < this._ringBufferLength) {
            outputChannelData.set(
                this._outputRingBuffer[0].subarray(outputReadIndex, nextReadIndex));
            this._states[STATE.OB_READ_INDEX] += outputChannelData.length;
        } else {
            const overflow = nextReadIndex - this._ringBufferLength;
            const firstHalf = this._outputRingBuffer[0].subarray(outputReadIndex);
            const secondHalf = this._outputRingBuffer[0].subarray(0, overflow);
            outputChannelData.set(firstHalf);
            outputChannelData.set(secondHalf, firstHalf.length);
            this._states[STATE.OB_READ_INDEX] = secondHalf.length;
        }
    }

    process(inputs, outputs) {
        if (!this._isInitialized) {
            return true;
        }

        // This example only handles mono channel.
        const outputChannelData = outputs[0][0];

        this._pullOutputChannelData(outputChannelData);

        if (this._states[STATE.IB_FRAMES_AVAILABLE] >= this._kernelLength) {
            // Now we have enough frames to process. Wake up the worker.
            Atomics.notify(this._states, STATE.REQUEST_RENDER, 1);
        }

        return true;
    }
}

registerProcessor('AudioWorklet1', AudioWorklet1Processor);