// Copyright 2019 Rodeo FX.  All rights reserved.
#ifndef REPLACE_RESOLVER_TOKENS_H
#define REPLACE_RESOLVER_TOKENS_H

#include <pxr/base/tf/staticTokens.h>
#include <pxr/pxr.h>
#include <pxr/usd/ar/api.h>

#include <vector>

PXR_NAMESPACE_OPEN_SCOPE

/// \class ReplaceResolverTokensType
///
/// \link ReplaceResolverTokens \endlink provides static, efficient
/// \link TfToken TfTokens\endlink for use in all public USD API.
///
/// \code
///     gprim.GetMyTokenValuedAttr().Set(UsdGeomTokens->all);
/// \endcode
struct ReplaceResolverTokensType {
    AR_API ReplaceResolverTokensType();

    const TfToken replacePairs;

    const TfToken replaceFileName;

    /// A vector of all of the tokens listed above.
    const std::vector<TfToken> allTokens;
};

/// \var ReplaceResolverTokens
///
/// A global variable with static, efficient \link TfToken TfTokens\endlink
/// for use in all public USD API.  \sa ReplaceResolverTokensType
extern AR_API TfStaticData<ReplaceResolverTokensType> ReplaceResolverTokens;

PXR_NAMESPACE_CLOSE_SCOPE

#endif // REPLACE_RESOLVER_TOKENS_H
