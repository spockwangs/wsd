function decode()
{
    var obj = document.getElementById('url');
    obj.value = decodeURIComponent(obj.value);
}

function encode()
{
    var obj = document.getElementById('url');
    obj.value = encodeURIComponent(obj.value);
}

document.getElementById('decode').onclick = decode;
document.getElementById('encode').onclick = encode;
