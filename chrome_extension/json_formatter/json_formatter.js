// Copyright (c) 2013 Tencent, Inc.
//     All rights reserved.
//
// Author: spockwang@tencent.com
//

var indentSpaces = 2;

// Return the formatted json string.
function formatJson(jsonObj) {
    if (isArray(jsonObj))
        return formatArray(jsonObj, 0);
    return formatObj(jsonObj, 0);
}

function formatValue(value, depth, asPropertyValue) {
    if (isArray(value))
        return formatArray(value, depth, asPropertyValue);
    if (isObject(value))
        return formatObj(value, depth, asPropertyValue);
    return formatScalar(value);
}

function isArray(obj) {
    return obj
        && typeof obj === 'object'
        && typeof obj.length === 'number'
        && typeof obj.splice === 'function'
        && !(obj.propertyIsEnumerable('length'));
}

function isString(obj) {
    return typeof obj === 'string';
}

function isNumber(obj) {
    return typeof obj === 'number';
}

function isNull(obj) {
    return obj === null;
}

function isObject(obj) {
    return obj && typeof obj === 'object';
}

function indent(n) {
    var s = '';
    for (var i = 0; i < n; ++i)
        s += ' ';
    return s;
}

function formatArray(arr, depth, asPropertyValue) {
    var s, i;
    if (asPropertyValue)
        s = '[\n';
    else
        s = indent(depth) + '[\n';
    for (i = 0; i < arr.length; ++i)
        s += formatValue(arr[i], depth+indentSpaces) + (i == arr.length-1 ? '\n' : ',\n');
    s += indent(depth) + ']\n';
    return s;
}

function formatObj(obj, depth, asPropertyValue) {
    var s;
    if (asPropertyValue)
        s = '{\n';
    else
        s = indent(depth) + '{\n';

    var firstProperty = true;
    for (property in obj) {
        if (!firstProperty) {
            s += ',\n';
        } else {
            firstProperty = false;
        }
        s += indent(depth + indentSpaces) + '"' + property + '": '
            + formatValue(obj[property], depth+indentSpaces, true);
    }
    s += '\n' + indent(depth) + '}';
    return s;
}

function formatScalar(obj) {
    if (isString(obj))
        return '"' + obj + '"';
    if (isNumber(obj))
        return obj.toString();
    if (isNull(obj))
        return 'null';
    return obj.toString();
}

document.getElementById('format').onclick = function () {
    var jsonEle = document.getElementById('json');
    var jsonObj;
    try {
        jsonObj = JSON.parse(jsonEle.value);
        jsonEle.value = formatJson(jsonObj);
    } catch (error) {
        jsonEle.value = 'error: ' + error.message;
    }
}
