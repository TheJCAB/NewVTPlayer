
const RingBufferHead     = 0;
const RingBufferTail     = 1;
const WorkerMessageCount = 2;
const MetaBufferSize     = 3;

export function initializeSharedBuffer(ringSizeInFloats)
{
    const metaPosition = ringSizeInFloats * 4;

    const buffer = new SharedArrayBuffer(metaPosition + MetaBufferSize * 4);

    const meta = new Int32Array(buffer, metaPosition, MetaBufferSize);

    meta[RingBufferHead    ] = 0;    // Worker-controlled head
    meta[RingBufferTail    ] = 0;    // Processor-controlled tail
    meta[WorkerMessageCount] = 0;    // Posted message count

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

    postingWorkerMessage()
    {
        return Atomics.add(this._meta, WorkerMessageCount, 1);
    }

    handledWorkerMessage()
    {
        return Atomics.sub(this._meta, WorkerMessageCount, 1);
    }

    areWorkerMessagesPending()
    {
        return Atomics.load(this._meta, WorkerMessageCount) > 0;
    }

    get RingSizeInFloats()
    {
        return this._ringBuffer.length;
    }

    getRingBufferSnapshot()
    {
        return new RingBufferSnapshot(this);
    }

    get ringBufferHead()
    { 
        return Atomics.load(this._meta, RingBufferHead);
    }

    set ringBufferHead(newHead)
    { 
        return Atomics.store(this._meta, RingBufferHead, newHead) % this.RingSizeInFloats;
    }

    get ringBufferTail()
    { 
        return Atomics.load(this._meta, RingBufferTail);
    }

    set ringBufferTail(newTail)
    { 
        return Atomics.store(this._meta, RingBufferTail, newTail) % this.RingSizeInFloats;
    }

    notifyRingBufferTail()
    { 
        return Atomics.notify(this._meta, RingBufferTail, 1);
    }

    waitRingBufferTail(head)
    { 
        return Atomics.wait(this._meta, RingBufferTail, head);
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
