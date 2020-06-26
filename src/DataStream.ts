/* DataStream */

export class DataStream {
    static BIG_ENDIAN = false;
    static LITTLE_ENDIAN = true;

    position: number;

    constructor(public buffer: Buffer, public endianness:boolean = DataStream.BIG_ENDIAN) {
        this.position = 0;
    }

    seek(pos: number) {
        this.position = pos;
    }

    skip(n: number) {
        this.position = this.position + n;
    }

    endOfFile() {
        return this.position >= this.buffer.length;
    }

    readBytes(length: number) {
        let result = this.buffer.slice(this.position, this.position + length);
        this.position += length;
        return result;
    }

    readInt8() {
        let result = this.buffer.readInt8(this.position);
        this.position += 1;
        return result;
    }

    readUint8() {
        let result = this.buffer.readUInt8(this.position);
        this.position += 1;
        return result;
    }

    readInt16() {
        let result = this.endianness ? this.buffer.readInt16LE(this.position) : this.buffer.readInt16BE(this.position);
        this.position += 2;
        return result;
    }

    readUint16() {
        let result = this.endianness ? this.buffer.readUInt16LE(this.position) : this.buffer.readUInt16BE(this.position);
        this.position += 2;
        return result;
    }

    readUint24() {
        let result = this.endianness
            ? (this.buffer.readUInt8(this.position) | (this.buffer.readUInt8(this.position + 1) << 8) | (this.buffer.readUInt8(this.position + 2) << 16))
            : ((this.buffer.readUInt8(this.position) << 16) | (this.buffer.readUInt8(this.position + 1) << 8) | this.buffer.readUInt8(this.position + 2));
        this.position += 3;
        return result;
    }

    readInt32() {
        let result = this.endianness ? this.buffer.readInt32LE(this.position) : this.buffer.readInt32BE(this.position);
        this.position += 4;
        return result;
    }

    readUint32() {
        let result = this.endianness ? this.buffer.readUInt32LE(this.position) : this.buffer.readUInt32BE(this.position);
        this.position += 4;
        return result;
    }

    readFloat() {
        let result = this.endianness ? this.buffer.readFloatLE(this.position) : this.buffer.readFloatBE(this.position);
        this.position += 4;
        return result;
    }

    readDouble() {
        let result = this.endianness ? this.buffer.readDoubleLE(this.position) : this.buffer.readDoubleBE(this.position);
        this.position += 8;
        return result;
    }

    readString(length: number) {
        let result = this.buffer.toString("utf8", this.position, this.position + length);
        this.position += length;
        return result;
    }

    readFourCC() {
        let result = this.buffer.toString("utf8", this.position, this.position + 4);
        if (this.endianness) {
            result = result.split("").reverse().join("");
        }
        this.position += 4;
        return result;
    }
}
