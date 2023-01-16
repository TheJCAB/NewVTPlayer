
const MetaSlot = Object.freeze({
    RingBufferHead    : 0,
    RingBufferTail    : 1,
    OutputState       : 2,
});
const MetaBufferSize  = 3;

export const OutputState = Object.freeze({
    Stopped      : 0,
    Playing      : 1,
    StopRequested: 2,
});

export function initializeSharedBuffer(ringSizeInFloats)
{
    const metaPosition = ringSizeInFloats * 4;

    const buffer = new SharedArrayBuffer(metaPosition + MetaBufferSize * 4);

    const meta = new Int32Array(buffer, metaPosition, MetaBufferSize);

    meta[MetaSlot.RingBufferHead    ] = 0;    // Worker-controlled head
    meta[MetaSlot.RingBufferTail    ] = 0;    // Processor-controlled tail
    meta[MetaSlot.OutputState       ] = OutputState.Stopped;

    return {
        buffer:           buffer,
        ringSizeInFloats: ringSizeInFloats,
        metaPosition:     metaPosition,
    };
}

export class SharedCommunication
{
    constructor(bufferData)
    {
        this._bufferData = bufferData;
        this._ringBuffer = new Float32Array(this._bufferData.buffer, 0, this._bufferData.ringSizeInFloats);
        this._meta       = new Int32Array(this._bufferData.buffer, this._bufferData.metaPosition, MetaBufferSize);
    }

    _waitAsyncSlot(metaSlot, expected)
    {
        const result = Atomics.waitAsync(this._meta, metaSlot, expected);
        if (result.async)
        {
            return result.value;
        }
        else
        {
            return Promise.resolve(result.value);
        }
    }

    get RingSizeInFloats() { return this._ringBuffer.length; }

    getRingBufferSnapshot() { return new RingBufferSnapshot(this); }

    get ringBufferHead()        { return Atomics.load (this._meta, MetaSlot.RingBufferHead); }
    set ringBufferHead(newHead) { return Atomics.store(this._meta, MetaSlot.RingBufferHead, newHead % this.RingSizeInFloats); }

    get ringBufferTail()        { return Atomics.load (this._meta, MetaSlot.RingBufferTail); }
    set ringBufferTail(newTail) { return Atomics.store(this._meta, MetaSlot.RingBufferTail, newTail % this.RingSizeInFloats); }

    notifyRingBufferTail()     { return Atomics.notify(this._meta, MetaSlot.RingBufferTail, 1); }
    waitRingBufferTail  (tail) { return this._waitAsyncSlot(MetaSlot.RingBufferTail, tail); }

    get outputState() { return Atomics.load(this._meta, MetaSlot.OutputState); }
    async requestStopOutput()
    {
        var state = Atomics.load(this._meta, MetaSlot.OutputState);
        switch (state)
        {
            case OutputState.Stopped:
                return;
            case OutputState.Playing:
                state = Atomics.compareExchange(this._meta, MetaSlot.OutputState, OutputState.Playing, OutputState.StopRequested);
                if (state == OutputState.Playing)
                {
                    state = OutputState.StopRequested;
                    Atomics.notify(this._meta, MetaSlot.OutputState, 1);
                }
                break;
            case OutputState.StopRequested:
                break;
        }
        await this._waitAsyncSlot(MetaSlot.OutputState, OutputState.StopRequested);
    }
    async completeStopOutput()
    {
        Atomics.store (this._meta, MetaSlot.OutputState, OutputState.Stopped);
        Atomics.notify(this._meta, MetaSlot.OutputState, 1);
    }
    async requestStartOutput()
    {
        Atomics.store (this._meta, MetaSlot.OutputState, OutputState.Playing);
        Atomics.notify(this._meta, MetaSlot.OutputState, 1);
    }
}

export class RingBufferSnapshot
{
    constructor(sharedCommunication)
    {
        this.head                 = sharedCommunication.ringBufferHead;
        this.tail                 = sharedCommunication.ringBufferTail;
        this.buffer               = sharedCommunication._ringBuffer;
        this.RingSizeInFloats     = sharedCommunication.RingSizeInFloats;
        this._sharedCommunication = sharedCommunication;
    }

    get ringBufferFilled()
    {
        return (this.head + this.RingSizeInFloats - this.tail) % (this.RingSizeInFloats);
    }

    get ringBufferAvailable()
    {
        return this.RingSizeInFloats - this.ringBufferFilled - 1;
    }

    isIndexFilled(index)
    {
        return (index + this.RingSizeInFloats - this.tail) % (this.RingSizeInFloats) <= this.ringBufferFilled;
    }

    fillFrom(samples)
    {
        const newHead = this.head + samples.length;
        if (newHead <= this.RingSizeInFloats)
        {
            this.buffer.set(samples, this.head);
        }
        else
        {
            const firstSize = this.RingSizeInFloats - this.head;
            this.buffer.set(samples.subarray(0, firstSize), this.head);
            this.buffer.set(samples.subarray(firstSize, samples.length));
        }
        this.head = newHead % this.RingSizeInFloats;
    }

    emptyTo(samples)
    {
        const newTail = this.tail + samples.length;
        if (newTail <= this.RingSizeInFloats)
        {
            samples.set(this.buffer.subarray(this.tail, newTail));
        }
        else
        {
            const firstSize = this.RingSizeInFloats - this.tail;
            samples.set(this.buffer.subarray(this.tail, this.tail + firstSize));
            samples.set(this.buffer.subarray(0, samples.length - firstSize), firstSize);
        }
        this.tail = newTail % this.RingSizeInFloats;
    }

    commitFilled()
    {
        this._sharedCommunication.ringBufferHead = this.head;
    }

    commitEmptied()
    {
        this._sharedCommunication.ringBufferTail = this.tail;
        this._sharedCommunication.notifyRingBufferTail();
    }
}
