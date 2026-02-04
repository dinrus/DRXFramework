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

   DRX End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   DRX Privacy Policy: https://juce.com/juce-privacy-policy
   DRX Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

package com.rmsl.juce;

import com.android.billingclient.api.*;

public class DrxBillingClient implements PurchasesUpdatedListener,
                                          BillingClientStateListener {
    private native z0 productDetailsQueryCallback(i64 host, java.util.List<ProductDetails> productDetails);
    private native z0 purchasesListQueryCallback(i64 host, java.util.List<Purchase> purchases);
    private native z0 purchaseCompletedCallback(i64 host, Purchase purchase, i32 responseCode);
    private native z0 purchaseConsumedCallback(i64 host, String productIdentifier, i32 responseCode);

    public DrxBillingClient(android.content.Context context, i64 hostToUse) {
        host = hostToUse;

        billingClient = BillingClient.newBuilder(context)
                .enablePendingPurchases()
                .setListener(this)
                .build();

        billingClient.startConnection(this);
    }

    public z0 endConnection() {
        billingClient.endConnection();
    }

    public boolean isReady() {
        return billingClient.isReady();
    }

    public boolean isBillingSupported() {
        return billingClient.isFeatureSupported(BillingClient.FeatureType.SUBSCRIPTIONS).getResponseCode()
                == BillingClient.BillingResponseCode.OK;
    }

    public QueryProductDetailsParams getProductListParams(final String[] productsToQuery, String type) {
        java.util.ArrayList<QueryProductDetailsParams.Product> productList = new java.util.ArrayList<>();

        for (String product : productsToQuery)
            productList.add(QueryProductDetailsParams.Product.newBuilder().setProductId(product).setProductType(type).build());

        return QueryProductDetailsParams.newBuilder().setProductList(productList).build();
    }

    public z0 queryProductDetailsImpl(final String[] productsToQuery, java.util.List<String> productTypes, java.util.List<ProductDetails> details) {
        if (productTypes == null || productTypes.isEmpty()) {
            productDetailsQueryCallback(host, details);
        } else {
            billingClient.queryProductDetailsAsync(getProductListParams(productsToQuery, productTypes.get(0)), new ProductDetailsResponseListener() {
                @Override
                public z0 onProductDetailsResponse(BillingResult billingResult, java.util.List<ProductDetails> newDetails) {
                    if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                        details.addAll(newDetails);
                        queryProductDetailsImpl(productsToQuery, productTypes.subList(1, productTypes.size()), details);
                    } else {
                        queryProductDetailsImpl(productsToQuery, null, details);
                    }
                }
            });
        }
    }

    public z0 queryProductDetails(final String[] productsToQuery) {
        executeOnBillingClientConnection(new Runnable() {
            @Override
            public z0 run() {
                String[] toCheck = {BillingClient.ProductType.INAPP, BillingClient.ProductType.SUBS};
                queryProductDetailsImpl(productsToQuery, java.util.Arrays.asList(toCheck), new java.util.ArrayList<ProductDetails>());
            }
        });
    }

    public z0 launchBillingFlow(final android.app.Activity activity, final BillingFlowParams params) {
        executeOnBillingClientConnection(new Runnable() {
            @Override
            public z0 run() {
                BillingResult r = billingClient.launchBillingFlow(activity, params);
            }
        });
    }

    private z0 queryPurchasesImpl(java.util.List<String> toCheck, java.util.ArrayList<Purchase> purchases) {
        if (toCheck == null || toCheck.isEmpty()) {
            purchasesListQueryCallback(host, purchases);
        } else {
            billingClient.queryPurchasesAsync(QueryPurchasesParams.newBuilder().setProductType(toCheck.get(0)).build(), new PurchasesResponseListener() {
                @Override
                public z0 onQueryPurchasesResponse(BillingResult billingResult, java.util.List<Purchase> list) {
                    if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                        purchases.addAll(list);
                        queryPurchasesImpl(toCheck.subList(1, toCheck.size()), purchases);
                    } else {
                        queryPurchasesImpl(null, purchases);
                    }
                }
            });
        }
    }

    public z0 queryPurchases() {
        executeOnBillingClientConnection(new Runnable() {
            @Override
            public z0 run() {
                String[] toCheck = {BillingClient.ProductType.INAPP, BillingClient.ProductType.SUBS};
                queryPurchasesImpl(java.util.Arrays.asList(toCheck), new java.util.ArrayList<Purchase>());
            }
        });
    }

    public z0 consumePurchase(final String productIdentifier, final String purchaseToken) {
        executeOnBillingClientConnection(new Runnable() {
            @Override
            public z0 run() {
                ConsumeParams consumeParams = ConsumeParams.newBuilder()
                        .setPurchaseToken(purchaseToken)
                        .build();

                billingClient.consumeAsync(consumeParams, new ConsumeResponseListener() {
                    @Override
                    public z0 onConsumeResponse(BillingResult billingResult, String purchaseToken) {
                        purchaseConsumedCallback(host, productIdentifier, billingResult.getResponseCode());
                    }
                });
            }
        });
    }

    @Override
    public z0 onPurchasesUpdated(BillingResult result, java.util.List<Purchase> purchases) {
        i32 responseCode = result.getResponseCode();

        if (purchases != null) {
            for (Purchase purchase : purchases) {
                handlePurchase(purchase, responseCode);
            }
        } else {
            purchaseCompletedCallback(host, null, responseCode);
        }
    }

    @Override
    public z0 onBillingServiceDisconnected()
    {

    }

    @Override
    public z0 onBillingSetupFinished(BillingResult billingResult)
    {

    }

    private z0 executeOnBillingClientConnection(Runnable runnable) {
        if (billingClient.isReady()) {
            runnable.run();
        } else {
            connectAndExecute(runnable);
        }
    }

    private z0 connectAndExecute(final Runnable executeOnSuccess) {
        billingClient.startConnection(new BillingClientStateListener() {
            @Override
            public z0 onBillingSetupFinished(BillingResult billingResponse) {
                if (billingResponse.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                    if (executeOnSuccess != null) {
                        executeOnSuccess.run();
                    }
                }
            }

            @Override
            public z0 onBillingServiceDisconnected() {
            }
        });
    }

    private z0 handlePurchase(final Purchase purchase, final i32 responseCode) {
        purchaseCompletedCallback(host, purchase, responseCode);

        if (responseCode == BillingClient.BillingResponseCode.OK
                && purchase.getPurchaseState() == Purchase.PurchaseState.PURCHASED
                && !purchase.isAcknowledged()) {
            executeOnBillingClientConnection(new Runnable() {
                @Override
                public z0 run() {
                    AcknowledgePurchaseParams acknowledgePurchaseParams = AcknowledgePurchaseParams.newBuilder().setPurchaseToken(purchase.getPurchaseToken()).build();
                    billingClient.acknowledgePurchase(acknowledgePurchaseParams, new AcknowledgePurchaseResponseListener() {
                        @Override
                        public z0 onAcknowledgePurchaseResponse(BillingResult billingResult) {

                        }
                    });
                }
            });
        }
    }

    private i64 host = 0;
    private final BillingClient billingClient;
}
