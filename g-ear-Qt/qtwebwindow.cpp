#include <QWebView>
#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include "qtwebwindow.h"

using std::make_pair;
using std::map;

namespace Gear
{
    class PrivateCookieJar : public QNetworkCookieJar
    {
    public:
        QList<QNetworkCookie> allCookies () const
        {
            return QNetworkCookieJar::allCookies();
        }
    };

#define method QtWebWindow::

// based on qtwidgets because it lets us do more direct manipulations

    method QtWebWindow(const shared_ptr<IWebWindowDelegate> &delegate) :
        IWebWindow(delegate),
        _visible(false)
    {
        //_window.resize(320,480);
        // display is wrong, so we resize accordingly, but TODO: make 320x480 work
        _window.resize(640,640);
        _window.setAttribute(Qt::WA_DeleteOnClose, false);
        connect(&_window, SIGNAL(closed()), this, SLOT(updateVisibilityStatus()));

        _webView = new QWebView(&_window);
        _webView->setPage(&_userAgentSwitcher);
        _cookieJar = new PrivateCookieJar();
        _webView->page()->networkAccessManager()->setCookieJar(_cookieJar);

        QObject::connect(_webView, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
        QObject::connect(_webView, SIGNAL(loadStarted()), this, SLOT(loadStarted()));

        //_webView->load(QUrl("http://www.google.com"));
        //_window.show();

        /*_viewer.setMainQmlFile(QStringLiteral("qrc:/WebWindow.qml"));
        _webView = _viewer.window()->findChild<QWebView *>("webview");

        _viewer.show();*/
    }

    void method loadFinished(bool ok)
    {
        if (ok) {
            _delegate->didFinishLoad();
        } else {
            _delegate->didFailLoad("");
        }
    }

    void method loadStarted()
    {
        _delegate->navigationAction();
    }

    void method updateVisibilityStatus()
    {
        if (_window.isVisible())
        {
            _visible = true;
        }
        else
        {
            _visible = false;
        }
    }

    void method setUserAgent(const string &userAgent)
    {
        _userAgentSwitcher.setUserAgent(userAgent);
    }

    string method url() const
    {
        return "";
    }

    string method title() const
    {
        return "";
    }

    vector<WebCookie> method cookies()
    {
        QNetworkCookieJar *cookieJar = _webView->page()->networkAccessManager()->cookieJar();
        QList<QNetworkCookie> cookies = _cookieJar->allCookies();
        vector<WebCookie> ret;
        for (auto &cookie : cookies) {
            map<string,string> properties;
            auto addA = [&](const string &key, const QByteArray &value){
                properties.insert(make_pair(key, string(value.data(), value.length())));
            };
            auto addS = [&](const string &key, const QString &value){
                properties.insert(make_pair(key, value.toStdString()));
            };
            addA("Name", cookie.name());
            addA("Value", cookie.value());
            addS("Domain", cookie.domain());

            ret.push_back(WebCookie(properties));
        }
        return ret;
    }

    void method loadUrl(const string &url)
    {
        _webView->load(QUrl(url.c_str()));
    }

    void method injectCookie(const WebCookie &cookie)
    {
    }

    void method show(const string &title)
    {
        _window.show();
        updateVisibilityStatus();
    }

    bool method visible() const
    {
        return _visible;
    }

    string method fieldValueForElementId(const string &elementId) const
    {
        return "";
    }

    // TODO: handle delegate

#undef method
#define method QtWebWindow::UserAgentSwitcher::

    void method setUserAgent(const string &userAgent)
    {
        _userAgent = userAgent;
    }

    QString method userAgentForUrl ( const QUrl & url ) const
    {
        return _userAgent.c_str();
    }

#undef method

}
