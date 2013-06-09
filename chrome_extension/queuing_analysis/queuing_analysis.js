// pre-condition: rho > 0 and n >=1 and integral
function posionRatio(rho, n) {
    var nominator = 1;
    var denominator = 1;
    var item_nominator = 1;
    var item_denominator = 1;
    
    for (var i = 1; i <= n-1; i++) {
        item_nominator *= n * rho;
        item_denominator *= i;
        nominator += item_nominator / item_denominator;
    }

    item_nominator *= n * rho;
    item_denominator *= n;
    var denominator = nominator + item_nominator / item_denominator;
    return nominator / denominator;
}

function erlang_c(rho, n) {
    var k = posionRatio(rho, n);
    return (1 - k) / (1 - rho * k);
}

function waitingSize(rho, n) {
    var c = erlang_c(rho, n);
    return c*rho / (1-rho);
}

// pre-condition: rho > 0 and n integral and > 0.
function residentSize(rho, n) {
    var c = erlang_c(rho, n);
    return waitingSize(rho, n) + n*rho;
}

// Compute T_w/T_s.
function TwOverTs(rho, n) {
    var c = erlang_c(rho, n);
    return c / (n*(1-rho));
}

// Compute T_r/T_s.
function TrOverTs(rho, n) {
    return TwOverTs(rho, n) + 1;
}

// Compute the T_w's y-percentile.
function waitingTimePercentile(rho, n, T_s, y) {
    var c = erlang_c(rho, n);
    return T_s * Math.log(100*c/(100-y)) / (n*(1-rho));
}

function compute() {
    var nthreads = parseInt(document.getElementById('nthreads').value);
    var T_s = parseFloat(document.getElementById('t_ms').value) / 1000;
    var html = '<table border="1"><tr><th>&rho;</th>'
        + '<th>r</th><th>w</th>'
        + '<th>T<sub>w</sub>(s)</th><th>&lambda; (#/s)</th>'
        + '<th>&lambda; (#/day)</th><th>T<sub>w</sub>&apos;s 95th percentile</th></tr>';

    for (var rho = 0.0; rho <= 0.9; rho += 0.1) {
        var r = residentSize(rho, nthreads);
        var w = waitingSize(rho, nthreads);
        var T_w = TwOverTs(rho, nthreads) * T_s;
        html += '<tr><td>' + rho.toFixed(3) + '</td><td>' + r.toFixed(3) + '</td>'
            + '<td>' + w.toFixed(3) + '</td>'
            + '<td>' + T_w.toFixed(3) + '</td><td>' + (nthreads*rho/T_s).toFixed(3) + '</td>'
            + '<td>' + ((nthreads*rho/T_s) * 3600 * 24).toFixed(0) + '</td>'
            + '<td>' + waitingTimePercentile(rho, nthreads, T_s, 95).toFixed(3) + '</td></tr>';
    }
    html += '</table>';
    document.getElementById('table').innerHTML = html;
}

document.getElementById('compute').onclick = compute;
