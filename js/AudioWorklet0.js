
class MyWorkletProcessor extends AudioWorkletProcessor {

    constructor() {
        super();
    }

    process(inputs, outputs, parameters) {
        // Use the 1st input and output only to make the example simpler. |input|
        // and |output| here have the similar structure with the AudioBuffer
        // interface. (i.e. An array of Float32Array)
        const input  = inputs[0];
        const output = outputs[0];

        // For this given render quantum, the channel count of the node is fixed
        // and identical for the input and the output.
        const channelCount = output.length;
        for (let channel = 0; channel < channelCount; ++channel) {
            for (let frame = 0; frame < 128; ++frame) {
                output[channel][frame] = ((frame & 15) - 7.5) / 7.5;
            }
        }
        return true;
    }
}

registerProcessor('AudioWorklet0', MyWorkletProcessor);