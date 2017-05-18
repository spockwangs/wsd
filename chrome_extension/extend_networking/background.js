// Copyright (c) 2017 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

chrome.browserAction.onClicked.addListener(function(activeTab) {
    var newURL = "http://auth-proxy.oa.com/DevNetTempVisit.aspx";
    chrome.tabs.create({ url: newURL });
});
