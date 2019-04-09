#ifndef HELLO_RESOLVER_TOKENS_H
#define HELLO_RESOLVER_TOKENS_H

#include <pxr/base/tf/staticTokens.h>
#include <pxr/pxr.h>
#include <pxr/usd/ar/api.h>

#include <vector>

PXR_NAMESPACE_OPEN_SCOPE

/// \class HelloResolverTokensType
///
/// \link HelloResolverTokens \endlink provides static, efficient
/// \link TfToken TfTokens\endlink for use in all public USD API.
///
/// \code
///     gprim.GetMyTokenValuedAttr().Set(UsdGeomTokens->all);
/// \endcode
struct HelloResolverTokensType {
    AR_API HelloResolverTokensType();

    const TfToken replacePairs;

    const TfToken replaceFileName;

    /// A vector of all of the tokens listed above.
    const std::vector<TfToken> allTokens;
};

/// \var HelloResolverTokens
///
/// A global variable with static, efficient \link TfToken TfTokens\endlink
/// for use in all public USD API.  \sa HelloResolverTokensType
extern AR_API TfStaticData<HelloResolverTokensType> HelloResolverTokens;

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HELLO_RESOLVER_TOKENS_H
