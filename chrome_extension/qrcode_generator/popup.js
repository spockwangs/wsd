var hints = (function () {
    var selected = -1;
    var ul = document.getElementById('url-hint');
    var is_hidden = true;
    
    function clearHints() {
        var ul = document.getElementById('url-hint');
        while (ul.hasChildNodes())
            ul.removeChild(ul.lastChild);
    }

    function getAllRememberedUrls() {
        var history = window.localStorage.getItem('history');
        if (history == null)
            return [];
        return JSON.parse(history);
    }

    function getUrlHints(input) {
        var urls = getAllRememberedUrls();
        var hints = [];
        for (var i = 0; i < urls.length; i++)
            if (urls[i].match(input))
                hints.push(urls[i]);
        return hints;
    }

    function select(ele) {
        ele.className = "hint is-selected";
    }

    function unselect(ele) {
        ele.className = "hint";
    }
    
    return {
        rememberThisUrl : function (url) {
            var history = window.localStorage.getItem('history');
            if (history == null)
                history = [];
            else
                history = JSON.parse(history);
            for (var i = 0; i < history.length; i++) {
                if (history[i] === url)
                    return;
            }
            history.unshift(url);
            while (history.length > 10)
                history.pop();
            window.localStorage.setItem('history', JSON.stringify(history));
        },
        hide : function () {
            ul.style.display = "none";
            clearHints();
            is_hidden = true;
            selected = -1;
        },
        show : function (input, on_selected) {
            var that = this;
            this.hide();
            var url_hints = getUrlHints(input);
            for (var i = 0; i < url_hints.length; i++) {
                var hint_ele = document.createElement('div');
                unselect(hint_ele);
                hint_ele.onmouseover = function(event) {
                    select(event.target);
                };
                hint_ele.onmouseout = function(event) {
                    unselect(event.target);
                };
                hint_ele.onmouseup = function(event) {
                    that.hide();
                    on_selected(event.target.textContent);
                };
                hint_ele.appendChild(document.createTextNode(url_hints[i]));
                ul.appendChild(hint_ele);
            }
            if (url_hints.length > 0)
                ul.style.display = "block";
            is_hidden = false;
            selected = -1;
        },
        selectNext : function() {
            var prev = selected;
            if (is_hidden)
                return;
            if (++selected >= ul.childNodes.length)
                selected = 0;

            if (prev >= 0)
                unselect(ul.childNodes[prev]);
            var hint_ele = ul.childNodes[selected];
            select(hint_ele);
            return hint_ele.textContent;
        },
        selectPrev : function() {
            var prev = selected;
            if (is_hidden)
                return;
            if (--selected < 0)
                selected = ul.childNodes.length-1;
            if (prev >= 0)
                unselect(ul.childNodes[prev]);
            var hint_ele = ul.childNodes[selected];
            select(hint_ele);
            return hint_ele.textContent;
        }
    };

}());

document.addEventListener('click', hints.hide);

document.getElementById('url').onkeyup = function (event) {
    if (event.keyCode == 13) {
        // The user clicked the "Enter" key.
        var url = document.getElementById('url').value;
        genQrCode(url);
        hints.rememberThisUrl(url);
        hints.hide();
    } else if (event.keyCode == 38) {
        // "Up" key
        document.getElementById('url').value = hints.selectPrev();
    } else if (event.keyCode == 40) {
        // "Down" key
        document.getElementById('url').value = hints.selectNext();
    } else {
        // Other keys.
        var input_obj = document.getElementById('url');
        hints.show(input_obj.value, function (url) {
            input_obj.value = url;
            genQrCode(url);
        });
    }
};

document.getElementById('url').onmousedown = hints.hide;

function genQrCode(url) {
    document.getElementById('qr-code').setAttribute(
        "src",
        "http://chart.apis.google.com/chart?cht=qr&chs=300x300&chl="
            + encodeURIComponent(url));
}

chrome.tabs.getSelected(null, function (tab) {
    document.getElementById('url').value = tab.url;
    genQrCode(tab.url);
});
