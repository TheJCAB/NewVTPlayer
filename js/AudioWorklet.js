import {
    SharedCommunication,
    OutputState,
} from "./SharedCommunicationBuffer.js"


class VTPlayerAudioWorkletProcessor extends AudioWorkletProcessor {
    constructor(nodeOptions) {
        super();

        this._isInitialized = false;
        this.port.onmessage = this._initialize.bind(this);
    }

    // Initializes upon the event from the worker backend.
    async _initialize(initData) {
        this._sharedCommunication = new SharedCommunication(initData.data.sharedCommunicationBuffer);

        this._isInitialized = true;
        //this.port.postMessage({});
    }

    process(inputs, outputs) {
        if (!this._isInitialized) {
            return true;
        }

        const state = this._sharedCommunication.outputState;
        switch (state)
        {
            case OutputState.Stopped      : return true;
            case OutputState.Playing      : break;
            case OutputState.StopRequested:
                this._sharedCommunication.completeStopOutput();
                return true;
        }

        // For now this is only mono channel.
        const outputChannelData = outputs[0][0];

        var ring = this._sharedCommunication.getRingBufferSnapshot();

        const filled = ring.ringBufferFilled;
        if (filled < outputChannelData.length)
        {
            // We don't have enough data, so silence it is.
            // TODO: to avoid jitter, we should now wait until the buffer is full before we start draining it again.
            // One longer pause is (arguably) better than many short ones.
            return true;
        }

        ring.emptyTo(outputChannelData);
        ring.commitEmptied();

        return true;
    }
}

registerProcessor('VTPlayerAudioWorklet', VTPlayerAudioWorkletProcessor);