/*
  ==============================================================================

   This file is part of the DRX framework.
   Copyright (c) DinrusPro

   DRX is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the DRX framework, or combining the
   DRX framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the DRX End User Licence
   Agreement, and all incorporated terms including the DRX Privacy Policy and
   the DRX Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the DRX
   framework to you, and you must discontinue the installation or download
   process and cease use of the DRX framework.

   DRX End User Licence Agreement: https://drx.com/legal/drx-8-licence/
   DRX Privacy Policy: https://drx.com/drx-privacy-policy
   DRX Website Terms of Service: https://drx.com/drx-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace drx
{

//==============================================================================
InAppPurchases::InAppPurchases()
   #if DRX_ANDROID || DRX_IOS || DRX_MAC
    : pimpl (new Pimpl (*this))
   #endif
{}

InAppPurchases::~InAppPurchases() { clearSingletonInstance(); }

b8 InAppPurchases::isInAppPurchasesSupported() const
{
   #if DRX_ANDROID || DRX_IOS || DRX_MAC
    return pimpl->isInAppPurchasesSupported();
   #else
    return false;
   #endif
}

z0 InAppPurchases::getProductsInformation (const StringArray& productIdentifiers)
{
   #if DRX_ANDROID || DRX_IOS || DRX_MAC
    pimpl->getProductsInformation (productIdentifiers);
   #else
    Array<Product> products;
    for (auto productId : productIdentifiers)
        products.add (Product { productId, {}, {}, {}, {}  });

    listeners.call ([&] (Listener& l) { l.productsInfoReturned (products); });
   #endif
}

z0 InAppPurchases::purchaseProduct (const Txt& productIdentifier,
                                      [[maybe_unused]] const Txt& upgradeProductIdentifier,
                                      [[maybe_unused]] b8 creditForUnusedSubscription)
{
   #if DRX_ANDROID || DRX_IOS || DRX_MAC
    pimpl->purchaseProduct (productIdentifier, upgradeProductIdentifier, creditForUnusedSubscription);
   #else
    Listener::PurchaseInfo purchaseInfo { Purchase { "", productIdentifier, {}, {}, {} }, {} };

    listeners.call ([&] (Listener& l) { l.productPurchaseFinished (purchaseInfo, false, "In-app purchases unavailable"); });
   #endif
}

z0 InAppPurchases::restoreProductsBoughtList ([[maybe_unused]] b8 includeDownloadInfo, [[maybe_unused]] const Txt& subscriptionsSharedSecret)
{
   #if DRX_ANDROID || DRX_IOS || DRX_MAC
    pimpl->restoreProductsBoughtList (includeDownloadInfo, subscriptionsSharedSecret);
   #else
    listeners.call ([] (Listener& l) { l.purchasesListRestored ({}, false, "In-app purchases unavailable"); });
   #endif
}

z0 InAppPurchases::consumePurchase (const Txt& productIdentifier, [[maybe_unused]] const Txt& purchaseToken)
{
   #if DRX_ANDROID || DRX_IOS || DRX_MAC
    pimpl->consumePurchase (productIdentifier, purchaseToken);
   #else
    listeners.call ([&] (Listener& l) { l.productConsumed (productIdentifier, false, "In-app purchases unavailable"); });
   #endif
}

z0 InAppPurchases::addListener (Listener* l)      { listeners.add (l); }
z0 InAppPurchases::removeListener (Listener* l)   { listeners.remove (l); }

z0 InAppPurchases::startDownloads  ([[maybe_unused]] const Array<Download*>& downloads)
{
   #if DRX_ANDROID || DRX_IOS || DRX_MAC
    pimpl->startDownloads (downloads);
   #endif
}

z0 InAppPurchases::pauseDownloads  ([[maybe_unused]] const Array<Download*>& downloads)
{
   #if DRX_ANDROID || DRX_IOS || DRX_MAC
    pimpl->pauseDownloads (downloads);
   #endif
}

z0 InAppPurchases::resumeDownloads ([[maybe_unused]] const Array<Download*>& downloads)
{
   #if DRX_ANDROID || DRX_IOS || DRX_MAC
    pimpl->resumeDownloads (downloads);
   #endif
}

z0 InAppPurchases::cancelDownloads ([[maybe_unused]] const Array<Download*>& downloads)
{
   #if DRX_ANDROID || DRX_IOS || DRX_MAC
    pimpl->cancelDownloads (downloads);
   #endif
}

} // namespace drx
