#ifndef QTWEBWINDOW_H
#define QTWEBWINDOW_H

#include <QtGui>
#include <QWidget>
#include <QWebPage>
#ifndef Q_MOC_RUN
#include "IWebWindow.h"
#endif

namespace Gear
{
    class ActualWindow : public QWidget
    {
        Q_OBJECT
    public:
        ~ActualWindow() {}

    signals:
        void closed();

    protected:
        virtual void closeEvent(QCloseEvent *event)
        {
            emit closed();
            QWidget::closeEvent(event);
        }
    private:
        explicit ActualWindow(QWidget *parent = 0, Qt::WindowFlags f = 0)
            : QWidget(parent, f)
        {
        }

        friend class QtWebWindow;
    };

    class QtWebWindow final : public QObject, public IWebWindow
    {
        Q_OBJECT

    public:
        QtWebWindow(const shared_ptr<IWebWindowDelegate> &delegate);

        virtual void setUserAgent(const string &userAgent);
        virtual string url() const;
        virtual string title() const;
        virtual vector<WebCookie> cookies();
        virtual void loadUrl(const string &url);
        virtual void injectCookie(const WebCookie &cookie);
        virtual void show(const string &title);
        virtual bool visible() const;
        virtual string fieldValueForElementId(const string &elementId) const;

    private:
        ActualWindow _window;

        class UserAgentSwitcher : public QWebPage
        {
        public:
            void setUserAgent(const string &userAgent);
            virtual QString userAgentForUrl ( const QUrl & url ) const;

        private:
            string _userAgent;
        };

        UserAgentSwitcher _userAgentSwitcher;
        class QWebView *_webView;
        class PrivateCookieJar *_cookieJar;
        bool _visible;

    private slots:
         void loadFinished(bool ok);
         void loadStarted();
         void updateVisibilityStatus();
    };
}

#endif // QTWEBWINDOW_H
