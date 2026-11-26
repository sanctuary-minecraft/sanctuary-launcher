import QtQuick
import QtWebView

WebView {
    id: browser
    objectName: "sanctuaryWebView"

    signal pageReady(url address)
    signal pageFailed()

    onLoadingChanged: function(request) {
        if (request.status === WebView.LoadSucceededStatus)
            pageReady(request.url)
        else if (request.status === WebView.LoadFailedStatus)
            pageFailed()
    }

    function evaluate(script) { browser.runJavaScript(script) }
}
