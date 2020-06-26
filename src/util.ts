export function formatBytes (num, length) {
    var hex = num.toString(16).toUpperCase();
    if (hex.length < length * 2) {
        hex = "0".repeat(length * 2 - hex.length) + hex;
    }
    if (hex.length === 2) return hex;
    return hex.match(/.{2}/g).join(' ');
}
