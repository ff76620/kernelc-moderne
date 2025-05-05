#pragma once 
#include 
#include "../ui/web_view.hpp"

namespace Win32Compat {
class MSHTMLWrapper {
private:
    WebView* webView;

public:
    MSHTMLWrapper() {
        webView = WebView::getInstance();
    }

    HRESULT CreateHTMLDocument(
        LPCWSTR pszHTML,
        IHTMLDocument2** ppDocument
    ) {
        DocumentParams params;
        params.html = convertToNativeString(pszHTML);
        return webView->createDocument(params, ppDocument);
    }

    HRESULT GetElementById(
        IHTMLDocument2* pDocument,
        LPCWSTR elementId,
        IHTMLElement** ppElement
    ) {
        return webView->getElementById(pDocument, convertToNativeString(elementId), ppElement);
    }

    HRESULT SetInnerHTML(
        IHTMLElement* pElement,
        LPCWSTR html
    ) {
        return webView->setInnerHTML(pElement, convertToNativeString(html));
    }

    HRESULT GetInnerHTML(
        IHTMLElement* pElement,
        BSTR* pHtml
    ) {
        return webView->getInnerHTML(pElement, pHtml);
    }

    HRESULT AddEventListener(
        IHTMLElement* pElement,
        LPCWSTR eventName,
        IDispatch* pCallback
    ) {
        return webView->addEventListener(
            pElement,
            convertToNativeString(eventName),
            pCallback
        );
    }
};
} // namespace Win32Compat