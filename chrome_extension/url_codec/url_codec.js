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
